#include "hft/order_gateway.hpp"

namespace hft {

OrderGateway::OrderGateway(RiskEngine& risk, ExchangeSimulator& exchange)
    : risk_(risk), exchange_(exchange) {}

GatewayResult OrderGateway::send(const Order& order, const OrderBook& book, const Portfolio& portfolio) {
  const RiskDecision decision = risk_.check(order, portfolio);
  if (!decision.accepted) {
    return {false, decision.reason, {}};
  }
  GatewayResult result;
  result.sent = true;
  result.reason = "sent";
  result.fills = exchange_.execute(order, book);
  return result;
}

} // namespace hft

