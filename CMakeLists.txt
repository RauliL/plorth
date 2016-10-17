CMAKE_MINIMUM_REQUIRED(VERSION 3.0)
PROJECT(plorth C CXX)

INCLUDE(CheckCXXCompilerFlag)
INCLUDE(CheckIncludeFile)
INCLUDE(CheckFunctionExists)

OPTION(
  PLORTH_GC_DEBUG
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

FIND_PATH(
  GMP_INCLUDE_DIR
  NAMES gmp.h
  DOC "The directory containing GMP include files."
)
FIND_PATH(
  GMPXX_INCLUDE_DIR
  NAMES gmpxx.h
  DOC "The directory containing GMPXX include files."
)
FIND_LIBRARY(
  GMP_LIBRARIES
  NAMES gmp libgmp
  DOC "Path to the GMP library."
)
FIND_LIBRARY(
  GMPXX_LIBRARIES
  NAMES gmpxx libgmpxx
  DOC "Path to the GMPXX library."
)
IF(NOT GMP_LIBRARIES OR NOT GMPXX_LIBRARIES)
  MESSAGE(FATAL_ERROR "Could not find GMPXX library.")
ENDIF()

CHECK_INCLUDE_FILE(unistd.h PLORTH_HAVE_UNISTD_H)
CHECK_FUNCTION_EXISTS(isatty PLORTH_HAVE_ISATTY)

CONFIGURE_FILE(
  "${CMAKE_CURRENT_SOURCE_DIR}/src/config.hpp.in"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/config.hpp"
)

INCLUDE_DIRECTORIES(
  "${GMP_INCLUDE_DIR}"
  "${GMPXX_INCLUDE_DIR}"
  "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

ADD_LIBRARY(
  plorth
  SHARED
  src/array.cpp
  src/bool.cpp
  src/error.cpp
  src/io.cpp
  src/memory.cpp
  src/number.cpp
  src/object.cpp
  src/quote.cpp
  src/runtime.cpp
  src/stack.cpp
  src/state.cpp
  src/string.cpp
  src/token.cpp
  src/unicode.cpp
  src/utils.cpp
  src/value.cpp
)

ADD_EXECUTABLE(
  plorth-cli
  src/console.cpp
  src/main.cpp
)

TARGET_LINK_LIBRARIES(
  plorth
  "${GMP_LIBRARIES}"
  "${GMPXX_LIBRARIES}"
)

TARGET_LINK_LIBRARIES(
  plorth-cli
  plorth
)

INSTALL(
  TARGETS
    plorth
    plorth-cli
  RUNTIME DESTINATION
    bin
  LIBRARY DESTINATION
    lib
)
