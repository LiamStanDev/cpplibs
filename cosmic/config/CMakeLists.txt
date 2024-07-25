project(config)

set(SRC config.cc)

add_library(config SHARED ${SRC})
target_include_directories(config PUBLIC .)
target_link_libraries(config PUBLIC log)
