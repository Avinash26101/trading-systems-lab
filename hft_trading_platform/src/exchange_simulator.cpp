#include "hft/exchange_simulator.hpp"

namespace hft {

ExchangeSimulator::ExchangeSimulator(double fee_per_share) : fee_per_share_(fee_per_share) {}

std::vector<Fill> ExchangeSimulator::execute(const Order& order, const OrderBook& book) {
  std::vector<Fill> fills;
  const TopOfBook top = book.top();
  if (order.side == Side::Buy && top.ask_quantity > 0 && order.price >= top.ask_price) {
    const Quantity quantity = order.quantity < top.ask_quantity ? order.quantity : top.ask_quantity;
    fills.push_back({order.id, order.side, top.ask_price, quantity});
    fees_ += static_cast<double>(quantity) * fee_per_share_;
  } else if (order.side == Side::Sell && top.bid_quantity > 0 && order.price <= top.bid_price) {
    const Quantity quantity = order.quantity < top.bid_quantity ? order.quantity : top.bid_quantity;
    fills.push_back({order.id, order.side, top.bid_price, quantity});
    fees_ += static_cast<double>(quantity) * fee_per_share_;
  }
  return fills;
}

} // namespace hft

