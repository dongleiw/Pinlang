cmake_minimum_required(VERSION 3.16.3)

project(pinlang)

file(GLOB_RECURSE sources      src/*.cpp src/*.h)
file(GLOB_RECURSE sources_parser      src_parser/*.cpp src_parser/*.h)
file(GLOB_RECURSE sources_antlr4_runtime      antlr4/cpp_runtime//*.h)
file(GLOB_RECURSE lib antlr4/libantlr4-runtime.a)

add_executable ( pinlang ${sources} ${sources_parser} ${sources_antlr4_runtime})

find_package(Threads REQUIRED)

set (CMAKE_CXX_FLAGS "-pthread")

target_link_libraries ( pinlang  ${lib})
target_compile_options(pinlang PUBLIC -std=c++17 -Wall -Wfloat-conversion -Wfloat-equal -g)

target_include_directories(pinlang PUBLIC "antlr4/cpp_runtime")
target_include_directories(pinlang PUBLIC "src_parser")
#include_directories("antlr4/cpp_runtime_headers")
