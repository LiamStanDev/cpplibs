project(config)

set(SRC config.cc )

add_library(conig SHARED ${SRC})
target_include_directories(config PUBLIC .)
