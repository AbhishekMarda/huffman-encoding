cmake_minimum_required(VERSION 3.15)
project(huffman VERSION 1.0 LANGUAGES CXX)
set(CMAKE_C_COMPILER g++)
set(CMAKE_CXX_COMPILER g++)

include(CTest)
enable_testing()

add_subdirectory(src)
