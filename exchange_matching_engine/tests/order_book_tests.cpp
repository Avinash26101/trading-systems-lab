#include "exchange/binary_protocol.hpp"
#include "exchange/order_book.hpp"

#include <cassert>

static void test_price_time_priority() {
  exchange::OrderBook book(32);
  book.add_order({1, exchange::Side::Sell, exchange::OrderType::Limit, 100, 10});
  book.add_order({2, exchange::Side::Sell, exchange::OrderType::Limit, 100, 10});
  const auto result = book.add_order({3, exchange::Side::Buy, exchange::OrderType::Limit, 100, 15});

  assert(result.accepted);
  assert(result.trades.size() == 2);
  assert(result.trades[0].sell_order_id == 1);
  assert(result.trades[0].quantity == 10);
  assert(result.trades[1].sell_order_id == 2);
  assert(result.trades[1].quantity == 5);
}

static void test_market_order_and_cancel() {
  exchange::OrderBook book(32);
  book.add_order({10, exchange::Side::Buy, exchange::OrderType::Limit, 99, 50});
  book.add_order({11, exchange::Side::Buy, exchange::OrderType::Limit, 100, 50});
  assert(book.cancel(11));

  const auto result = book.add_order({12, exchange::Side::Sell, exchange::OrderType::Market, 0, 60});
  assert(result.trades.size() == 1);
  assert(result.trades[0].buy_order_id == 10);
  assert(result.trades[0].price == 99);
  assert(result.trades[0].quantity == 50);
  assert(result.remaining_quantity == 10);
}

static void test_binary_protocol() {
  exchange::OrderInput order{77, exchange::Side::Sell, exchange::OrderType::Limit, 12345, 200};
  const auto encoded = exchange::protocol::encode_new_order(order);
  const auto decoded = exchange::protocol::decode_new_order(encoded);
  assert(decoded.id == order.id);
  assert(decoded.side == order.side);
  assert(decoded.type == order.type);
  assert(decoded.price == order.price);
  assert(decoded.quantity == order.quantity);

  const auto cancel = exchange::protocol::decode_cancel(exchange::protocol::encode_cancel(77));
  assert(cancel.id == 77);
}

static void test_async_engine() {
  exchange::MatchingEngine engine(1024, 1024);
  engine.start();
  for (int i = 1; i <= 100; ++i) {
    assert(engine.submit({static_cast<exchange::OrderId>(i), exchange::Side::Buy,
                          exchange::OrderType::Limit, 100, 1}));
  }
  while (engine.processed() < 100) {
    exchange::sleep_ms(1);
  }
  engine.stop();
  assert(engine.snapshot().live_orders == 100);
  assert(engine.latency().count() == 100);
}

int main() {
  test_price_time_priority();
  test_market_order_and_cancel();
  test_binary_protocol();
  test_async_engine();
  return 0;
}
