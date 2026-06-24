#include "exchange/order_book.hpp"

#include <stdexcept>

namespace exchange {

OrderBook::OrderBook(std::size_t max_orders) : pool_(max_orders) {}

OrderBook::~OrderBook() {
  for (auto& level : bids_) {
    for (Order* order : level.second) {
      pool_.destroy(order);
    }
  }
  for (auto& level : asks_) {
    for (Order* order : level.second) {
      pool_.destroy(order);
    }
  }
}

MatchResult OrderBook::add_order(const OrderInput& input) {
  MatchResult result;
  if (input.id == 0 || input.quantity == 0) {
    result.message = "invalid order";
    return result;
  }
  if (orders_.find(input.id) != orders_.end()) {
    result.message = "duplicate order id";
    return result;
  }

  OrderInput incoming = input;
  result.accepted = true;

  if (incoming.side == Side::Buy) {
    match_against(asks_, incoming, result, [&incoming](Price ask) {
      return incoming.type == OrderType::Market || incoming.price >= ask;
    });
  } else {
    match_against(bids_, incoming, result, [&incoming](Price bid) {
      return incoming.type == OrderType::Market || incoming.price <= bid;
    });
  }

  result.remaining_quantity = incoming.quantity;
  if (incoming.quantity > 0 && incoming.type == OrderType::Limit) {
    rest_order(incoming);
  }
  release_empty_and_inactive();
  return result;
}

bool OrderBook::cancel(OrderId id) {
  auto found = orders_.find(id);
  if (found == orders_.end()) return false;
  found->second->open_quantity = 0;
  orders_.erase(found);
  release_empty_and_inactive();
  return true;
}

BookSnapshot OrderBook::snapshot() const {
  BookSnapshot snap;
  snap.live_orders = orders_.size();
  if (!bids_.empty()) {
    snap.best_bid = bids_.begin()->first;
    for (Order* order : bids_.begin()->second) snap.bid_quantity += order->open_quantity;
  }
  if (!asks_.empty()) {
    snap.best_ask = asks_.begin()->first;
    for (Order* order : asks_.begin()->second) snap.ask_quantity += order->open_quantity;
  }
  return snap;
}

template <typename Levels, typename Crosses>
void OrderBook::match_against(Levels& levels, OrderInput& incoming, MatchResult& result, Crosses crosses) {
  while (incoming.quantity > 0 && !levels.empty()) {
    auto level = levels.begin();
    if (!crosses(level->first)) break;

    auto& queue = level->second;
    while (incoming.quantity > 0 && !queue.empty()) {
      Order* resting = queue.front();
      if (resting->open_quantity == 0) {
        pool_.destroy(resting);
        queue.pop_front();
        continue;
      }

      const Quantity traded = incoming.quantity < resting->open_quantity
                                  ? incoming.quantity
                                  : resting->open_quantity;
      incoming.quantity -= traded;
      resting->open_quantity -= traded;

      Trade trade;
      trade.price = resting->input.price;
      trade.quantity = traded;
      if (incoming.side == Side::Buy) {
        trade.buy_order_id = incoming.id;
        trade.sell_order_id = resting->input.id;
      } else {
        trade.buy_order_id = resting->input.id;
        trade.sell_order_id = incoming.id;
      }
      result.trades.push_back(trade);

      if (resting->open_quantity == 0) {
        orders_.erase(resting->input.id);
        pool_.destroy(resting);
        queue.pop_front();
      }
    }

    if (queue.empty()) {
      levels.erase(level);
    }
  }
}

void OrderBook::rest_order(const OrderInput& input) {
  Order* order = pool_.create(Order{input, input.quantity, next_sequence_++});
  orders_[input.id] = order;
  if (input.side == Side::Buy) {
    bids_[input.price].push_back(order);
  } else {
    asks_[input.price].push_back(order);
  }
}

void OrderBook::release_empty_and_inactive() {
  for (auto level = bids_.begin(); level != bids_.end();) {
    auto& q = level->second;
    while (!q.empty() && q.front()->open_quantity == 0) {
      pool_.destroy(q.front());
      q.pop_front();
    }
    if (q.empty()) level = bids_.erase(level);
    else ++level;
  }
  for (auto level = asks_.begin(); level != asks_.end();) {
    auto& q = level->second;
    while (!q.empty() && q.front()->open_quantity == 0) {
      pool_.destroy(q.front());
      q.pop_front();
    }
    if (q.empty()) level = asks_.erase(level);
    else ++level;
  }
}

MatchingEngine::MatchingEngine(std::size_t queue_capacity, std::size_t max_orders)
    : book_(max_orders), queue_(queue_capacity), running_(false), processed_(0) {}

MatchingEngine::~MatchingEngine() {
  stop();
}

void MatchingEngine::start() {
  bool expected = false;
  if (!running_.compare_exchange_strong(expected, true)) return;
  worker_.start([this] { run(); });
}

void MatchingEngine::stop() {
  bool expected = true;
  if (!running_.compare_exchange_strong(expected, false)) return;
  EngineCommand stop_command;
  stop_command.kind = CommandKind::Stop;
  while (!queue_.enqueue(stop_command)) {
    yield_thread();
  }
  if (worker_.joinable()) worker_.join();
}

bool MatchingEngine::submit(const OrderInput& order) {
  EngineCommand command;
  command.kind = CommandKind::Add;
  command.order = order;
  return queue_.enqueue(command);
}

bool MatchingEngine::cancel(OrderId id) {
  EngineCommand command;
  command.kind = CommandKind::Cancel;
  command.cancel_id = id;
  return queue_.enqueue(command);
}

std::uint64_t MatchingEngine::processed() const {
  return processed_.load(std::memory_order_acquire);
}

BookSnapshot MatchingEngine::snapshot() const {
  LockGuard lock(book_mutex_);
  return book_.snapshot();
}

void MatchingEngine::run() {
  while (running_.load(std::memory_order_acquire)) {
    EngineCommand command;
    if (!queue_.dequeue(command)) {
      yield_thread();
      continue;
    }
    if (command.kind == CommandKind::Stop) break;

    const auto start = std::chrono::high_resolution_clock::now();
    {
      LockGuard lock(book_mutex_);
      if (command.kind == CommandKind::Add) {
        book_.add_order(command.order);
      } else if (command.kind == CommandKind::Cancel) {
        book_.cancel(command.cancel_id);
      }
    }
    const auto end = std::chrono::high_resolution_clock::now();
    latency_.record(static_cast<std::uint64_t>(
        std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count()));
    processed_.fetch_add(1, std::memory_order_release);
  }
}

} // namespace exchange
