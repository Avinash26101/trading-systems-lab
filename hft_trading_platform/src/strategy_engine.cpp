#include "hft/strategy_engine.hpp"

namespace hft {

ImbalanceStrategy::ImbalanceStrategy(double threshold, Quantity order_size)
    : threshold_(threshold), order_size_(order_size), next_order_id_(1) {}

Order ImbalanceStrategy::on_book(const OrderBook& book) {
  const TopOfBook top = book.top();
  const double value = book.imbalance();
  Order order;
  if (top.bid_quantity <= 0 || top.ask_quantity <= 0) return order;

  if (value >= threshold_) {
    order.id = next_order_id_++;
    order.side = Side::Buy;
    order.price = top.ask_price;
    order.quantity = order_size_;
    order.reason = "positive order-book imbalance";
    ++buy_signals_;
  } else if (value <= -threshold_) {
    order.id = next_order_id_++;
    order.side = Side::Sell;
    order.price = top.bid_price;
    order.quantity = order_size_;
    order.reason = "negative order-book imbalance";
    ++sell_signals_;
  }
  return order;
}

} // namespace hft

