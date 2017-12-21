CMAKE_MINIMUM_REQUIRED(VERSION 3.0)
PROJECT(plorth C CXX)

INCLUDE(CheckCXXCompilerFlag)

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

ADD_SUBDIRECTORY(libplorth)

IF(PLORTH_ENABLE_TARGET_CLI)
  ADD_SUBDIRECTORY(plorth)
ENDIF()

IF(PLORTH_ENABLE_TARGET_WEBASSEMBLY)
  ADD_SUBDIRECTORY(plorth-webassembly)
ENDIF()
