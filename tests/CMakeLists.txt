find_package(Qt5Test REQUIRED)
find_package(Qt5Network REQUIRED)
find_package(Qt5Widgets REQUIRED)

add_executable(fqterm_http_test fqterm_http_test.cpp)
target_include_directories(fqterm_http_test PRIVATE
  ${CMAKE_SOURCE_DIR}/src/utilities
  ${CMAKE_SOURCE_DIR}/src/common)
target_link_libraries(fqterm_http_test PRIVATE
  fqterm_utilities fqterm_common Qt5::Test Qt5::Network Qt5::Widgets)
target_compile_definitions(fqterm_http_test PRIVATE FQTERM_TEST_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
add_test(NAME fqterm_http COMMAND fqterm_http_test -o "${CMAKE_CURRENT_BINARY_DIR}/http-test-results.txt,txt")
set_tests_properties(fqterm_http PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen" TIMEOUT 60)

# Explicit real-site verification; never run this target's program from CTest.
add_executable(fqterm_http_probe fqterm_http_probe.cpp)
target_include_directories(fqterm_http_probe PRIVATE
  ${CMAKE_SOURCE_DIR}/src/utilities ${CMAKE_SOURCE_DIR}/src/common)
target_link_libraries(fqterm_http_probe PRIVATE
  fqterm_utilities fqterm_common Qt5::Network Qt5::Widgets)
