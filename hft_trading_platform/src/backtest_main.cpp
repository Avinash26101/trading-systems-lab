#include "hft/backtester.hpp"

#include <iostream>
#include <string>

int main(int argc, char** argv) {
  const std::string input = argc > 1 ? argv[1] : "hft_trading_platform/data/sample_market_data.csv";
  const std::string metrics = argc > 2 ? argv[2] : "docs/metrics.js";

  hft::Backtester backtester;
  const hft::BacktestResult result = backtester.run(input);
  hft::Backtester::write_dashboard_metrics(metrics, result);

  std::cout << "HFT backtest\n";
  std::cout << "updates=" << result.updates << "\n";
  std::cout << "orders_sent=" << result.orders_sent << "\n";
  std::cout << "fills=" << result.fills << "\n";
  std::cout << "risk_rejects=" << result.risk_rejects << "\n";
  std::cout << "final_position=" << result.final_position << "\n";
  std::cout << "final_pnl=" << result.final_pnl << "\n";
  return 0;
}

