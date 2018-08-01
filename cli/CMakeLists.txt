CMAKE_MINIMUM_REQUIRED(VERSION 3.0)
PROJECT(plorth-cli C CXX)

ADD_EXECUTABLE(
  plorth-cli
  ext/linenoise/linenoise.c
  src/main.cpp
  src/repl.cpp
)

TARGET_COMPILE_OPTIONS(
  plorth-cli
  PRIVATE
    -Wall -Werror
)

TARGET_COMPILE_FEATURES(
  plorth-cli
  PRIVATE
    cxx_std_11
)

TARGET_INCLUDE_DIRECTORIES(
  plorth-cli
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/ext/linenoise
)

TARGET_LINK_LIBRARIES(
  plorth-cli
  plorth
)

SET_TARGET_PROPERTIES(
  plorth-cli
  PROPERTIES OUTPUT_NAME plorth
)

INSTALL(
  TARGETS
    plorth-cli
  RUNTIME DESTINATION
    bin
)

