project(fqterm)

set(FQTERM_VERSION "0.9.8")
set(CMAKE_VERBOSE_MAKEFILE ON)
cmake_minimum_required(VERSION 3.0.0 FATAL_ERROR)

if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

set(CMAKE_MODULE_PATH
  ${CMAKE_SOURCE_DIR}/cmake
)

set(CMAKE_COLOR_MAKEFILE ON)

include(GetGitRevisionDescription)
git_describe(MCGGIT_WC_REVISION)
message("Git revision: " ${MCGGIT_WC_REVISION})

SET(USE_QT5 OFF CACHE PATH "Do the Qt5 build")

IF(USE_QT5)
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
ELSE(USE_QT5)
  find_package(Qt4 COMPONENTS QtCore QtGui QtNetwork QtScript QtXml REQUIRED)
  include(${QT_USE_FILE})
  qt4_add_translation(fqterm_qm_files
    ${CMAKE_SOURCE_DIR}/res/dict/fqterm_zh_CN.ts
    ${CMAKE_SOURCE_DIR}/res/dict/ui_zh_CN.ts
    ${CMAKE_SOURCE_DIR}/res/dict/protocol_zh_CN.ts
    ${CMAKE_SOURCE_DIR}/res/dict/common_zh_CN.ts
    )
ENDIF(USE_QT5)

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

if (NOT USE_QT5)
  IF(WIN32)
    FILE(READ ${QT_LIBRARY_DIR}/QtCore.prl FQ_CORE_QT_CONFIG)
  ELSE(WIN32)
    IF(APPLE)
      FILE(READ ${QT_LIBRARY_DIR}/QtCore.framework/QtCore.prl FQ_CORE_QT_CONFIG)
    ELSE(APPLE)
      FILE(READ ${QT_LIBRARY_DIR}/libQtCore.prl FQ_CORE_QT_CONFIG)
    ENDIF(APPLE)
  ENDIF(WIN32)

  IF(NOT FQ_CORE_QT_CONFIG MATCHES "static")
    SET(FQTERM_USE_STATIC_QT false)
  ELSE(NOT FQ_CORE_QT_CONFIG MATCHES "static")
    SET(FQTERM_USE_STATIC_QT true)
  ENDIF(NOT FQ_CORE_QT_CONFIG MATCHES "static")
endif(NOT USE_QT5)

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
  fqterm_unite
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
