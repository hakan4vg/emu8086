#!/bin/bash

# Compile the test disassembler
g++ -o disassemble_test examples/disassemble_test.cpp disassembler/disassembler.cpp -I. -std=c++17

# Run it on our test binary
./disassemble_test examples/output/test_mov_cx.bin examples/output/test_mov_cx.dis

# Display the result
cat examples/output/test_mov_cx.dis 