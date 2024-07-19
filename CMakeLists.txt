cmake_minimum_required(VERSION 3.12)
project(cpplibs LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS true)
set(CMAKE_VERBOSE_MAKEFILE ON)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

# C++ standard
set(CMAKE_CXX_STANDARD_REQUIRED 20)
set(CMAKE_CXX_STANDARD_REQUIRED true)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -g -Wall -Wextra -pthread -Wno-unused-function -Wno-unused-parameter")

# Variable
set(SRC_DIR src)

# sub modules
add_subdirectory(${SRC_DIR}/log)
add_subdirectory(${SRC_DIR}/common)

add_executable(test tests/test.cc)
target_link_libraries(test PUBLIC log common)

