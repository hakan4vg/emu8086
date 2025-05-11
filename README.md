# EMU8086 - 8086 Processor Emulator

An emulator for the Intel 8086 microprocessor with an integrated assembler and disassembler.

## Features

- Full 8086 CPU emulation with registers, flags, and memory model
- Segmented memory architecture (1MB address space)
- Support for most 8086 instructions including:
  - Data transfer (MOV)
  - Arithmetic operations (ADD, SUB, INC, DEC)
  - Logical operations (AND, OR, XOR)
  - String operations (MOVS, CMPS, STOS, LODS, SCAS)
  - Control flow (JMP, conditional jumps)
  - Stack operations (PUSH, POP)
  - I/O operations (IN, OUT)
- Integrated assembler to convert assembly to machine code
- Integrated disassembler to convert machine code back to assembly
- Simple I/O emulation through port-based interface

## Project Structure

```
emu8086/
  ├── assembler/     - 8086 assembly language parser and encoder
  ├── cpu/           - CPU emulation, registers, memory, flags, instructions
  ├── disassembler/  - Binary to assembly conversion
  ├── examples/      - Sample assembly programs
  │   └── output/    - Compiled binary outputs
  ├── io/            - I/O port controller and handlers
  ├── utils/         - Utility functions and helpers
  ├── build.sh       - Shell script for quick building with g++
  ├── cmake_build.sh - Shell script for building with CMake
  ├── CMakeLists.txt - CMake configuration
  ├── main.cpp       - Main program entry point
  └── README.md      - This file
```

## Current Project Status

- ✅ **Core CPU Components**: Registers, flags, memory model
- ✅ **Memory Management**: Full segmented memory with bounds checking
- ✅ **Instruction Set**: Support for basic instructions (MOV, ADD, SUB, CMP, JMP, etc.)
- ✅ **Assembler**: Parse assembly code to binary
- ✅ **Disassembler**: Convert binary back to readable assembly
- ✅ **I/O System**: Basic INT 10h support for video output
- ✅ **Error Handling**: Fixed memory errors and bounds checking
- ⏳ **Additional Instructions**: Working on more complex instructions
- ⏳ **Performance Tracking**: Cycle counting, timing emulation
- ⏳ **Debugger**: Breakpoints, memory inspection, step execution

For more details about future plans, see the [roadmap](roadmap.md).

## Building

### Using the Build Scripts

For convenience, build scripts are provided:

```bash
# Make the build script executable
chmod +x build.sh

# Using the included shell script (recommended)
./build.sh

# Or if you prefer using CMake
chmod +x cmake_build.sh
./cmake_build.sh
```

### Manual Build Using CMake

The project uses CMake for building:

```bash
mkdir -p build
cd build
cmake ..
make
```

### Manual Build Using G++ Directly

If CMake is not available, you can build directly with g++:

```bash
g++ -std=c++17 -Wall -Wextra -g -O2 -o emu8086 \
    main.cpp \
    cpu/*.cpp \
    utils/*.cpp \
    io/*.cpp \
    assembler/*.cpp \
    disassembler/*.cpp
```

## Usage

The emulator provides a unified command-line interface with several options:

```
./emu8086 [options]
```

### Options

- `-i <file>`: Input assembly file (default: examples/simple.asm)
- `-o <file>`: Output binary file (default: examples/output/simple.bin)
- `-d`: Disassemble the binary file
- `-e`: Execute the binary file (default)
- `-h, --help`: Show help message

### Example Commands

1. **Assemble and execute an assembly file in one step:**
   ```
   ./emu8086 -i examples/hello.asm -o examples/output/hello.bin
   ```

2. **Just assemble without executing:**
   ```
   ./emu8086 -i examples/test.asm -o examples/output/test.bin -e false
   ```

3. **Disassemble a binary file:**
   ```
   ./emu8086 -i examples/output/hello.bin -d
   ```

4. **Disassemble and execute at the same time:**
   ```
   ./emu8086 -i examples/test.asm -o examples/output/test.bin -d -e
   ```

### Assembly Syntax

The assembler supports standard 8086 assembly syntax:

```assembly
; Comments start with semicolon

; Data movement
MOV AX, 1234h    ; Load immediate value to register
MOV BX, AX       ; Move register to register
MOV [SI], AX     ; Move register to memory
MOV AX, [BX+SI]  ; Move memory to register with complex addressing

; Arithmetic
ADD AX, BX       ; Add register to register
SUB CX, 5        ; Subtract immediate from register
INC DX           ; Increment register
CMP AX, BX       ; Compare values, set flags

; Control flow
JMP label        ; Unconditional jump
JE label         ; Jump if equal (ZF=1)
CMP AX, 0        ; Compare AX with 0
JNE not_zero     ; Jump if not equal (ZF=0)

; Interrupts
INT 10h          ; Invoke interrupt (e.g., for video output)

; Data definition
DB 65, 66, 67    ; Define bytes (ASCII for 'ABC')

; Halting execution
HLT              ; Halt the CPU
```

### Video Output

The emulator supports basic text output through INT 10h, function 0Eh (teletype):

```assembly
; Display character 'A' using INT 10h function 0Eh
MOV AX, 0E41h    ; AH=0E (teletype function), AL='A' (ASCII code)
INT 10h          ; Call video BIOS interrupt
```

## Examples

Sample assembly programs are provided in the `examples` directory:

- `simple.asm`: Displays "Hello!" using INT 10h
- `counter.asm`: Counts from 1 to 5 and displays the digits
- `hello.asm`: Basic test of register operations and jump
- More examples in the `examples` folder

All examples can be assembled and run using:

```bash
./emu8086 -i examples/simple.asm
```

## Recent Improvements

- Added support for 8-bit register immediate moves (MOV AH, 0x0E, MOV AL, 'P', etc.)
- Fixed character literal handling in the assembler
- Added direct encoding for shift/rotate instructions (SHL, SHR, ROL, ROR, RCL, RCR, SAL, SAR)
- Improved disassembly of MOV register immediate instructions
- Added support for logical operations (AND, OR, XOR)
- Fixed flag handling for CLC, STC, CMC instructions

## Memory Usage and Fixes

The emulator includes fixes for memory management issues:

1. Added proper bounds checking in the disassembler
2. Improved error handling for malformed input
3. Added proper cleanup of resources when processing files
4. Fixed edge case handling for out-of-bounds memory access

## Recent Codebase Improvements

The codebase has recently undergone the following improvements:

1. Documentation reorganization:
   - Updated and expanded README.md with comprehensive project information
   - Consolidated development roadmap into a single, structured roadmap.md file

2. Code quality improvements:
   - Removed redundant and unnecessary comments throughout the codebase
   - Maintained only essential comments that explain complex logic
   - Improved code readability in both assembler and disassembler components

3. Project structure cleanup:
   - Removed redundant or obsolete documentation files
   - Organized example assembly files in the examples directory

## License

This project is open source. Feel free to use and modify.
