# log
add_executable(test_log test_log.cc)
target_link_libraries(test_log PRIVATE cosmic)

# mutex
add_executable(test_mutex test_mutex.cc)
target_link_libraries(test_mutex PRIVATE cosmic)

# config
add_executable(test_config test_config.cc)
target_link_libraries(test_config PRIVATE cosmic)
