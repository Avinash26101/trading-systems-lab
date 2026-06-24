#include "hft/backtester.hpp"

#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace hft {

static void apply_fill(const Fill& fill, Portfolio& portfolio) {
  if (fill.side == Side::Buy) {
    portfolio.position += fill.quantity;
    portfolio.cash -= fill.price * static_cast<double>(fill.quantity);
  } else {
    portfolio.position -= fill.quantity;
    portfolio.cash += fill.price * static_cast<double>(fill.quantity);
  }
}

std::vector<MarketUpdate> Backtester::load_csv(const std::string& csv_path) {
  std::ifstream in(csv_path.c_str());
  if (!in) throw std::runtime_error("could not open market data csv: " + csv_path);

  std::vector<MarketUpdate> updates;
  std::string line;
  std::getline(in, line);
  while (std::getline(in, line)) {
    if (line.empty()) continue;
    std::stringstream ss(line);
    std::string cell;
    MarketUpdate update;
    std::getline(ss, cell, ',');
    update.sequence = static_cast<std::uint64_t>(std::stoull(cell));
    std::getline(ss, cell, ',');
    update.bid_price = std::stod(cell);
    std::getline(ss, cell, ',');
    update.bid_quantity = std::stoi(cell);
    std::getline(ss, cell, ',');
    update.ask_price = std::stod(cell);
    std::getline(ss, cell, ',');
    update.ask_quantity = std::stoi(cell);
    updates.push_back(update);
  }
  return updates;
}

BacktestResult Backtester::run(const std::string& csv_path) {
  OrderBook book;
  ImbalanceStrategy strategy(0.30, 5);
  RiskEngine risk(100, 20, 100000.0);
  ExchangeSimulator exchange;
  OrderGateway gateway(risk, exchange);
  Portfolio portfolio;
  BacktestResult result;
  double last_mid = 0.0;

  for (const auto& update : load_csv(csv_path)) {
    ++result.updates;
    book.apply(update);
    const TopOfBook top = book.top();
    if (top.bid_price > 0.0 && top.ask_price > 0.0) {
      last_mid = (top.bid_price + top.ask_price) / 2.0;
    }

    const Order order = strategy.on_book(book);
    if (order.id == 0) continue;

    const GatewayResult gateway_result = gateway.send(order, book, portfolio);
    if (!gateway_result.sent) {
      ++result.risk_rejects;
      continue;
    }
    ++result.orders_sent;
    for (const auto& fill : gateway_result.fills) {
      apply_fill(fill, portfolio);
      ++result.fills;
    }
  }

  result.final_position = portfolio.position;
  result.final_pnl = portfolio.cash + static_cast<double>(portfolio.position) * last_mid - exchange.fees();
  result.fees = exchange.fees();
  result.buy_signals = strategy.buy_signals();
  result.sell_signals = strategy.sell_signals();
  return result;
}

void Backtester::write_dashboard_metrics(const std::string& path, const BacktestResult& result) {
  std::ofstream out(path.c_str());
  if (!out) throw std::runtime_error("could not write dashboard metrics: " + path);
  out << "const metrics = {\n";
  out << "  throughput: \"see CI benchmark artifact\",\n";
  out << "  p99: \"see CI benchmark artifact\",\n";
  out << "  pnl: \"$" << std::fixed << std::setprecision(2) << result.final_pnl << "\",\n";
  out << "  updates: \"" << result.updates << "\",\n";
  out << "  orders: \"" << result.orders_sent << "\",\n";
  out << "  fills: \"" << result.fills << "\",\n";
  out << "  riskRejects: \"" << result.risk_rejects << "\",\n";
  out << "  position: \"" << result.final_position << "\"\n";
  out << "};\n\n";
  out << "for (const [key, value] of Object.entries(metrics)) {\n";
  out << "  const node = document.getElementById(key);\n";
  out << "  if (node) node.textContent = value;\n";
  out << "}\n";
}

} // namespace hft

