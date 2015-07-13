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

include_directories(
  ${OPENSSL_INCLUDE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/internal
  ${CMAKE_CURRENT_SOURCE_DIR}/../common
  ${CMAKE_CURRENT_BINARY_DIR}
)

add_library(fqterm_unite
  ${fqunite_SRCS}
  ${fqunite_internal_SRCS}
)

if(USE_QT5)
  set(CMAKE_AUTOMOC ON)
  find_package(Qt5Core REQUIRED)
  find_package(Qt5Widgets REQUIRED)
  find_package(Qt5Network REQUIRED)
  find_package(Qt5Xml REQUIRED)
  target_link_libraries(fqterm_unite
    Qt5::Core
    Qt5::Widgets
    Qt5::Network
    Qt5::Xml
    )
else(USE_QT5)
  qt4_automoc(
    ${fqunite_SRCS}
    ${fqunite_internal_SRCS}
    )
endif(USE_QT5)

add_dependencies(fqterm_unite
  fqterm_common
)

  
