#include "hft/order_book.hpp"

namespace hft {

void OrderBook::apply(const MarketUpdate& update) {
  bids_.clear();
  asks_.clear();
  if (update.bid_quantity > 0) bids_[update.bid_price] = update.bid_quantity;
  if (update.ask_quantity > 0) asks_[update.ask_price] = update.ask_quantity;
}

TopOfBook OrderBook::top() const {
  TopOfBook top;
  if (!bids_.empty()) {
    top.bid_price = bids_.begin()->first;
    top.bid_quantity = bids_.begin()->second;
  }
  if (!asks_.empty()) {
    top.ask_price = asks_.begin()->first;
    top.ask_quantity = asks_.begin()->second;
  }
  return top;
}

double OrderBook::imbalance() const {
  const TopOfBook t = top();
  const double total = static_cast<double>(t.bid_quantity + t.ask_quantity);
  if (total <= 0.0) return 0.0;
  return static_cast<double>(t.bid_quantity - t.ask_quantity) / total;
}

bool OrderBook::crossed() const {
  const TopOfBook t = top();
  return t.bid_price > 0.0 && t.ask_price > 0.0 && t.bid_price >= t.ask_price;
}

} // namespace hft

