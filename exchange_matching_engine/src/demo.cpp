#include "exchange/order_book.hpp"

#include <iostream>

int main() {
  exchange::OrderBook book(1024);

  book.add_order({1, exchange::Side::Sell, exchange::OrderType::Limit, 10100, 100});
  book.add_order({2, exchange::Side::Sell, exchange::OrderType::Limit, 10200, 100});
  auto result = book.add_order({3, exchange::Side::Buy, exchange::OrderType::Limit, 10100, 40});
  auto snapshot = book.snapshot();

  std::cout << "Exchange matching demo\n";
  std::cout << "Trades: " << result.trades.size() << "\n";
  for (const auto& trade : result.trades) {
    std::cout << "buy=" << trade.buy_order_id
              << " sell=" << trade.sell_order_id
              << " price=" << trade.price
              << " qty=" << trade.quantity << "\n";
  }
  std::cout << "Best ask: " << snapshot.best_ask << " x " << snapshot.ask_quantity << "\n";
  return 0;
}

