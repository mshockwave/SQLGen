cmake_minimum_required(VERSION 3.13)
project(sqlgen)

set(CMAKE_CXX_STANDARD 14)

find_package(LLVM REQUIRED CONFIG)

message(STATUS "Using LLVM ${LLVM_PACKAGE_VERSION}")

add_definitions(${LLVM_DEFINITIONS})
include_directories(${LLVM_INCLUDE_DIRS})
link_directories(${LLVM_LIBRARY_DIR})

list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")
include(AddLLVM)

# Handle RTTI stuff, which often leads to error
if(NOT ${LLVM_ENABLE_RTTI})
  if("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang" OR
      "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR
      "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")
    message(STATUS "Disable RTTI")
  elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /GR-")
    message(STATUS "Disable RTTI")
  endif()
  # Do not give any flags for other less widely used
  # compilers
endif()

set(_SOURCE_FILES
    Error.cpp
    sqlgen.cpp
    SQLEmitter.cpp
    SQLInsertEmitter.cpp
    SQLQueryEmitter.cpp
    SQLTableEmitter.cpp
    )

# This is the old way of setting link components
# It fails if we are linking against a shared libLLVM as shipped by most linux distros these days
#set(LLVM_LINK_COMPONENTS
#    Support
#    TableGen
#   )

add_llvm_executable(sqlgen
  ${_SOURCE_FILES})

# This is the correct way of setting link components
# It is aware of the shared libLLVM: https://lists.llvm.org/pipermail/llvm-dev/2017-November/119054.html
llvm_config(sqlgen support tablegen)

set(FILECHECK_PATH "" CACHE FILEPATH "Path to FileCheck.")

if (NOT FILECHECK_PATH)
  # Try to find FileCheck from LLVM first
  find_program(FILECHECK_EXE
    FileCheck
    NO_CACHE)

  if (NOT FILECHECK_EXE)
    # Then try to find the python version, filecheck
    find_program(FILECHECK_EXE
      filecheck
      NO_CACHE)
  endif()

  if (FILECHECK_EXE)
    message(STATUS "Using ${FILECHECK_EXE}")
    set(FILECHECK_PATH ${FILECHECK_EXE})
  endif()
endif()

add_subdirectory(test)
