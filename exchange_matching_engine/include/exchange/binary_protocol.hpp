#pragma once

#include "exchange/types.hpp"

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <vector>

namespace exchange {
namespace protocol {

enum class MessageType : std::uint8_t {
  NewOrder = 1,
  CancelOrder = 2
};

struct CancelInput {
  OrderId id = 0;
};

inline void append_u8(std::vector<std::uint8_t>& out, std::uint8_t value) {
  out.push_back(value);
}

template <typename T>
void append_le(std::vector<std::uint8_t>& out, T value) {
  for (std::size_t i = 0; i < sizeof(T); ++i) {
    out.push_back(static_cast<std::uint8_t>((static_cast<std::uint64_t>(value) >> (i * 8)) & 0xff));
  }
}

template <typename T>
T read_le(const std::vector<std::uint8_t>& data, std::size_t& offset) {
  if (offset + sizeof(T) > data.size()) throw std::runtime_error("truncated binary message");
  std::uint64_t value = 0;
  for (std::size_t i = 0; i < sizeof(T); ++i) {
    value |= static_cast<std::uint64_t>(data[offset++]) << (i * 8);
  }
  return static_cast<T>(value);
}

inline std::vector<std::uint8_t> encode_new_order(const OrderInput& order) {
  std::vector<std::uint8_t> out;
  append_u8(out, static_cast<std::uint8_t>(MessageType::NewOrder));
  append_le<OrderId>(out, order.id);
  append_u8(out, static_cast<std::uint8_t>(order.side));
  append_u8(out, static_cast<std::uint8_t>(order.type));
  append_le<Price>(out, order.price);
  append_le<Quantity>(out, order.quantity);
  return out;
}

inline OrderInput decode_new_order(const std::vector<std::uint8_t>& data) {
  std::size_t offset = 0;
  const auto type = static_cast<MessageType>(read_le<std::uint8_t>(data, offset));
  if (type != MessageType::NewOrder) throw std::runtime_error("expected new-order message");
  OrderInput order;
  order.id = read_le<OrderId>(data, offset);
  order.side = static_cast<Side>(read_le<std::uint8_t>(data, offset));
  order.type = static_cast<OrderType>(read_le<std::uint8_t>(data, offset));
  order.price = read_le<Price>(data, offset);
  order.quantity = read_le<Quantity>(data, offset);
  return order;
}

inline std::vector<std::uint8_t> encode_cancel(OrderId id) {
  std::vector<std::uint8_t> out;
  append_u8(out, static_cast<std::uint8_t>(MessageType::CancelOrder));
  append_le<OrderId>(out, id);
  return out;
}

inline CancelInput decode_cancel(const std::vector<std::uint8_t>& data) {
  std::size_t offset = 0;
  const auto type = static_cast<MessageType>(read_le<std::uint8_t>(data, offset));
  if (type != MessageType::CancelOrder) throw std::runtime_error("expected cancel message");
  CancelInput cancel;
  cancel.id = read_le<OrderId>(data, offset);
  return cancel;
}

} // namespace protocol
} // namespace exchange

