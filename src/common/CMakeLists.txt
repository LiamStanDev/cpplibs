project(common)

set(SRC process.cc singleton.cc)

add_library(common STATIC ${SRC})
target_include_directories(common PUBLIC .)
