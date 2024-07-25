#pragma once

#include "cosmic/sync.h"
#include <functional>
#include <string>

namespace cosmic {

/**
 * @brief Thread is a pthread wrapper.
 */
class Thread {
public:
  Thread(std::function<void()> cb, const std::string& name);
  ~Thread();

  pid_t getId() const { return m_id; }
  const std::string& getName() const { return m_name; }
  void join();

  // static method is for getting info about current running thread
  static Thread& GetThis();
  static const std::string& GetName();
  // for thread not create by use e.g. main thread
  static void SetName(const std::string& name);

private:
  Thread(const Thread&) = delete;
  Thread(Thread&&) = delete;
  Thread& operator=(const Thread&) = delete;

  // NOTE: Why static run method ?
  // pthread only use static method, it can't use method
  // because method depends on instance which has this pointer
  // in the method first argument. The static method is like
  // global function, so it dosen't use this pointer.
  static void* run(void* args); // for pthread start_routine

private:
  pid_t m_id = -1;            // thread id
  pthread_t m_thread = 0;     // handle
  std::function<void()> m_cb; // call back
  std::string m_name;         // thread name

  Semaphore m_semaphore;
};
} // namespace cosmic
