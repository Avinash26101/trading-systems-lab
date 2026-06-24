#pragma once

#include <atomic>
#include <cstddef>
#include <memory>

namespace exchange {

template <typename T>
class BoundedMpmcQueue {
 public:
  explicit BoundedMpmcQueue(std::size_t capacity)
      : buffer_mask_(round_up_pow2(capacity) - 1),
        buffer_(new Cell[buffer_mask_ + 1]),
        enqueue_pos_(0),
        dequeue_pos_(0) {
    for (std::size_t i = 0; i <= buffer_mask_; ++i) {
      buffer_[i].sequence.store(i, std::memory_order_relaxed);
    }
  }

  BoundedMpmcQueue(const BoundedMpmcQueue&) = delete;
  BoundedMpmcQueue& operator=(const BoundedMpmcQueue&) = delete;

  bool enqueue(const T& data) {
    Cell* cell = nullptr;
    std::size_t pos = enqueue_pos_.load(std::memory_order_relaxed);
    for (;;) {
      cell = &buffer_[pos & buffer_mask_];
      const std::size_t seq = cell->sequence.load(std::memory_order_acquire);
      const intptr_t diff = static_cast<intptr_t>(seq) - static_cast<intptr_t>(pos);
      if (diff == 0) {
        if (enqueue_pos_.compare_exchange_weak(pos, pos + 1, std::memory_order_relaxed)) {
          break;
        }
      } else if (diff < 0) {
        return false;
      } else {
        pos = enqueue_pos_.load(std::memory_order_relaxed);
      }
    }
    cell->data = data;
    cell->sequence.store(pos + 1, std::memory_order_release);
    return true;
  }

  bool dequeue(T& data) {
    Cell* cell = nullptr;
    std::size_t pos = dequeue_pos_.load(std::memory_order_relaxed);
    for (;;) {
      cell = &buffer_[pos & buffer_mask_];
      const std::size_t seq = cell->sequence.load(std::memory_order_acquire);
      const intptr_t diff = static_cast<intptr_t>(seq) - static_cast<intptr_t>(pos + 1);
      if (diff == 0) {
        if (dequeue_pos_.compare_exchange_weak(pos, pos + 1, std::memory_order_relaxed)) {
          break;
        }
      } else if (diff < 0) {
        return false;
      } else {
        pos = dequeue_pos_.load(std::memory_order_relaxed);
      }
    }
    data = cell->data;
    cell->sequence.store(pos + buffer_mask_ + 1, std::memory_order_release);
    return true;
  }

 private:
  struct Cell {
    std::atomic<std::size_t> sequence;
    T data;
  };

  static std::size_t round_up_pow2(std::size_t value) {
    std::size_t result = 1;
    while (result < value) result <<= 1;
    return result;
  }

  const std::size_t buffer_mask_;
  std::unique_ptr<Cell[]> buffer_;
  alignas(64) std::atomic<std::size_t> enqueue_pos_;
  alignas(64) std::atomic<std::size_t> dequeue_pos_;
};

} // namespace exchange

