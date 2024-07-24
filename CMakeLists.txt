cmake_minimum_required(VERSION 3.12)
project(cpplibs LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS true)
set(CMAKE_VERBOSE_MAKEFILE ON)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

# C++ standard
set(CMAKE_CXX_STANDARD_REQUIRED 20)
set(CMAKE_CXX_STANDARD_REQUIRED true)
# use snaitize=address needs libasan, and sanitize=undefined needs libubsan
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -g -Wall -Wextra -fno-omit-frame-pointer -fsanitize=address -fsanitize=undefined -pthread -Wno-unused-function -Wno-unused-parameter")
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -g -Wall -Wextra  -pthread -Wno-unused-function -Wno-unused-parameter -fno-omit-frame-pointer")

# Variable
set(SRC_DIR src)

# sub modules
add_subdirectory(${SRC_DIR}/log)
add_subdirectory(${SRC_DIR}/common)
add_subdirectory(${SRC_DIR}/thread)

add_executable(test_log tests/test_log.cc)
target_include_directories(test_log PUBLIC ${SRC_DIR})
target_link_libraries(test_log PRIVATE common log)

add_executable(test_mutex tests/test_mutex.cc)
target_include_directories(test_mutex PUBLIC  ${SRC_DIR})
target_link_libraries(test_mutex PRIVATE common thread log)



