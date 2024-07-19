#pragma once

#include <pthread.h>
#include <cstdint>

namespace cpplibs {
namespace common {

pid_t getThreadId();

uint32_t getFiberId();

}
} // namespace cpplibs
