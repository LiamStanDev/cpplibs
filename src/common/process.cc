#include "process.h"

#include <sys/syscall.h> // include all macro of syscall
#include <unistd.h>

namespace cpplibs {

pid_t getThreadId() {
  pid_t id = syscall(SYS_getpid);
  return id;
}

uint32_t getFiberId() { return 0; }

} // namespace cpplibs
