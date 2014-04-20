set(fqunite_SRCS
  fqterm_mini_server.h
  fqterm_mini_server.cpp
)

set(fqunite_internal_SRCS
  internal/unite.h
  internal/user.h
  internal/user.cpp
  internal/board.h
  internal/board.cpp
  internal/article.h
  internal/article.cpp
  internal/session.h
  internal/session.cpp
  internal/state.h
  internal/state.cpp
  internal/statebuilder.h
  internal/statebuilder.cpp
)

set(CMAKE_AUTOMOC ON)
find_package(Qt5Core REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Network REQUIRED)
find_package(Qt5Xml REQUIRED)

include_directories(
  ${QT_QTXML_INCLUDE_DIR}
  ${OPENSSL_INCLUDE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/internal
  ${CMAKE_CURRENT_SOURCE_DIR}/../common
  ${CMAKE_CURRENT_BINARY_DIR}
)

add_library(fqterm_unite
  ${fqunite_SRCS}
  ${fqunite_internal_SRCS}
)

add_dependencies(fqterm_unite
  fqterm_common
)

target_link_libraries(fqterm_unite
  Qt5::Core
  Qt5::Widgets
  Qt5::Network
  Qt5::Xml
)
