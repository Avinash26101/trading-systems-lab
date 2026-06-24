#include "exchange/order_book.hpp"

#include <chrono>
#include <iostream>
#include <random>
#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

static std::uint64_t now_ns() {
#if defined(_WIN32)
  static LARGE_INTEGER frequency = [] {
    LARGE_INTEGER value;
    QueryPerformanceFrequency(&value);
    return value;
  }();
  LARGE_INTEGER counter;
  QueryPerformanceCounter(&counter);
  return static_cast<std::uint64_t>(
      (static_cast<long double>(counter.QuadPart) * 1000000000.0L) /
      static_cast<long double>(frequency.QuadPart));
#else
  return static_cast<std::uint64_t>(
      std::chrono::duration_cast<std::chrono::nanoseconds>(
          std::chrono::steady_clock::now().time_since_epoch())
          .count());
#endif
}

int main() {
  const int orders = 100000;
  exchange::OrderBook book(static_cast<std::size_t>(orders * 2));
  exchange::LatencyRecorder latency;
  std::mt19937 rng(42);
  std::uniform_int_distribution<int> side_dist(0, 1);
  std::uniform_int_distribution<int> price_dist(9990, 10010);
  std::uniform_int_distribution<int> qty_dist(1, 20);
  const int batch_size = 64;

  const auto begin = std::chrono::high_resolution_clock::now();
  for (int i = 1; i <= orders; i += batch_size) {
    const auto batch_begin = now_ns();
    int in_batch = 0;
    for (; in_batch < batch_size && i + in_batch <= orders; ++in_batch) {
      exchange::OrderInput input;
      input.id = static_cast<exchange::OrderId>(i + in_batch);
      input.side = side_dist(rng) == 0 ? exchange::Side::Buy : exchange::Side::Sell;
      input.type = exchange::OrderType::Limit;
      input.price = price_dist(rng);
      input.quantity = static_cast<exchange::Quantity>(qty_dist(rng));
      book.add_order(input);
    }
    const auto batch_end = now_ns();
    const auto batch_ns = batch_end - batch_begin;
    latency.record(static_cast<std::uint64_t>(batch_ns / (in_batch > 0 ? in_batch : 1)));
  }
  const auto end = std::chrono::high_resolution_clock::now();
  const double seconds = std::chrono::duration<double>(end - begin).count();
  const double throughput = static_cast<double>(orders) / seconds;

  std::cout << "orders=" << orders << "\n";
  std::cout << "throughput_ops_per_sec=" << static_cast<std::uint64_t>(throughput) << "\n";
  std::cout << "p50_ns=" << latency.percentile(50.0) << "\n";
  std::cout << "p99_ns=" << latency.percentile(99.0) << "\n";
  std::cout << "p999_ns=" << latency.percentile(99.9) << "\n";
  return 0;
}
