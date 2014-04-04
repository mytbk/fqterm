set(fqterm_utilities_SRCS
  fqterm_convert.h
  fqterm_http.h
  fqterm_ip_location.h
  fqterm_convert.cpp
  fqterm_http.cpp
  fqterm_ip_location.cpp
)

set(fq_SRCS
  fqwcwidth.h
  fqwcwidth.cpp
)

qt4_automoc(${fqterm_utilities_SRCS})

include_directories(
  ${QT_INCLUDE_DIR}
  ${QT_QTCORE_INCLUDE_DIR}
  ${QT_QTGUI_INCLUDE_DIR}
  ${QT_QTNETWORK_INCLUDE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/../common
  ${CMAKE_CURRENT_BINARY_DIR}
)

add_library(fqterm_utilities
  ${fqterm_utilities_SRCS}
  ${fq_SRCS}
)

add_dependencies(fqterm_utilities
  fqterm_common
)
