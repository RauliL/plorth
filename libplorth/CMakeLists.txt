CMAKE_MINIMUM_REQUIRED(VERSION 3.0)
PROJECT(libplorth C CXX)

INCLUDE(CheckIncludeFile)
INCLUDE(CheckFunctionExists)

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
  PLORTH_ENABLE_MUTEXES
  "Enable if you want to implement thread safety with mutexes."
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

CONFIGURE_FILE(
  "${CMAKE_CURRENT_SOURCE_DIR}/include/plorth/config.hpp.in"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/plorth/config.hpp"
)

ADD_LIBRARY(
  plorth
  SHARED
  src/compiler.cpp
  src/context.cpp
  src/dictionary.cpp
  src/exec.cpp
  src/eval.cpp
  src/globals.cpp
  src/io-input.cpp
  src/io-output.cpp
  src/memory.cpp
  src/module.cpp
  src/position.cpp
  src/runtime.cpp
  src/unicode.cpp
  src/utils.cpp
  src/value.cpp
  src/value-array.cpp
  src/value-boolean.cpp
  src/value-error.cpp
  src/value-number.cpp
  src/value-object.cpp
  src/value-quote.cpp
  src/value-string.cpp
  src/value-symbol.cpp
  src/value-word.cpp
)

TARGET_COMPILE_OPTIONS(
  plorth
  PRIVATE
    -Wall -Werror
)

TARGET_COMPILE_FEATURES(
  plorth
  PRIVATE
    cxx_std_11
)

TARGET_INCLUDE_DIRECTORIES(
  plorth
  PUBLIC
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

IF(NOT DEFINED ENV{EMSCRIPTEN})
  INSTALL(
    TARGETS
      plorth
    LIBRARY DESTINATION
      lib
  )
  INSTALL(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
    DESTINATION include
    FILES_MATCHING PATTERN "*.hpp"
  )
  INSTALL(
    DIRECTORY ${CMAKE_SOURCE_DIR}/runtime/
    DESTINATION lib/plorth
    FILES_MATCHING PATTERN "*.plorth"
  )
ENDIF()
