CMAKE_MINIMUM_REQUIRED(VERSION 3.0)
PROJECT(plorth C CXX)

INCLUDE(CheckCXXCompilerFlag)
INCLUDE(CheckIncludeFile)
INCLUDE(CheckFunctionExists)

OPTION(
  PLORTH_ENABLE_TARGET_CLI
  "Enable CLI interpreter."
  ON
)

OPTION(
  PLORTH_ENABLE_TARGET_WEBASSEMBLY
  "Enable WebAssembly interpreter."
  OFF
)

OPTION(
  PLORTH_ENABLE_MODULES
  "Whether modules has been enabled or not."
  ON
)

OPTION(
  PLORTH_ENABLE_SYMBOL_CACHE
  "Whether symbols should be cached or not."
  OFF
)

OPTION(
  PLORTH_ENABLE_INTEGER_CACHE
  "Whether commonly used integer numbers should be cached or not."
  ON
)

OPTION(
  PLORTH_ENABLE_MEMORY_POOL
  "Enable if you want the interpreter to use memory pools."
  ON
)

OPTION(
  PLORTH_ENABLE_32BIT_INT
  "Enable if you want to use 32-bit integers instead of 64-bit."
  OFF
)

OPTION(
  PLORTH_ENABLE_GC_DEBUG
  "Display debug messages from garbage collector."
  OFF
)

CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)

IF(COMPILER_SUPPORTS_CXX11)
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
ELSEIF(COMPILER_SUPPORTS_CXX0X)
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
ELSE()
  MESSAGE(FATAL_ERROR "Compiler ${CMAKE_CXX_COMPILER} has no C++11 support.")
ENDIF()

CHECK_INCLUDE_FILE(unistd.h HAVE_UNISTD_H)
CHECK_INCLUDE_FILE(sysexits.h HAVE_SYSEXITS_H)
CHECK_INCLUDE_FILE(sys/types.h HAVE_SYS_TYPES_H)
CHECK_INCLUDE_FILE(sys/stat.h HAVE_SYS_STAT_H)

CHECK_FUNCTION_EXISTS(fork HAVE_FORK)
CHECK_FUNCTION_EXISTS(isatty HAVE_ISATTY)
CHECK_FUNCTION_EXISTS(stat HAVE_STAT)
CHECK_FUNCTION_EXISTS(realpath HAVE_REALPATH)

IF(PLORTH_ENABLE_MODULES)
  IF(NOT ${HAVE_STAT})
    MESSAGE(FATAL_ERROR "Required function stat() is missing.")
  ELSEIF(NOT ${HAVE_REALPATH})
    MESSAGE(FATAL_ERROR "Required function realpath() is missing.")
  ENDIF()
ENDIF()

CONFIGURE_FILE(
  "${CMAKE_CURRENT_SOURCE_DIR}/include/plorth/config.hpp.in"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/plorth/config.hpp"
)

INCLUDE_DIRECTORIES(
  "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

ADD_SUBDIRECTORY(libplorth)

IF(PLORTH_ENABLE_TARGET_CLI)
  ADD_SUBDIRECTORY(plorth-cli)
ENDIF()

IF(PLORTH_ENABLE_TARGET_WEBASSEMBLY)
  ADD_SUBDIRECTORY(plorth-webassembly)
ENDIF()
