set(fqterm_SRCS
  fqterm_app.h
  fqterm_frame.h
  fqterm_screen.h
  fqterm_window.h
  fqterm_wndmgr.h
  fqterm_app.cpp
  fqterm_frame.cpp
  fqterm_screen.cpp
  fqterm_window.cpp
  fqterm_wndmgr.cpp
  fqterm_python.h
  fqterm_python.cpp
  fqterm_scriptengine.h
  fqterm_scriptengine.cpp
)

IF(GLOBAL_HOTKEY EQUAL 1)
IF(WIN32)
  set(global_shortcut_impl_SRCS 3rdparty/qxtglobalshortcut_win.cpp)
ELSE(WIN32)
  IF(APPLE)
    set(global_shortcut_impl_SRCS 3rdparty/qxtglobalshortcut_mac.cpp)
  ELSE(APPLE)
    set(global_shortcut_impl_SRCS 3rdparty/qxtglobalshortcut_x11.cpp)
    IF(USE_QT5)
      find_package(Qt5X11Extras REQUIRED)
      set(QXTLIB Qt5::X11Extras)
    ENDIF(USE_QT5)
  ENDIF(APPLE)
ENDIF(WIN32)
set(GLOBAL_SHORTCUT_SRC
  ${global_shortcut_impl_SRCS}
  3rdparty/qxtglobalshortcut.cpp
  3rdparty/qxtglobalshortcut.h
  3rdparty/qxtglobalshortcut_p.h
  3rdparty/qxtglobal.h
)
ADD_DEFINITIONS(-DUSE_GLOBAL_HOTKEY)
ENDIF(GLOBAL_HOTKEY EQUAL 1)
  

set(3rdparty_SRCS
  3rdparty/dotnetstyle.cpp
  3rdparty/dotnetstyle.h
  3rdparty/uaocodec.h
  3rdparty/uaocodec.cpp
  ${GLOBAL_SHORTCUT_SRC}
)

if(USE_QT5)
  set(CMAKE_AUTOMOC ON)
  find_package(Qt5Core REQUIRED)
  find_package(Qt5Widgets REQUIRED)
  find_package(Qt5Network REQUIRED)
  find_package(Qt5Script REQUIRED)

  qt5_create_translation(fqterm_qm_file
    ${fqterm_SRCS}
    ${CMAKE_SOURCE_DIR}/res/dict/fqterm_zh_CN.ts
    )
else(USE_QT5)
  qt4_automoc(${fqterm_SRCS}
    ${3rdparty_SRCS}
    )

  qt4_create_translation(fqterm_qm_file
    ${fqterm_SRCS}
    ${CMAKE_SOURCE_DIR}/res/dict/fqterm_zh_CN.ts
    )
endif(USE_QT5)

include_directories(
  ${QT_INCLUDE_DIR}
  ${QT_QTCORE_INCLUDE_DIR}
  ${QT_QTGUI_INCLUDE_DIR}
  ${QT_QTNETWORK_INCLUDE_DIR}
  ${QT_QTSCRIPT_INCLUDE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/../common
  ${CMAKE_CURRENT_SOURCE_DIR}/../protocol
  ${CMAKE_CURRENT_SOURCE_DIR}/../terminal
  ${CMAKE_CURRENT_SOURCE_DIR}/../utilities
  ${CMAKE_CURRENT_SOURCE_DIR}/../unite
  ${CMAKE_CURRENT_SOURCE_DIR}/../ui
  ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty
  ${CMAKE_CURRENT_BINARY_DIR}/../ui
  ${CMAKE_CURRENT_BINARY_DIR}
)

add_library(fqterm_main
  ${fqterm_SRCS}
  ${3rdparty_SRCS}
)

add_dependencies(fqterm_main
  fqterm_common
  fqterm_terminal
  fqterm_protocol
  fqterm_ui
  fqterm_utilities
  fqterm_unite
)

if(USE_QT5)
  target_link_libraries(
    fqterm_main
    Qt5::Core
    Qt5::Widgets
    Qt5::Network
    Qt5::Script
    ${QXTLIB}
    )
endif(USE_QT5)
