cmake_minimum_required(VERSION 3.28)
project(emu8086)

set(CMAKE_CXX_STANDARD 20)

# Create examples/output directory
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/examples/output)

add_executable(emu8086 main.cpp
        cpu/cpu.hpp
        cpu/registers.hpp
        cpu/flags.hpp
        cpu/instructions.hpp
        cpu/memory.hpp
        cpu/memory.cpp
        cpu/instructions.cpp
        utils/utils.cpp
        utils/utils.h
        io/io.hpp
        io/io.cpp
        assembler/assembler.hpp
        assembler/assembler.cpp
        disassembler/disassembler.hpp
        disassembler/disassembler.cpp)

# Copy the executable to the project root directory for convenience
add_custom_command(TARGET emu8086 POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:emu8086> ${CMAKE_CURRENT_SOURCE_DIR}/emu8086
        COMMENT "Copying executable to project root")
