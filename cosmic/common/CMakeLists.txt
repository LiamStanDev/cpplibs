project(common)

set(SRC process.cc)

add_library(common STATIC ${SRC})
target_include_directories(common PUBLIC .)
