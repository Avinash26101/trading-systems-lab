#pragma once

#include "hft/order_book.hpp"
#include "hft/types.hpp"

#include <vector>

namespace hft {

class ExchangeSimulator {
 public:
  explicit ExchangeSimulator(double fee_per_share = 0.0002);
  std::vector<Fill> execute(const Order& order, const OrderBook& book);
  double fees() const { return fees_; }

 private:
  double fee_per_share_;
  double fees_ = 0.0;
};

} // namespace hft

