CMAKE_MINIMUM_REQUIRED(VERSION 3.0)
PROJECT(plorth C CXX)

OPTION(
  PLORTH_ENABLE_CLI
  "Whether CLI interpreter should be built or not."
  ON
)

OPTION(
  PLORTH_ENABLE_GUI
  "Whether GUI interpreter should be built or not."
  OFF
)

OPTION(
  PLORTH_ENABLE_WEBASSEMBLY
  "Whether WebAssembly interpreter should be built or not."
  OFF
)

ADD_SUBDIRECTORY(libplorth)
IF(PLORTH_ENABLE_CLI)
  ADD_SUBDIRECTORY(cli)
ENDIF()
IF(PLORTH_ENABLE_GUI)
  ADD_SUBDIRECTORY(gui)
ENDIF()
IF(PLORTH_ENABLE_WEBASSEMBLY)
  ADD_SUBDIRECTORY(webassembly)
ENDIF()
