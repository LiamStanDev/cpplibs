#pragma once

#include <cstdint>
#include <pthread.h>

namespace cosmic {

pid_t GetProcessId();

pid_t GetThreadId();

uint32_t GetFiberId();

} // namespace cosmic
