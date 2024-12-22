cmake_minimum_required(VERSION 3.28)
project(emu8086)

set(CMAKE_CXX_STANDARD 20)

add_executable(emu8086 main.cpp
        cpu/cpu.hpp
        cpu/registers.hpp
        cpu/flags.hpp
        cpu/instructions.hpp
        cpu/memory.hpp
        cpu/memory.cpp
        cpu/instructions.cpp)
