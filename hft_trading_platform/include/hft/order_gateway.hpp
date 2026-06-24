#pragma once

#include "hft/exchange_simulator.hpp"
#include "hft/risk_engine.hpp"
#include "hft/types.hpp"

#include <vector>

namespace hft {

struct GatewayResult {
  bool sent = false;
  std::string reason;
  std::vector<Fill> fills;
};

class OrderGateway {
 public:
  OrderGateway(RiskEngine& risk, ExchangeSimulator& exchange);
  GatewayResult send(const Order& order, const OrderBook& book, const Portfolio& portfolio);

 private:
  RiskEngine& risk_;
  ExchangeSimulator& exchange_;
};

} // namespace hft

