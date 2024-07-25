project(thread)

set(SRC thread.cc sync.cc)

add_library(thread STATIC ${SRC})
target_include_directories(thread PUBLIC .)
target_link_libraries(thread PUBLIC common)
