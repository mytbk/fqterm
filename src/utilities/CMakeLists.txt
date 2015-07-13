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

include_directories(
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

if(USE_QT5)
  set(CMAKE_AUTOMOC ON)
  find_package(Qt5Core REQUIRED)
  find_package(Qt5Widgets REQUIRED)
  find_package(Qt5Network REQUIRED)
  target_link_libraries(fqterm_utilities Qt5::Core Qt5::Widgets Qt5::Network)
else(USE_QT5)
  qt4_automoc(${fqterm_utilities_SRCS})
endif(USE_QT5)
