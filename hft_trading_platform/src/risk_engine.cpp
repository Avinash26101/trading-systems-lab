#include "hft/risk_engine.hpp"

#include <cmath>

namespace hft {

RiskEngine::RiskEngine(int max_position, Quantity max_order_quantity, double max_notional)
    : max_position_(max_position),
      max_order_quantity_(max_order_quantity),
      max_notional_(max_notional) {}

RiskDecision RiskEngine::check(const Order& order, const Portfolio& portfolio) const {
  if (order.id == 0 || order.quantity <= 0) {
    return {false, "no actionable order"};
  }
  if (order.quantity > max_order_quantity_) {
    return {false, "order quantity exceeds limit"};
  }
  if (std::fabs(order.price * static_cast<double>(order.quantity)) > max_notional_) {
    return {false, "order notional exceeds limit"};
  }

  const int signed_quantity = order.side == Side::Buy ? order.quantity : -order.quantity;
  const int projected = portfolio.position + signed_quantity;
  if (std::abs(projected) > max_position_) {
    return {false, "position limit exceeded"};
  }
  return {true, "accepted"};
}

} // namespace hft

