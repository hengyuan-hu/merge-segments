cmake_minimum_required (VERSION 3.3)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

find_package (BOOST REQUIRED)
include_directories (${BOOST_INCLUDE_DIR})

find_package (OpenCV REQUIRED)
include_directories (${OPENCV_INCLUDE_DIR})

file (GLOB SOURCES "./*.cc" "./*.h")
add_executable (segment ${SOURCES})

target_link_libraries(segment ${OpenCV_LIBS})
