set(export_SRCS
  fqterm_socket.cpp
  fqterm_socket.h
  fqterm_telnet_socket.cpp
  fqterm_telnet_socket.h
  fqterm_local_socket.cpp
  fqterm_local_socket.h
  fqterm_ssh_socket.h
  fqterm_ssh_socket.cpp
)

set(internal_SRCS
  internal/crc32.h
  internal/fqterm_serialization.h
  internal/fqterm_ssh_auth.h
  internal/fqterm_ssh_buffer.h
  internal/fqterm_ssh_cipher.h
  internal/fqterm_ssh_const.h
  internal/fqterm_ssh_des.h
  internal/fqterm_ssh_hash.h
  internal/fqterm_ssh_kex.h
  internal/fqterm_ssh2_kex.h
  internal/fqterm_ssh_mac.h
  internal/fqterm_ssh_md5.h
  internal/fqterm_ssh_packet.h
  internal/fqterm_ssh1_packet.h
  internal/fqterm_ssh2_packet.h
  internal/fqterm_ssh_rsa.h
  internal/fqterm_ssh_channel.h
  internal/fqterm_ssh_types.h
  internal/fqterm_ssh_pubkey.h
  internal/crc32.cpp
  internal/fqterm_ssh_auth.cpp
  internal/fqterm_ssh_buffer.cpp
  internal/fqterm_ssh_des.cpp
  internal/fqterm_ssh_kex.cpp
  internal/fqterm_ssh2_kex.cpp
  internal/fqterm_ssh_mac.cpp
  internal/fqterm_ssh_md5.cpp
  internal/fqterm_ssh_packet.cpp
  internal/fqterm_ssh1_packet.cpp
  internal/fqterm_ssh2_packet.cpp
  internal/fqterm_ssh_rsa.cpp
  internal/fqterm_ssh_channel.cpp
)

if(USE_QT5)
  set(CMAKE_AUTOMOC ON)
  find_package(Qt5Core REQUIRED)
  find_package(Qt5Network REQUIRED)
else(USE_QT5)
  qt4_automoc(
    ${export_SRCS}
    ${internal_SRCS}
    )
endif(USE_QT5)

include_directories(
  ${QT_INCLUDE_DIR}
  ${QT_QTCORE_INCLUDE_DIR}
  ${QT_QTNETWORK_INCLUDE_DIR}
  ${OPENSSL_INCLUDE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/internal
  ${CMAKE_CURRENT_SOURCE_DIR}/../common
  ${CMAKE_CURRENT_BINARY_DIR}
)

add_library(fqterm_protocol
  ${export_SRCS}
  ${internal_SRCS}
)

add_dependencies(fqterm_protocol
  fqterm_common
)

if(USE_QT5)
  target_link_libraries(fqterm_protocol Qt5::Core Qt5::Network)
endif(USE_QT5)
