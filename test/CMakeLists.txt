#
# Copyleft
#

set(EXAMPLE_SRCS
  example.c)

add_executable(example ${EXAMPLE_SRCS})
target_link_libraries(example cconf)
# set_target_properties(example PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

###############################################################################
# Unit tests
enable_testing()
# add_test(NAME example COMMAND check_main)
add_test(NAME example COMMAND check_main)
