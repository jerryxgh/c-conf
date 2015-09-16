#
# Copyright (C) 2015 ZhongYing, Inc. All rights reserved.
#
# Author: -  xu guanghui (xugh@zhong-ying.com)
# Changes:
#      -

# This CMake script will delete build directories and files to bring the
# package back to it's distribution state

# Find directories and files that we will want to remove
file(GLOB CMAKEFILES "${CMAKE_BINARY_DIR}/*")

# Place these files and directories into a list
set(DEL ${CMAKEFILES})

# Loop over the directories and delete each one
foreach(D ${DEL})
  message(STATUS "Deleting ${D}")
  if(EXISTS ${D})
    file(REMOVE_RECURSE ${D})
  endif()
endforeach()
