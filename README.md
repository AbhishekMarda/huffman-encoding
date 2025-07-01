# Huffman encoder

An implementation of the lossless compression algorithm.

## System requirements
1. Cmake >= 3.15
2. g++ supporting c++17 and above


## Build
Run the following from the root of the repo (assuming Linux based environment)
```sh
mkdir build
cd build
cmake ..
cmake --build .
```

## Encode
Run `build/src/huffman -e -i [input file] -o [output compressed file]`

## Decode
Run `build/src/huffman -d -i [input file] -o [output decompressed file]`