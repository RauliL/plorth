CMAKE_MINIMUM_REQUIRED(VERSION 3.1)
PROJECT(plorth-webassembly C CXX)

IF(NOT DEFINED ENV{EMSCRIPTEN})
  MESSAGE(FATAL_ERROR "Emscripten environment variable wasn't found.")
ENDIF()

SET(EMSCRIPTEN_CXX_FLAGS "--bind -s WASM=1 -s MODULARIZE=1 -s EXPORT_NAME=\"'Plorth'\"")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${EMSCRIPTEN_CXX_FLAGS}")
SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${EMSCRIPTEN_CXX_FLAGS}")
SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${EMSCRIPTEN_CXX_FLAGS}")

ADD_EXECUTABLE(
  plorth-webassembly
  ../libplorth/src/compiler.cpp
  ../libplorth/src/context.cpp
  ../libplorth/src/dictionary.cpp
  ../libplorth/src/exec.cpp
  ../libplorth/src/eval.cpp
  ../libplorth/src/globals.cpp
  ../libplorth/src/io-input.cpp
  ../libplorth/src/io-output.cpp
  ../libplorth/src/memory.cpp
  ../libplorth/src/module.cpp
  ../libplorth/src/position.cpp
  ../libplorth/src/runtime.cpp
  ../libplorth/src/unicode.cpp
  ../libplorth/src/utils.cpp
  ../libplorth/src/value.cpp
  ../libplorth/src/value-array.cpp
  ../libplorth/src/value-boolean.cpp
  ../libplorth/src/value-error.cpp
  ../libplorth/src/value-number.cpp
  ../libplorth/src/value-object.cpp
  ../libplorth/src/value-quote.cpp
  ../libplorth/src/value-string.cpp
  ../libplorth/src/value-symbol.cpp
  ../libplorth/src/value-word.cpp
  ../cli/src/repl.cpp
  src/main.cpp
)

TARGET_INCLUDE_DIRECTORIES(
  plorth-webassembly
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../libplorth/include
)

SET_TARGET_PROPERTIES(
  plorth-webassembly
  PROPERTIES OUTPUT_NAME plorth
)
