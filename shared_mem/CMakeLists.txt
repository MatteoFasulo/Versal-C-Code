cmake_minimum_required(VERSION 2.8)

project(shared_mem)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2")

link_directories    (${CMAKE_BINARY_DIR})
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/include)

find_package(OpenCV REQUIRED)

add_library(
    smem_alloc SHARED
    src/smem_alloc.cpp
)

add_executable(
    sender
    src/sender.cpp
)

add_executable(
    receiver
    src/receiver.cpp
)

target_link_libraries(
    sender
    smem_alloc
    ${OpenCV_LIBS}
)

target_link_libraries(
    receiver
    smem_alloc
    ${OpenCV_LIBS}
)