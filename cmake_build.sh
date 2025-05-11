#!/bin/bash

# CMake build script for emu8086 emulator

mkdir -p build
cd build

mkdir -p ../examples/output

echo "Configuring with CMake..."
cmake .. 

echo "Building emu8086..."
make

if [ $? -eq 0 ]; then
    echo "Build successful! The executable is at ./build/emu8086"
    
    echo "Usage: ./emu8086 [options]"
    echo "Options:"
    echo "  -i <file>    Input assembly file (default: examples/simple.asm)"
    echo "  -o <file>    Output binary file (default: examples/output/simple.bin)"
    echo "  -d           Disassemble the binary file"
    echo "  -e           Execute the binary file (default)"
    echo "  -h, --help   Show this help message"
else
    echo "Build failed!"
    exit 1
fi 