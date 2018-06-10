CMAKE_MINIMUM_REQUIRED(VERSION 3.1)
PROJECT(plorth-gui C CXX)

FIND_PACKAGE(PkgConfig)

SET(CMAKE_CXX_STANDARD 11)

PKG_CHECK_MODULES(GTKMM gtkmm-3.0)

LINK_DIRECTORIES(
  ${GTKMM_LIBRARY_DIRS}
)

INCLUDE_DIRECTORIES(
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}../libplorth/include
  ${GTKMM_INCLUDE_DIRS}
)

ADD_EXECUTABLE(
  plorth-gui
  src/line-display.cpp
  src/line-editor.cpp
  src/main.cpp
  src/stack-display.cpp
  src/utils.cpp
  src/window.cpp
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
