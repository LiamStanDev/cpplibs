cmake_minimum_required(VERSION 3.12)
project(cosmic LANGUAGES CXX)

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
set(SRC_DIR cosmic)
set(INC_DIR include)
set(DEPS_DIR deps)
set(TEST_DIR tests)

file(GLOB SOURCES "${SRC_DIR}/*.cc")

# sub modules
add_subdirectory(${DEPS_DIR}/yaml-cpp)

# main library
add_library(${PROJECT_NAME} SHARED ${SOURCES})
target_include_directories(${PROJECT_NAME} 
    PUBLIC ${INC_DIR}
    PRIVATE
)
target_link_libraries(${PROJECT_NAME} 
    PUBLIC yaml-cpp
    PRIVATE
)

# test
add_subdirectory(${TEST_DIR})
