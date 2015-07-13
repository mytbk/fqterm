set(export_SRCS
  fqterm_buffer.h
  fqterm_session.h
  fqterm_text_line.h
  fqterm_session.cpp
  fqterm_buffer.cpp
  fqterm_text_line.cpp
)

set(internal_SRCS
  internal/fqterm_decode.h
  internal/fqterm_telnet.h
  internal/fqterm_zmodem.h
  internal/fqterm_decode.cpp
  internal/fqterm_telnet.cpp
  internal/fqterm_zmodem.cpp
)

include_directories(
  ${QT_INCLUDE_DIR}
  ${QT_QTCORE_INCLUDE_DIR}
  ${QT_QTGUI_INCLUDE_DIR}
  ${QT_QTNETWORK_INCLUDE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/internal
  ${CMAKE_CURRENT_SOURCE_DIR}/../common
  ${CMAKE_CURRENT_SOURCE_DIR}/../utilities
  ${CMAKE_CURRENT_SOURCE_DIR}/../protocol
)

add_library(fqterm_terminal
  ${export_SRCS}
  ${internal_SRCS}
)

add_dependencies(fqterm_terminal
  fqterm_common
  fqterm_protocol
  fqterm_utilities
)

if(USE_QT5)
  set(CMAKE_AUTOMOC ON)
  find_package(Qt5Core REQUIRED)
  find_package(Qt5Network REQUIRED)
  find_package(Qt5Widgets REQUIRED)
  target_link_libraries(fqterm_terminal Qt5::Core Qt5::Widgets Qt5::Network)
else(USE_QT5)
  QT4_AUTOMOC(
    ${export_SRCS}
    ${internal_SRCS}
    )
endif(USE_QT5)
