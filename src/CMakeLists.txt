add_library(
    huffman_tools
    encoder.cpp
    decoder.cpp
    node.cpp
)

target_include_directories(
    huffman_tools
    PUBLIC ${CMAKE_SOURCE_DIR}/include
)

add_executable(huffman main.cpp)
target_link_libraries(huffman PRIVATE huffman_tools)