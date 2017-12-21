SET(
  PLORTH_INTERPRETER_SOURCES
  compiler.cpp
  context.cpp
  globals.cpp
  memory.cpp
  module.cpp
  position.cpp
  runtime.cpp
  unicode.cpp
  utils.cpp
  value.cpp
  value-array.cpp
  value-boolean.cpp
  value-error.cpp
  value-number.cpp
  value-object.cpp
  value-quote.cpp
  value-string.cpp
  value-symbol.cpp
  value-word.cpp
)

ADD_LIBRARY(
  plorth
  SHARED
  ${PLORTH_INTERPRETER_SOURCES}
)

IF(NOT PLORTH_ENABLE_TARGET_WEBASSEMBLY)
  INSTALL(
    TARGETS
      plorth
    LIBRARY DESTINATION
      lib
  )
  INSTALL(
    DIRECTORY ${CMAKE_SOURCE_DIR}/include/
    DESTINATION include
    FILES_MATCHING PATTERN "*.hpp"
  )
  INSTALL(
    DIRECTORY ${CMAKE_SOURCE_DIR}/runtime/
    DESTINATION lib/plorth
    FILES_MATCHING PATTERN "*.plorth"
  )
ENDIF()
