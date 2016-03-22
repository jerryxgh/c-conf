#
# Copyleft
#

set(EXAMPLE_SRCS
  example.c)

# !!!important!!!: target name of add_executable should be globally unique.
# see https://cmake.org/cmake/help/v2.8.11/cmake.html#command:add_executable
# Adds an executable target called <name> to be built from the source files
# listed in the command invocation. The <name> corresponds to the logical target
# name and must be globally unique within a project. The actual file name of the
# executable built is constructed based on conventions of the native platform
# (such as <name>.exe or just <name>).
add_executable(example_bin ${EXAMPLE_SRCS})
target_link_libraries(example_bin cconf)
set_target_properties(example_bin PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

###############################################################################
# Unit tests
if(CHECK_FOUND)
  enable_testing()
  # add_test(NAME example COMMAND check_main)
  add_test(NAME example COMMAND example_bin)
endif()
