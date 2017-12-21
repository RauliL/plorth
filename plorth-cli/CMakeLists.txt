ADD_EXECUTABLE(
  plorth-cli
  main.cpp
  repl.cpp
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

