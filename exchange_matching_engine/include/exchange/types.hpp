#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace exchange {

using OrderId = std::uint64_t;
using Quantity = std::uint32_t;
using Price = std::int64_t;

enum class Side : std::uint8_t {
  Buy = 0,
  Sell = 1
};

enum class OrderType : std::uint8_t {
  Limit = 0,
  Market = 1
};

struct OrderInput {
  OrderId id = 0;
  Side side = Side::Buy;
  OrderType type = OrderType::Limit;
  Price price = 0;
  Quantity quantity = 0;
};

struct Trade {
  OrderId buy_order_id = 0;
  OrderId sell_order_id = 0;
  Price price = 0;
  Quantity quantity = 0;
};

struct MatchResult {
  std::vector<Trade> trades;
  Quantity remaining_quantity = 0;
  bool accepted = false;
  std::string message;
};

} // namespace exchange

