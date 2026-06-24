#include "hft/backtester.hpp"
#include "hft/order_gateway.hpp"
#include "hft/strategy_engine.hpp"
#include "hft/udp_feed.hpp"

#include <cassert>

static void test_imbalance_strategy_and_risk() {
  hft::OrderBook book;
  book.apply({1, 100.00, 900, 100.02, 100});
  hft::ImbalanceStrategy strategy(0.30, 5);
  const hft::Order order = strategy.on_book(book);
  assert(order.id == 1);
  assert(order.side == hft::Side::Buy);
  assert(order.price == 100.02);

  hft::RiskEngine risk(10, 10, 2000.0);
  hft::Portfolio portfolio;
  const hft::RiskDecision decision = risk.check(order, portfolio);
  assert(decision.accepted);
}

static void test_gateway_and_exchange_fill() {
  hft::OrderBook book;
  book.apply({1, 100.00, 100, 100.01, 100});
  hft::RiskEngine risk(100, 20, 100000.0);
  hft::ExchangeSimulator exchange;
  hft::OrderGateway gateway(risk, exchange);
  hft::Portfolio portfolio;
  const hft::GatewayResult result =
      gateway.send({10, hft::Side::Buy, 100.01, 5, "test"}, book, portfolio);
  assert(result.sent);
  assert(result.fills.size() == 1);
  assert(result.fills[0].price == 100.01);
}

static void test_udp_codec() {
  const hft::MarketUpdate input{42, 99.5, 10, 99.6, 20};
  const hft::MarketUpdate output = hft::decode_update(hft::encode_update(input));
  assert(output.sequence == input.sequence);
  assert(output.bid_price == input.bid_price);
  assert(output.ask_quantity == input.ask_quantity);
}

static void test_backtest() {
  hft::Backtester backtester;
  const hft::BacktestResult result =
      backtester.run("hft_trading_platform/data/sample_market_data.csv");
  assert(result.updates > 0);
  assert(result.orders_sent > 0);
  assert(result.fills > 0);
}

int main() {
  test_imbalance_strategy_and_risk();
  test_gateway_and_exchange_fill();
  test_udp_codec();
  test_backtest();
  return 0;
}

