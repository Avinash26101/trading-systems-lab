#pragma once

#include <cstdint>
#include <string>

namespace hft {

using Price = double;
using Quantity = int;

enum class Side {
  Buy,
  Sell
};

enum class Signal {
  Hold,
  Buy,
  Sell
};

struct MarketUpdate {
  std::uint64_t sequence = 0;
  Price bid_price = 0.0;
  Quantity bid_quantity = 0;
  Price ask_price = 0.0;
  Quantity ask_quantity = 0;
};

struct Order {
  std::uint64_t id = 0;
  Side side = Side::Buy;
  Price price = 0.0;
  Quantity quantity = 0;
  std::string reason;
};

struct Fill {
  std::uint64_t order_id = 0;
  Side side = Side::Buy;
  Price price = 0.0;
  Quantity quantity = 0;
};

struct Portfolio {
  int position = 0;
  double cash = 0.0;
  double realized_pnl = 0.0;
};

} // namespace hft

