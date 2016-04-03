set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

if(IMAGE_USE_PICFLOW)
  set(IMGVIEW_SRCS
    imageviewer.h
    imageviewer.cpp)
else(IMAGE_USE_PICFLOW)
  set(IMGVIEW_SRCS
    imageviewer_origin.cpp
    imageviewer_origin.h)
endif(IMAGE_USE_PICFLOW)

set(fqterm_imageviewer_SRCS
  fqtermimage.cpp
  fqtermimage.h
  fqterm_canvas.cpp
  fqterm_canvas.h
  pictureflow.cpp
  pictureflow.h
  ${IMGVIEW_SRCS})

include_directories(
  ${QT_INCLUDE_DIR}
  ${QT_QTCORE_INCLUDE_DIR}
  ${QT_QTGUI_INCLUDE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/../../common
  ${CMAKE_CURRENT_BINARY_DIR}
)

add_library(fqterm_imageviewer
  ${fqterm_imageviewer_SRCS})

if(USE_QT5)
  target_link_libraries(fqterm_imageviewer Qt5::Core Qt5::Widgets)
endif(USE_QT5)
