#ifndef REGISTERS_HPP
#define REGISTERS_HPP

#include <cstdint> // For fixed-width integer types
#include "flags.hpp" // Include FLAGS structure

namespace CPU {

    // 16-bit general-purpose register with access to high and low bytes
    union GeneralRegister {
        uint16_t value; // Full 16-bit register
        struct {
            uint8_t low;  // Low 8 bits
            uint8_t high; // High 8 bits
        };
    };

    struct Registers {
        // General-purpose registers
        GeneralRegister AX, BX, CX, DX;

        // Index and pointer registers
        uint16_t SI, DI, SP, BP;

        // Segment registers
        uint16_t CS, DS, SS, ES;

        // Instruction pointer
        uint16_t IP;

        // Flags register
        Flags FLAGS;

        // Constructor to initialize all registers to 0
        Registers() : AX{}, BX{}, CX{}, DX{}, SI(0), DI(0), SP(0), BP(0),
                      CS(0), DS(0), SS(0), ES(0), IP(0), FLAGS() {}
    };

} // namespace CPU

#endif // REGISTERS_HPP
