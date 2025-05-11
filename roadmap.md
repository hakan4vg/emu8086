# 8086 Emulator Project Roadmap

## Project Overview
This project aims to create a realistic 8086 emulator with:
- Full instruction set and registers
- Proper 16-bit segmented memory model
- A CLI tool to run binary and assembly files
- Assembler, disassembler, and an optional debugger
- Accurate memory handling with offsets
- Emulated clock speeds and pipeline behavior
- Performance tracking and additional features

## Current Progress

- [x] Phase 1: Initial Setup
- [x] Phase 2: Core CPU and Memory Emulation
- [x] Phase 3: Instruction Execution Framework
- [x] Phase 4: Command-Line Interface (CLI)
- [x] Phase 5: Assembler & Disassembler
- [x] Phase 6: Expanding Instruction Coverage & Realism
- [ ] Phase 7: Performance Tracking and Monitoring
- [ ] Phase 8: Debugger & GUI Front-End
- [ ] Phase 9: Additional Refinements and Stretch Goals

---

## Completed Phases

### Phase 1: Initial Setup ✓
1. **Project Structure and Build System**
   - Created a clear folder layout
   - Initialized CMake build system
   - Set up code scaffolding for CPU, Memory, and Register modules
2. **Basic Main Function**
   - Drafted a `main()` that initializes core components
   - Ensured the build runs without errors

### Phase 2: Core CPU and Memory Emulation ✓
1. **Register Definitions**
   - Implemented all 8086 registers (AX, BX, CX, DX, SI, DI, SP, BP, CS, DS, SS, ES, IP, FLAGS)
   - Verified correct usage of `union` for high/low bytes
2. **Memory Architecture**
   - Implemented segmented addressing (segment:offset → physical)
   - Reserved a single contiguous block for the emulated memory (1 MB)
   - Provided read/write functions for byte and word
3. **Flags Handling**
   - Implemented individual flags (Carry, Zero, Sign, Overflow, etc.)
   - Defined utility methods to set, clear, and check flags

### Phase 3: Instruction Execution Framework ✓
1. **Instruction Decoding**
   - Created an opcode lookup structure
   - Designed a fetch-decode-execute cycle
2. **Basic Instruction Set**
   - Implemented a core set (MOV, ADD, SUB, INC, DEC)
   - Implemented an instruction dispatch mechanism
3. **Execution Loop**
   - Functional fetch-decode-execute cycle
   - Successfully executed test programs

### Phase 4: Command-Line Interface (CLI) ✓
1. **CLI Scaffolding**
   - Implemented argument parsing (e.g., `emu8086 -i <filename>`)
   - Added support for reading binary or text-based assembly files
2. **File Loading**
   - Added functionality to load `.bin` or assembled machine code into the emulator's memory
   - Integrated with the assembler to process assembly source files

### Phase 5: Assembler & Disassembler ✓
1. **Assembler**
   - Implemented parser for basic 8086 assembly syntax
   - Created functionality to convert mnemonics and operands into machine opcodes
   - Added support for outputting binary files for the emulator
2. **Disassembler**
   - Implemented functionality to read binary files and convert to readable assembly
   - Added correct interpretation of opcodes into mnemonics
3. **Integration**
   - Extended the CLI with commands for assembling, disassembling, and executing

### Phase 6: Expanding Instruction Coverage & Realism ✓
1. **Extended Instruction Set**
   - Implemented most 8086 instructions
   - Fixed and enhanced flag behavior and addressing modes
2. **I/O Handling**
   - Implemented port-based I/O emulation
   - Mapped INT 10h to console text output
   - Created the IO controller
3. **Clock Emulation**
   - Added cycle counting for each instruction

---

## Upcoming Phases and TODOs

### Phase 7: Enhanced Realism and Instruction Set Completion

#### 1. Comprehensive Instruction Set Implementation
- **Objective**: Complete the implementation of any remaining 8086 instructions with correct operand handling and flag updates.
- **Tasks**:
  - [✅] Verify implementation of: `ADC`, `SBB`, `TEST`, `NOT`, `NEG`, `MUL`, `IMUL`, `DIV`, `IDIV`
  - [✅] Verify shift/rotate operations: `ROL`, `ROR`, `RCL`, `RCR`, `SAL`, `SAR`
  - [ ] Implement control flow operations: `LOOP`, `LOOPE`/`LOOPZ`, `LOOPNE`/`LOOPNZ`, `JCXZ`
  - [ ] Ensure string operations with prefixes work correctly
  - [ ] Verify processor control instructions: `CLC`, `STC`, `CMC`, `CLD`, `STD`, `CLI`, `STI`
  - [ ] Write unit tests for each instruction with different addressing modes and flag effects
  - [ ] Refactor flag handling for correctness, especially `AF` and `OF`

#### 2. Assembler and Disassembler Enhancements
- **Objective**: Expand instruction and directive support in assembler/disassembler.
- **Tasks**:
  - [ ] Add missing CPU instructions to the assembler's instruction table
  - [ ] Implement `DW`, `DD` directives
  - [ ] Implement `ORG` directive and segment directives
  - [ ] Support `BYTE PTR`, `WORD PTR` size overrides
  - [ ] Improve label resolution for jumps/calls
  - [ ] Update disassembler's opcode tables and decoding logic
  - [ ] Enhance ModR/M byte decoding for all addressing modes

### Phase 8: Debugger and I/O Expansion

#### 1. Command-Line Debugger
- **Objective**: Implement basic debugging capabilities within the CLI.
- **Tasks**:
  - [ ] Design CLI commands for debugger actions
  - [ ] Implement breakpoint management
  - [ ] Add single-step execution logic
  - [ ] Add functions to display CPU state (registers, flags, memory)
  - [ ] Implement INT 1 handler for Trap Flag

#### 2. Advanced Interrupt Handling and I/O Emulation
- **Objective**: Implement a proper Interrupt Vector Table (IVT) and expand emulated I/O devices.
- **Tasks**:
  - [ ] Implement IVT lookup in `handleINT`
  - [ ] Enhance INT 10h for more video functions
  - [ ] Implement INT 16h (Keyboard) services
  - [ ] Add INT 21h (DOS) services
  - [ ] Consider basic PIC and PIT emulation

#### 3. (Optional) GUI Front-End
- **Objective**: Create a simple GUI to visualize CPU state and control the debugger.
- **Tasks**:
  - [ ] Select and integrate a GUI library
  - [ ] Design and implement UI views (registers, flags, memory, disassembly)
  - [ ] Link GUI actions to debugger functions

### Phase 9: Refinements and Stretch Goals

#### 1. Advanced Assembler/Disassembler Features
- **Objective**: Add macros and improve data representation.
- **Tasks**:
  - [ ] Implement macro definition and expansion
  - [ ] Support structures and records
  - [ ] Enhance disassembler for better data/code distinction

#### 2. Performance Metrics and Visualization
- **Objective**: Add tracking for cycle counts, emulated CPU speed, etc.
- **Tasks**:
  - [ ] Add real-time performance tracking
  - [ ] Visualize or log these metrics in the CLI
  - [ ] Provide debug printouts of registers and memory

#### 3. Testing & Documentation
- **Objective**: Ensure robustness and comprehensive documentation.
- **Tasks**:
  - [ ] Create thorough test suite covering all instructions and features
  - [ ] Update user documentation
  - [ ] Improve internal code documentation

## Known Issues and Areas for Improvement

### CPU Core (`cpu/`)
- **Instruction Set Coverage**:
  - Segment override prefixes could be better handled in memory addressing
- **Flag Accuracy**:
  - `AF` (Auxiliary Carry Flag) calculation needs consistent implementation
  - `OF` (Overflow Flag) for shifts/rotates needs verification
- **Interrupt Handling**:
  - `handleINT` is basic, only simulating a few interrupt services
  - Hardware interrupts are not emulated
  - `IF` and `TF` flags are defined but not fully utilized

### Assembler (`assembler/`)
- **Instruction Support**:
  - Many addressing modes and operand size variants missing
  - No support for segment override prefixes
- **Directives**: Only `DB` is handled fully
- **Label Resolution**: Needs improvement for relative jumps
- **Macros**: No support yet

### Disassembler (`disassembler/`)
- **Instruction Coverage**:
  - Some opcodes and group opcodes need full decoding
  - Segment override prefixes not displayed
- **Addressing Modes**: Not all 8086 addressing modes covered
- **Operand Formatting**: Could be improved for clarity

### I/O System (`io/`)
- **Port Handling**: Limited to a few basic handlers
- **Peripheral Emulation**: No emulation for standard PC peripherals

### CLI (`main.cpp`)
- The `-e` flag behavior could be more consistently documented
- No interactive debugger interface yet

---

## Project Folder Structure
To keep the project clean and comprehensible, especially for a GPT model or any contributor, you can follow a structure like this:

```
project_root/
  |-- CMakeLists.txt
  |-- main.cpp
  |-- build.sh
  |-- cmake_build.sh
  |-- README.md
  |-- roadmap.md
  |
  |-- cpu/
  |    |-- registers.hpp
  |    |-- flags.hpp
  |    |-- instructions.hpp
  |    |-- instructions.cpp
  |    |-- memory.hpp
  |    |-- memory.cpp
  |    |-- cpu.hpp
  |
  |-- utils/
  |    |-- utils.h
  |    |-- utils.cpp
  |
  |-- assembler/
  |    |-- assembler.hpp
  |    |-- assembler.cpp
  |
  |-- disassembler/
  |    |-- disassembler.hpp
  |    |-- disassembler.cpp
  |
  |-- io/
  |    |-- io.hpp
  |    |-- io.cpp
  |
  |-- examples/
  |    |-- simple.asm
  |    |-- counter.asm
  |    |-- hello.asm
  |    |-- arithmetic.asm
  |    |
  |    |-- output/
  |         |-- simple.bin
  |         |-- counter.bin
  |         |-- hello.bin
  |         |-- arithmetic.bin
  |
  |-- build/
       |-- (build artifacts)
```

> **Note**: The folder names are flexible. The important part is that each module (CPU, assembler, disassembler, etc.) has a dedicated space.

