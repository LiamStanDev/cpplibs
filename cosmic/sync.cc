#include "cosmic/sync.h"

#include <semaphore.h>
#include <stdexcept>

namespace cosmic {

Semaphore::Semaphore(uint32_t count) {
  // pshared = 0: means semaphore only share in this process.
  // count is the initial value of semaphore.
  int res = sem_init(&m_semaphore, 0, count);

  // TODO: Add log
  if (res != 0) {
    throw std::runtime_error("sem_init error");
  }
}

Semaphore::~Semaphore() { sem_destroy(&m_semaphore); }

void Semaphore::wait() {
  // sem_wait will decrease by 1 semaphore. If semaphore is 0,
  // this operation will block this thread.
  int res = sem_wait(&m_semaphore);

  if (res != 0) {
    throw std::runtime_error("sem_wait error");
  }
}

void Semaphore::notify() {
  // sem_post will increase by 1 semaphore.
  int res = sem_post(&m_semaphore);

  if (res != 0) {
    throw std::runtime_error("sem_post error");
  }
}

} 
