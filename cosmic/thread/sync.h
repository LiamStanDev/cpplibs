#pragma once

#include <cstdint>
#include <exception>
#include <memory>
#include <pthread.h>
#include <semaphore.h>
#include <stdexcept>

namespace cosmic {

class Semaphore {
public:
  Semaphore(uint32_t count = 0);
  ~Semaphore();

  void wait();
  void notify();

private:
  Semaphore(const Semaphore&) = delete;
  Semaphore(Semaphore&&) = delete;
  Semaphore& operator=(const Semaphore&) = delete;

private:
  sem_t m_semaphore;
};

template <class T> struct MutexInner {
  T data;
  pthread_mutex_t mtx;
  MutexInner(T data) : data(std::move(data)) {
    pthread_mutex_init(&mtx, nullptr);
  }
  ~MutexInner() { pthread_mutex_destroy(&mtx); }
};

template <class T> class MutexGuard {
public:
  MutexGuard(MutexInner<T>& inner) : m_inner(inner) {
    int res = pthread_mutex_lock(&m_inner.mtx);
    if (res != 0) {
      throw std::runtime_error("pthread_mutex_lock error");
    }
  }
  ~MutexGuard() noexcept {
    int res = pthread_mutex_unlock(&m_inner.mtx);
    if (res != 0) {
      // throw std::runtime_error("pthread_mutex_lock error");
      std::terminate();
    }
  }

  T& operator*() { return m_inner.data; }
  T* operator->() { return &m_inner.data; }

private:
  MutexInner<T>& m_inner;
};

template <class T> class Mutex {
public:
  Mutex(T data)
      : m_inner(std::unique_ptr<MutexInner<T>>{new MutexInner<T>{data}}) {}
  ~Mutex() {}

  MutexGuard<T> lock() { return MutexGuard<T>{*m_inner}; }

private:
  std::unique_ptr<MutexInner<T>> m_inner;
};

} // namespace cosmic
