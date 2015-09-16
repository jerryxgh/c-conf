cmake_minimum_required(VERSION 2.8)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

###############################################################################
# Project info
project(cconf C)
set(CCONF_VERSION_MAJOR 0)
set(CCONF_VERSION_MINOR 2)
set(CCONF_VERSION_PATCH 0)

###############################################################################
# Add a distclean target to the Makefile
ADD_CUSTOM_TARGET(distclean COMMAND ${CMAKE_COMMAND}
  -P ${CMAKE_MODULE_PATH}/distclean.cmake)

###############################################################################
# Set a default build type if none was specified
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to 'Release' as none was specified.")
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Release" "Debug")
endif()

set(CMAKE_C_FLAGS_DEBUG "$ENV{CFLAGS} -O0 -W -Wall -Wunused -Werror -Wstrict-prototypes -Wmissing-prototypes -Wmissing-declarations -Wold-style-definition -Wpointer-arith -Wcast-align -Wnested-externs -Wcast-qual -Wformat-security -Wundef -Wwrite-strings -g -ggdb -std=c99")

set(CMAKE_C_FLAGS_RELEASE "$ENV{CFLAGS} -O2 -W -Wall -Wunused -Werror -Wstrict-prototypes -Wmissing-prototypes -Wmissing-declarations -Wold-style-definition -Wpointer-arith -Wcast-align -Wnested-externs -Wcast-qual -Wformat-security -Wundef -Wwrite-strings -std=c99")

###############################################################################
# Generate compile commands
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

###############################################################################
# Coverage option
option(ENABLE_COVERAGE "Use gcov" ON)
message(STATUS ENABLE_COVERAGE=${ENABLE_COVERAGE})

if(ENABLE_COVERAGE)
  SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -fprofile-arcs -ftest-coverage")
endif()

###############################################################################
# Enable unit tests if check is installed
find_package(Check)

if(CHECK_FOUND)
  add_subdirectory(test)
  enable_testing()
endif()

###############################################################################
# include directories
include_directories(BEFORE
  ${CMAKE_SOURCE_DIR}/src
  )

###############################################################################
# Subdirectories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
add_subdirectory(src)
add_subdirectory(test)
