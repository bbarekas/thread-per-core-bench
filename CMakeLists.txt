cmake_minimum_required(VERSION 3.16)
project(multi_thread)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

enable_language(Rust)
include(CMakeCargo)

include_directories("include")
add_subdirectory(rust-mt)

add_executable(multi_thread main.cpp)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

find_package(benchmark REQUIRED)

target_link_libraries(multi_thread PRIVATE rust-mt-lib dl Threads::Threads benchmark::benchmark)

