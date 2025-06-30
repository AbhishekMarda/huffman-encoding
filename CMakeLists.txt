cmake_minimum_required(VERSION 3.15)

option(DEBUG "Enable debug mode" ON)
if(DEBUG)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
    add_compile_definitions(DEBUG)
endif()
project(huffman VERSION 1.0 LANGUAGES CXX)
set(CMAKE_C_COMPILER g++)
set(CMAKE_CXX_COMPILER g++)

include(CTest)
enable_testing()

add_subdirectory(src)
add_subdirectory(test)
