cmake_minimum_required(VERSION 3.16.3)

set (CMAKE_CXX_FLAGS "-pthread")
set (CMAKE_C_COMPILER "clang-12")
set (CMAKE_CXX_COMPILER "clang++-12")

project(pinlang)

file(GLOB_RECURSE sources      src/*.cpp src/*.h)
file(GLOB_RECURSE sources_parser      src_parser/*.cpp src_parser/*.h)
file(GLOB_RECURSE sources_antlr4_runtime      antlr4/cpp_runtime//*.h)
file(GLOB_RECURSE lib antlr4/libantlr4-runtime.a)

add_executable ( pinlang ${sources} ${sources_parser} ${sources_antlr4_runtime})

find_package(Threads REQUIRED)

target_link_libraries ( pinlang  ${lib} "dl"  -L/usr/lib/llvm-12/lib -lLLVM-12 )
target_compile_options(pinlang PUBLIC -I/usr/lib/llvm-12/include -std=c++17 -Wall -Wfloat-conversion -Wfloat-equal -g)

target_include_directories(pinlang PUBLIC "antlr4/cpp_runtime")
target_include_directories(pinlang PUBLIC "src_parser")
#include_directories("antlr4/cpp_runtime_headers")


#17 target_compile_options(generate_ir PUBLIC  -std=c++14   -fno-exceptions -D_GNU_SOURCE -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS  -Wall -Wfloat-
