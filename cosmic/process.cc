#include "cosmic/process.h"

#include <sys/syscall.h> // include all macro of syscall
#include <unistd.h>

namespace cosmic {

pid_t GetProcessId() {
  pid_t id = syscall(SYS_getpid);
  return id;
}

pid_t GetThreadId() {
  pid_t id = syscall(SYS_gettid);
  return id;
}

uint32_t GetFiberId() { return 0; }

} // namespace cosmic
