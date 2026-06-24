#pragma once

#include "hft/types.hpp"

#include <string>

namespace hft {

struct RiskDecision {
  bool accepted = false;
  std::string reason;
};

class RiskEngine {
 public:
  RiskEngine(int max_position, Quantity max_order_quantity, double max_notional);
  RiskDecision check(const Order& order, const Portfolio& portfolio) const;

 private:
  int max_position_;
  Quantity max_order_quantity_;
  double max_notional_;
};

} // namespace hft

