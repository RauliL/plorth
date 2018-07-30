CMAKE_MINIMUM_REQUIRED(VERSION 3.0)
PROJECT(plorth-cli C CXX)

INCLUDE_DIRECTORIES(
  ${CMAKE_CURRENT_SOURCE_DIR}/../libplorth/include
  ${CMAKE_CURRENT_SOURCE_DIR}/ext/linenoise
)

ADD_EXECUTABLE(
  plorth-cli
  ext/linenoise/linenoise.c
  src/main.cpp
  src/repl.cpp
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

