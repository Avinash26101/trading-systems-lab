#pragma once

#include "hft/types.hpp"

#include <map>
#include <vector>

namespace hft {

struct TopOfBook {
  Price bid_price = 0.0;
  Quantity bid_quantity = 0;
  Price ask_price = 0.0;
  Quantity ask_quantity = 0;
};

class OrderBook {
 public:
  void apply(const MarketUpdate& update);
  TopOfBook top() const;
  double imbalance() const;
  bool crossed() const;

 private:
  std::map<Price, Quantity, std::greater<Price>> bids_;
  std::map<Price, Quantity, std::less<Price>> asks_;
};

} // namespace hft

