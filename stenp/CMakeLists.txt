cmake_minimum_required(VERSION 3.15)
project(call_life)

set(CMAKE_CXX_STANDARD 17)

# Set the build output location to a common directory
# set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output)
# set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output)
# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output)

find_package(OpenXLSX REQUIRED)
find_package(IYFC REQUIRED)
find_package(OpenMP REQUIRED)
#find_package(SEAL REQUIRED)

add_executable(call_life call_life.cpp)
target_link_libraries(call_life IYFC::iyfc OpenXLSX::OpenXLSX)
