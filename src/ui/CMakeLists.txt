set(fqterm_ui_SRCS
  aboutdialog.h
  addrdialog.h
  articledialog.h
  keydialog.h
  msgdialog.h
  prefdialog.h
  quickdialog.h
  schemadialog.h
  soundconf.h
  sshlogindialog.h
  zmodemdialog.h
  aboutdialog.cpp
  addrdialog.cpp
  articledialog.cpp
  keydialog.cpp
  msgdialog.cpp
  prefdialog.cpp
  quickdialog.cpp
  schemadialog.cpp
  soundconf.cpp
  sshlogindialog.cpp
  zmodemdialog.cpp
  osdmessage.h
  osdmessage.cpp
  overlayWidget.h
  overlayWidget.cpp
  pictureflow.h
  pictureflow.cpp
  popupMessage.h
  popupMessage.cpp
  popwidget.h
  popwidget.cpp
  progressBar.h
  progressBar.cpp
  fqterm_time_label.h
  fqterm_time_label.cpp
  fqterm_tool_button.h
  fqterm_tool_button.cpp
  imageviewer.h
  imageviewer.cpp
  shortcutdialog.h
  shortcutdialog.cpp
  statusBar.h
  statusBar.cpp
  fqterm_canvas.h
  fqterm_canvas.cpp
  sitemanager.h
  sitemanager.cpp
  iplookup.h
  iplookup.cpp
  defineescape.h
  defineescape.cpp
)

set(fqterm_ui_gen_HS
  ui_aboutdialog.h
  ui_addrdialog.h
  ui_articledialog.h
  ui_keydialog.h
  ui_msgdialog.h
  ui_prefdialog.h
  ui_quickdialog.h
  ui_schemadialog.h
  ui_sitemanager.h
  ui_soundconf.h
  ui_sshlogin.h
  ui_zmodemdialog.h
  ui_iplookup.h
  ui_defineescape.h
)

set(fqterm_ui_UIS
  aboutdialog.ui
  addrdialog.ui
  articledialog.ui
  keydialog.ui
  msgdialog.ui
  prefdialog.ui
  quickdialog.ui
  schemadialog.ui
  sitemanager.ui
  soundconf.ui
  sshlogin.ui
  zmodemdialog.ui
  iplookup.ui
  defineescape.ui
)

if(USE_QT5)
qt5_wrap_ui(fqterm_ui_UIS_H
  ${fqterm_ui_UIS} 
)

set(CMAKE_AUTOMOC ON)
find_package(Qt5Core REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Network REQUIRED)

qt5_create_translation(fqterm_ui_qm_file
  ${fqterm_ui_UIS}
  ${fqterm_ui_SRCS}
  ${CMAKE_SOURCE_DIR}/res/dict/ui_zh_CN.ts
)
else(USE_QT5)
qt4_wrap_ui(fqterm_ui_UIS_H
  ${fqterm_ui_UIS} 
)

qt4_automoc(
  ${fqterm_ui_SRCS}
  ${fqterm_ui_gen_HS}
)

qt4_create_translation(fqterm_ui_qm_file
  ${fqterm_ui_UIS}
  ${fqterm_ui_SRCS}
  ${CMAKE_SOURCE_DIR}/res/dict/ui_zh_CN.ts
)
endif(USE_QT5)

include_directories(
  ${QT_INCLUDE_DIR}
  ${QT_QTCORE_INCLUDE_DIR}
  ${QT_QTGUI_INCLUDE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/../common
  ${CMAKE_CURRENT_SOURCE_DIR}/../utilities
  ${CMAKE_CURRENT_BINARY_DIR}
)

add_library(fqterm_ui
  ${fqterm_ui_SRCS}
  ${fqterm_ui_gen_HS}
)

add_dependencies(fqterm_ui
  fqterm_common
  fqterm_utilities
)

ADD_DEFINITIONS(-D "FQTERM_GIT_REVISION=\\\"${MCGGIT_WC_REVISION}\\\"")

if(USE_QT5)
  target_link_libraries(fqterm_ui Qt5::Core Qt5::Widgets)
endif(USE_QT5)
