set(fqterm_common_SRCS
  common.h
  common.cpp
  fqterm.h
  fqterm_trace.h
  fqterm_trace.cpp
  fqterm_config.h
  fqterm_config.cpp
  fqterm_sound.h
  fqterm_sound.cpp
  fqterm_param.h
  fqterm_param.cpp
  fqterm_path.h
  fqterm_path.cpp
  fqterm_font.h
  fqterm_font.cpp
  fqterm_exif_extractor.h
  fqterm_exif_extractor.cpp
  fqterm_filedialog.h
  fqterm_filedialog.cpp
  fqterm_shortcuthelper.h
  fqterm_shortcuthelper.cpp
)

if(USE_QT5)
  set(CMAKE_INCLUDE_CURRENT_DIR ON)
  set(CMAKE_AUTOMOC ON)
  find_package(Qt5Core REQUIRED)
  find_package(Qt5Widgets REQUIRED)
  find_package(Qt5Multimedia REQUIRED)
else(USE_QT5)
  qt4_automoc(${fqterm_common_SRCS})
  include_directories(
    ${QT_INCLUDE_DIR}
    ${QT_QTCORE_INCLUDE_DIR}
    ${QT_QTGUI_INCLUDE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    )
endif(USE_QT5)

add_library(fqterm_common
  ${fqterm_common_SRCS}
)

if(USE_QT5)
  target_link_libraries(fqterm_common Qt5::Core Qt5::Widgets Qt5::Multimedia)
endif(USE_QT5)
  