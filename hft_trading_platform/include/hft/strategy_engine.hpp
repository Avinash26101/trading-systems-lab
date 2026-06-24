#pragma once

#include "hft/order_book.hpp"
#include "hft/types.hpp"

namespace hft {

class ImbalanceStrategy {
 public:
  explicit ImbalanceStrategy(double threshold = 0.35, Quantity order_size = 10);
  Order on_book(const OrderBook& book);
  int buy_signals() const { return buy_signals_; }
  int sell_signals() const { return sell_signals_; }

 private:
  double threshold_;
  Quantity order_size_;
  std::uint64_t next_order_id_;
  int buy_signals_ = 0;
  int sell_signals_ = 0;
};

} // namespace hft

