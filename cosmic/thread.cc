#include "cosmic/thread.h"

#include "cosmic/process.h"
#include <functional>
#include <pthread.h>
#include <semaphore.h>
#include <stdexcept>

namespace cosmic {

static thread_local Thread* t_thread = nullptr;

Thread& Thread::GetThis() { return *t_thread; }

const std::string& Thread::GetName() { return t_thread->getName(); }

void Thread::SetName(const std::string& name) {
  if (t_thread) {
    t_thread->m_name = name;
  }
}

Thread::Thread(std::function<void()> cb, const std::string& name)
    : m_cb(cb), m_name(name) {
  if (name.empty()) {
    m_name = "UNKNOWN";
  }

  // create thread and pass thread it self as argument
  int res = pthread_create(&m_thread, nullptr, Thread::run, this);

  // TODO: Add log
  if (res != 0) {
    throw std::runtime_error("pthread_create error");
  }

  // ensure thread start then exit constructor
  m_semaphore.wait();
}

Thread::~Thread() {
  if (m_thread) {
    pthread_detach(m_thread);
  }
}

void Thread::join() {
  if (m_thread) {
    int res = pthread_join(m_thread, nullptr);

    // TODO: Add log
    if (res != 0) {
      throw std::runtime_error("pthread_join error");
    }
    m_thread = 0;
  }
}

void* Thread::run(void* args) {
  Thread* thread = reinterpret_cast<Thread*>(args);
  t_thread = thread;

  thread->m_id = GetThreadId();

  // NOTE: Why substr to 15 char
  // the pthread name only has 16 characters space (include end \0).
  pthread_setname_np(thread->m_thread, thread->m_name.substr(0, 15).c_str());

  std::function<void()> cb;
  cb.swap(thread->m_cb);

  thread->m_semaphore.notify();

  cb();

  return 0;
}

} // namespace cosmic
