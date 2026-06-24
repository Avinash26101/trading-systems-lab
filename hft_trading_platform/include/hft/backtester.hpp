#pragma once

#include "hft/exchange_simulator.hpp"
#include "hft/order_book.hpp"
#include "hft/order_gateway.hpp"
#include "hft/risk_engine.hpp"
#include "hft/strategy_engine.hpp"
#include "hft/types.hpp"

#include <string>
#include <vector>

namespace hft {

struct BacktestResult {
  int updates = 0;
  int orders_sent = 0;
  int fills = 0;
  int risk_rejects = 0;
  int final_position = 0;
  double final_pnl = 0.0;
  double fees = 0.0;
  int buy_signals = 0;
  int sell_signals = 0;
};

class Backtester {
 public:
  BacktestResult run(const std::string& csv_path);
  static std::vector<MarketUpdate> load_csv(const std::string& csv_path);
  static void write_dashboard_metrics(const std::string& path, const BacktestResult& result);
};

} // namespace hft

