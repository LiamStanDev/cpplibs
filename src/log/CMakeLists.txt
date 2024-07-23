project(log)

add_library(log SHARED log.cc)
target_include_directories(log PUBLIC .)
target_link_libraries(log PRIVATE common)
