project(fqterm)

set(FQTERM_VERSION "0.9.8")

cmake_minimum_required( VERSION 2.8.11 FATAL_ERROR )

if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

set(CMAKE_MODULE_PATH
  ${CMAKE_SOURCE_DIR}/cmake
)

IF(NOT CMAKE_BUILD_TYPE) 
    SET(CMAKE_BUILD_TYPE "MinsizeRel") 
ENDIF(NOT CMAKE_BUILD_TYPE)

set(CMAKE_COLOR_MAKEFILE ON)

include(GetGitRevisionDescription)
git_describe(MCGGIT_WC_REVISION)
message("Git revision: " ${MCGGIT_WC_REVISION})

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

IF(WIN32)
  SET(SUB_CMAKELIST "CMakeLists.txt.win32")
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
