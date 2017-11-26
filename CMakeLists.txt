cmake_minimum_required(VERSION 2.8)

set(EXECUTABLE_NAME "ANPR")
set(SFML_INCLUDE_DIR "/usr/include")

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake_modules" ${CMAKE_MODULE_PATH})

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build (Debug or Release)" FORCE)
endif()

project( ${EXECUTABLE_NAME} )

# OpenCV include stuff
set(CUDA_CUDART_LIBRARY "/usr/local/cuda/lib64/libcudart.so" CACHE INTERNAL "")
find_package( OpenCV 3 REQUIRED )

# build flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -pthread")

include_directories( ${OpenCV_INCLUDE_DIRS} ${SFML_INCLUDE_DIR} )

# get all source files
FILE(GLOB MyCPPSources ANPR/*.cpp)

# build and link
add_executable( ${EXECUTABLE_NAME} ${MyCPPSources} )
target_link_libraries(${EXECUTABLE_NAME} ${OpenCV_LIBS} sfml-system sfml-network stdc++fs)
