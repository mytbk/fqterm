project(fqterm)

set(FQTERM_VERSION "0.9.10.1")
set(CMAKE_VERBOSE_MAKEFILE ON)
cmake_minimum_required(VERSION 3.9)

set(CMAKE_MODULE_PATH
  ${CMAKE_SOURCE_DIR}/cmake
)

set(CMAKE_COLOR_MAKEFILE ON)

IF (("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU") OR ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang"))
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror -Wno-error=deprecated-declarations")
  set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -Werror -Wno-error=deprecated-declarations")
ENDIF ()

include(GetGitRevisionDescription)
git_describe(MCGGIT_WC_REVISION)
IF(MCGGIT_WC_REVISION MATCHES "NOTFOUND")
  message("Version: " ${FQTERM_VERSION})
ELSE(MCGGIT_WC_REVISION MATCHES "NOTFOUND")
  message("Git revision: " ${MCGGIT_WC_REVISION})
ENDIF(MCGGIT_WC_REVISION MATCHES "NOTFOUND")

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
find_package(Qt5Core REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5LinguistTools REQUIRED)
qt5_add_translation(fqterm_qm_files
  ${CMAKE_SOURCE_DIR}/res/dict/fqterm_zh_CN.ts
  ${CMAKE_SOURCE_DIR}/res/dict/ui_zh_CN.ts
  ${CMAKE_SOURCE_DIR}/res/dict/protocol_zh_CN.ts
  ${CMAKE_SOURCE_DIR}/res/dict/common_zh_CN.ts
  )

add_custom_target(fqterm_qm_target
  DEPENDS ${fqterm_qm_files}
)
FIND_PACKAGE(PythonLibs)
if (PYTHONLIBS_FOUND)
  SET(PYTHON_LIB ${PYTHON_LIBRARIES})
  INCLUDE_DIRECTORIES(${PYTHON_INCLUDE_PATH})
  ADD_DEFINITIONS(-DFOUND_PYTHON)
else (PYTHONLIBS_FOUND)
  SET(PYTHON_LIB)
endif (PYTHONLIBS_FOUND)

SET(SUB_CMAKELIST "UNKNOWN")

ADD_DEFINITIONS(-D "FQTERM_VERSION_STRING=\\\"${FQTERM_VERSION}\\\"")

# Before adding src directory, make sure all the necessary definitions
# are added
add_subdirectory(src)

set(FQTERM_LIBS
  fqterm_main
  fqterm_terminal
  fqterm_protocol
  fqterm_utilities
  fqterm_ui
  fqterm_imageviewer
  fqterm_common
  )

IF(WIN32)
  IF(MINGW)
    SET(SUB_CMAKELIST "CMakeLists.txt.mingw")
  ELSE(MINGW)
    SET(SUB_CMAKELIST "CMakeLists.txt.win32")
  ENDIF(MINGW)
ELSE(WIN32)
  IF(APPLE)
    SET(SUB_CMAKELIST "CMakeLists.txt.macos")
  ELSE(APPLE)
    IF(UNIX)
      IF(CMAKE_SYSTEM_NAME MATCHES "Linux|kFreeBSD")
        SET(SUB_CMAKELIST "CMakeLists.txt.linux")
      ELSE(CMAKE_SYSTEM_NAME MATCHES "Linux|kFreeBSD")
		IF(CMAKE_SYSTEM_NAME MATCHES "FreeBSD")
		  SET(SUB_CMAKELIST "CMakeLists.txt.freebsd")
		ENDIF(CMAKE_SYSTEM_NAME MATCHES "FreeBSD")
      ENDIF(CMAKE_SYSTEM_NAME MATCHES "Linux|kFreeBSD")
    ENDIF(UNIX)
  ENDIF(APPLE)
ENDIF(WIN32)

IF(CMAKE_SYSTEM_NAME MATCHES "GNU")
  SET(SUB_CMAKELIST "CMakeLists.txt.linux")
ENDIF(CMAKE_SYSTEM_NAME MATCHES "GNU")

IF(SUB_CMAKELIST MATCHES "UNKNOWN")
  MESSAGE(FATAL_ERROR "Unsupported system type detected!")
ELSE(SUB_CMAKELIST MATCHES "UNKNOWN")
  INCLUDE("${SUB_CMAKELIST}")
ENDIF(SUB_CMAKELIST MATCHES "UNKNOWN")
