#pragma once

#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <process.h>
#include <windows.h>

#include <functional>
#include <stdexcept>
#else
#include <chrono>
#include <mutex>
#include <thread>
#endif

namespace exchange {

#if defined(_WIN32)

class Mutex {
 public:
  Mutex() { InitializeCriticalSection(&section_); }
  ~Mutex() { DeleteCriticalSection(&section_); }
  Mutex(const Mutex&) = delete;
  Mutex& operator=(const Mutex&) = delete;
  void lock() { EnterCriticalSection(&section_); }
  void unlock() { LeaveCriticalSection(&section_); }

 private:
  CRITICAL_SECTION section_;
};

class LockGuard {
 public:
  explicit LockGuard(Mutex& mutex) : mutex_(mutex) { mutex_.lock(); }
  ~LockGuard() { mutex_.unlock(); }
  LockGuard(const LockGuard&) = delete;
  LockGuard& operator=(const LockGuard&) = delete;

 private:
  Mutex& mutex_;
};

class Thread {
 public:
  Thread() : handle_(nullptr) {}
  ~Thread() {
    if (handle_) CloseHandle(handle_);
  }

  Thread(const Thread&) = delete;
  Thread& operator=(const Thread&) = delete;

  template <typename Fn>
  void start(Fn fn) {
    auto* heap_fn = new std::function<void()>(fn);
    handle_ = reinterpret_cast<HANDLE>(_beginthreadex(
        nullptr, 0, &Thread::entry, heap_fn, 0, nullptr));
    if (!handle_) {
      delete heap_fn;
      throw std::runtime_error("failed to start thread");
    }
  }

  bool joinable() const { return handle_ != nullptr; }

  void join() {
    if (!handle_) return;
    WaitForSingleObject(handle_, INFINITE);
    CloseHandle(handle_);
    handle_ = nullptr;
  }

 private:
  static unsigned __stdcall entry(void* arg) {
    std::function<void()>* fn = static_cast<std::function<void()>*>(arg);
    (*fn)();
    delete fn;
    return 0;
  }

  HANDLE handle_;
};

inline void yield_thread() {
  Sleep(0);
}

inline void sleep_ms(unsigned milliseconds) {
  Sleep(milliseconds);
}

#else

using Mutex = std::mutex;
using LockGuard = std::lock_guard<std::mutex>;

class Thread {
 public:
  Thread() = default;

  template <typename Fn>
  void start(Fn fn) {
    thread_ = std::thread(fn);
  }

  bool joinable() const { return thread_.joinable(); }
  void join() { thread_.join(); }

 private:
  std::thread thread_;
};

inline void yield_thread() {
  std::this_thread::yield();
}

inline void sleep_ms(unsigned milliseconds) {
  std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

#endif

} // namespace exchange

