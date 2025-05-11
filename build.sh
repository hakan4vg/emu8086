#!/bin/bash
set -e

# Build script for emu8086 emulator
# This script compiles the project directly using g++

# Set compiler options
CXX="g++"
CXXFLAGS="-std=c++17 -Wall -Wextra -g -O2"

# Source files
SOURCES=(
    "main.cpp"
    "cpu/memory.cpp"
    "cpu/instructions.cpp"
    "utils/utils.cpp"
    "io/io.cpp"
    "assembler/assembler.cpp"
    "disassembler/disassembler.cpp"
)

OUTPUT="emu8086"

mkdir -p build
cd build

mkdir -p ../examples/output

echo "Building emu8086..."
${CXX} ${CXXFLAGS} -o ${OUTPUT} ../*.cpp ../cpu/*.cpp ../utils/*.cpp ../io/*.cpp ../assembler/*.cpp ../disassembler/*.cpp

if [ $? -eq 0 ]; then
    echo "Build successful! The executable is at ./build/${OUTPUT}"

else
    echo "Build failed!"
    exit 1
fi

echo "Build successful!"
echo "Usage: ./emu8086 [options]"
echo "Options:"
echo "  -i <file>    Input assembly file (default: examples/simple.asm)"
echo "  -o <file>    Output binary file (default: examples/output/simple.bin)"
echo "  -d           Disassemble the binary file"
echo "  -e           Execute the binary file (default)"
echo "  -h, --help   Show this help message" 