#pragma once

#include "exchange/latency.hpp"
#include "exchange/lock_free_queue.hpp"
#include "exchange/memory_pool.hpp"
#include "exchange/threading.hpp"
#include "exchange/types.hpp"

#include <atomic>
#include <cstddef>
#include <deque>
#include <functional>
#include <map>
#include <unordered_map>
#include <vector>

namespace exchange {

struct BookSnapshot {
  Price best_bid = 0;
  Price best_ask = 0;
  Quantity bid_quantity = 0;
  Quantity ask_quantity = 0;
  std::size_t live_orders = 0;
};

class OrderBook {
 public:
  explicit OrderBook(std::size_t max_orders = 1 << 20);
  ~OrderBook();

  MatchResult add_order(const OrderInput& input);
  bool cancel(OrderId id);
  BookSnapshot snapshot() const;

 private:
  struct Order {
    OrderInput input;
    Quantity open_quantity;
    std::uint64_t sequence;
  };

  using BidLevels = std::map<Price, std::deque<Order*>, std::greater<Price>>;
  using AskLevels = std::map<Price, std::deque<Order*>, std::less<Price>>;

  template <typename Levels, typename Crosses>
  void match_against(Levels& levels, OrderInput& incoming, MatchResult& result, Crosses crosses);

  void rest_order(const OrderInput& input);
  void release_empty_and_inactive();

  MemoryPool<Order> pool_;
  BidLevels bids_;
  AskLevels asks_;
  std::unordered_map<OrderId, Order*> orders_;
  std::uint64_t next_sequence_ = 1;
};

enum class CommandKind {
  Add,
  Cancel,
  Stop
};

struct EngineCommand {
  CommandKind kind = CommandKind::Add;
  OrderInput order;
  OrderId cancel_id = 0;
};

class MatchingEngine {
 public:
  explicit MatchingEngine(std::size_t queue_capacity = 1 << 16, std::size_t max_orders = 1 << 20);
  ~MatchingEngine();

  MatchingEngine(const MatchingEngine&) = delete;
  MatchingEngine& operator=(const MatchingEngine&) = delete;

  void start();
  void stop();
  bool submit(const OrderInput& order);
  bool cancel(OrderId id);
  std::uint64_t processed() const;
  BookSnapshot snapshot() const;
  const LatencyRecorder& latency() const { return latency_; }

 private:
  void run();

  OrderBook book_;
  BoundedMpmcQueue<EngineCommand> queue_;
  Thread worker_;
  std::atomic<bool> running_;
  std::atomic<std::uint64_t> processed_;
  mutable Mutex book_mutex_;
  LatencyRecorder latency_;
};

} // namespace exchange
