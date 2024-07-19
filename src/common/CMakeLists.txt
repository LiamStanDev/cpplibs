project(common)

add_library(common STATIC process.cc)
target_include_directories(common PUBLIC .)
