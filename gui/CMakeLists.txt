CMAKE_MINIMUM_REQUIRED(VERSION 3.1)
PROJECT(plorth-gui CXX)

FIND_PACKAGE(PkgConfig)

SET(CMAKE_CXX_STANDARD 11)

PKG_CHECK_MODULES(GTKMM gtkmm-3.0 REQUIRED)

LINK_DIRECTORIES(
  ${GTKMM_LIBRARY_DIRS}
)

ADD_EXECUTABLE(
  plorth-gui
  src/context.cpp
  src/dictionary-display.cpp
  src/line-display.cpp
  src/line-editor.cpp
  src/main.cpp
  src/stack-display.cpp
  src/utils.cpp
  src/window.cpp
  ../cli/src/utils.cpp
)

TARGET_COMPILE_OPTIONS(
  plorth-gui
  PRIVATE
    -Wall -Werror
)

TARGET_COMPILE_FEATURES(
  plorth-gui
  PRIVATE
    cxx_std_11
)

TARGET_INCLUDE_DIRECTORIES(
  plorth-gui
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${GTKMM_INCLUDE_DIRS}
)

TARGET_LINK_LIBRARIES(
  plorth-gui
  plorth
  ${GTKMM_LIBRARIES}
)

INSTALL(
  TARGETS
    plorth-gui
  RUNTIME DESTINATION
    bin
)
