project(log)

add_library(log SHARED log.cc)
target_include_directories(log PUBLIC .)
