#pragma once

#include "exchange/threading.hpp"

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <vector>

namespace exchange {

class LatencyRecorder {
 public:
  void record(std::uint64_t nanoseconds) {
    LockGuard lock(mutex_);
    samples_.push_back(nanoseconds);
  }

  template <typename Fn>
  void time(Fn fn) {
    const auto start = std::chrono::high_resolution_clock::now();
    fn();
    const auto end = std::chrono::high_resolution_clock::now();
    record(static_cast<std::uint64_t>(
        std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count()));
  }

  std::uint64_t percentile(double p) const {
    std::vector<std::uint64_t> copy;
    {
      LockGuard lock(mutex_);
      copy = samples_;
    }
    if (copy.empty()) return 0;
    std::sort(copy.begin(), copy.end());
    const double rank = (p / 100.0) * static_cast<double>(copy.size() - 1);
    return copy[static_cast<std::size_t>(rank + 0.5)];
  }

  std::size_t count() const {
    LockGuard lock(mutex_);
    return samples_.size();
  }

 private:
  mutable Mutex mutex_;
  std::vector<std::uint64_t> samples_;
};

} // namespace exchange
