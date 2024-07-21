#pragma once

#include <cstdint>
#include <pthread.h>

namespace cpplibs {

pid_t getThreadId();

uint32_t getFiberId();

} // namespace cpplibs
