#pragma once

#include <cstddef>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

namespace exchange {

template <typename T>
class MemoryPool {
 public:
  explicit MemoryPool(std::size_t capacity)
      : storage_(capacity), next_(capacity), free_head_(0) {
    for (std::size_t i = 0; i < capacity; ++i) {
      next_[i] = i + 1;
    }
  }

  MemoryPool(const MemoryPool&) = delete;
  MemoryPool& operator=(const MemoryPool&) = delete;

  template <typename... Args>
  T* create(Args&&... args) {
    if (free_head_ >= storage_.size()) {
      throw std::bad_alloc();
    }
    const std::size_t index = free_head_;
    free_head_ = next_[index];
    void* slot = &storage_[index];
    return new (slot) T(std::forward<Args>(args)...);
  }

  void destroy(T* ptr) {
    if (!ptr) return;
    const std::size_t index =
        static_cast<std::size_t>(reinterpret_cast<Slot*>(ptr) - storage_.data());
    ptr->~T();
    next_[index] = free_head_;
    free_head_ = index;
  }

  std::size_t capacity() const { return storage_.size(); }
  std::size_t available() const {
    std::size_t count = 0;
    for (std::size_t cursor = free_head_; cursor < storage_.size(); cursor = next_[cursor]) {
      ++count;
    }
    return count;
  }

 private:
  using Slot = typename std::aligned_storage<sizeof(T), alignof(T)>::type;

  std::vector<Slot> storage_;
  std::vector<std::size_t> next_;
  std::size_t free_head_;
};

} // namespace exchange
