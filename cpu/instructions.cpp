//
// Created by Hakan Avgın on 21.12.2024.
//
#include "memory.hpp"
#include "registers.hpp"
#include "flags.hpp"
#include "instructions.hpp"
#include <iostream>

namespace CPU {
    Instructions::Instructions(Memory &mem, Registers &reg, Flags &flg)
            : memory(mem), registers(reg), flags(flg) {
        //Initialize opcode table
        opcodeTable[0x88] = std::bind(&Instructions::handleMOV, this);
        opcodeTable[0x89] = std::bind(&Instructions::handleMOV, this);
        opcodeTable[0x8A] = std::bind(&Instructions::handleMOV, this);
        opcodeTable[0x8B] = std::bind(&Instructions::handleMOV, this);

        opcodeTable[0x01] = std::bind(&Instructions::handleADD, this);
        opcodeTable[0x03] = std::bind(&Instructions::handleADD, this);

        opcodeTable[0x29] = std::bind(&Instructions::handleSUB, this);
        opcodeTable[0x2B] = std::bind(&Instructions::handleSUB, this);

        opcodeTable[0xF6] = std::bind(&Instructions::handleMUL, this);
        opcodeTable[0xF7] = std::bind(&Instructions::handleMUL, this);

        opcodeTable[0xF6] = std::bind(&Instructions::handleDIV, this);
        opcodeTable[0xF7] = std::bind(&Instructions::handleDIV, this);

        opcodeTable[0x40] = std::bind(&Instructions::handleINC, this);  // INC for AX
        opcodeTable[0x41] = std::bind(&Instructions::handleINC, this);  // INC for CX
        opcodeTable[0x42] = std::bind(&Instructions::handleINC, this);  // INC for DX
        opcodeTable[0x43] = std::bind(&Instructions::handleINC, this);  // INC for BX
        opcodeTable[0x44] = std::bind(&Instructions::handleINC, this);  // INC for SP
        opcodeTable[0x45] = std::bind(&Instructions::handleINC, this);  // INC for BP
        opcodeTable[0x46] = std::bind(&Instructions::handleINC, this);  // INC for SI
        opcodeTable[0x47] = std::bind(&Instructions::handleINC, this);  // INC for DI

        opcodeTable[0x48] = std::bind(&Instructions::handleDEC, this);  // DEC for AX
        opcodeTable[0x49] = std::bind(&Instructions::handleDEC, this);  // DEC for CX
        opcodeTable[0x4A] = std::bind(&Instructions::handleDEC, this);  // DEC for DX
        opcodeTable[0x4B] = std::bind(&Instructions::handleDEC, this);  // DEC for BX
        opcodeTable[0x4C] = std::bind(&Instructions::handleDEC, this);  // DEC for SP
        opcodeTable[0x4D] = std::bind(&Instructions::handleDEC, this);  // DEC for BP
        opcodeTable[0x4E] = std::bind(&Instructions::handleDEC, this);  // DEC for SI
        opcodeTable[0x4F] = std::bind(&Instructions::handleDEC, this);  // DEC for DI

        opcodeTable[0x38] = std::bind(&Instructions::handleCMP, this);

        opcodeTable[0xE9] = std::bind(&Instructions::handleJMP, this);
        opcodeTable[0x74] = std::bind(&Instructions::handleJE, this);
        opcodeTable[0x75] = std::bind(&Instructions::handleJNE, this);
        opcodeTable[0x77] = std::bind(&Instructions::handleJG, this);
        opcodeTable[0x7D] = std::bind(&Instructions::handleJGE, this);
        opcodeTable[0x7C] = std::bind(&Instructions::handleJL, this);
        opcodeTable[0x7E] = std::bind(&Instructions::handleJLE, this);

        opcodeTable[0xCD] = std::bind(&Instructions::handleINT, this);

        opcodeTable[0xF4] = std::bind(&Instructions::handleHLT, this);

        opcodeTable[0x20] = std::bind(&Instructions::handleAND, this);
        opcodeTable[0x21] = std::bind(&Instructions::handleAND, this);

        opcodeTable[0x08] = std::bind(&Instructions::handleOR, this);
        opcodeTable[0x09] = std::bind(&Instructions::handleOR, this);

        opcodeTable[0x30] = std::bind(&Instructions::handleXOR, this);
        opcodeTable[0x31] = std::bind(&Instructions::handleXOR, this);

        opcodeTable[0xF6] = std::bind(&Instructions::handleNOT, this);
        opcodeTable[0xF7] = std::bind(&Instructions::handleNOT, this);

        opcodeTable[0xD0] = std::bind(&Instructions::handleSHL, this);  // SHL by 1
        opcodeTable[0xD1] = std::bind(&Instructions::handleSHL, this);  // SHL by 1 (for 16-bit registers)
        opcodeTable[0xD2] = std::bind(&Instructions::handleSHR, this);  // SHR by 1
        opcodeTable[0xD3] = std::bind(&Instructions::handleSHR, this);  // SHR by 1 (for 16-bit registers)
    }

    uint8_t Instructions::fetchByte() {
        uint32_t physicalAddress = memory.calculatePhysicalAddress(registers.CS, registers.IP);
        uint8_t byte = memory.readByte(physicalAddress);
        registers.IP++;
        return byte;
    }
    uint16_t Instructions::fetchWord() {
        uint32_t physicalAddress = memory.calculatePhysicalAddress(registers.CS, registers.IP);
        uint16_t word = memory.readWord(physicalAddress);
        registers.IP += 2;
        return word;
    }
    void Instructions::decodeAndExecute(uint8_t opcode) {
        if(opcodeTable.find(opcode) != opcodeTable.end()) {
            opcodeTable[opcode](); //execute the instruction
        } else {
            throw std::runtime_error("Unknown opcode: " + std::to_string(opcode));
        }
    }
    void Instructions::executeNext() {
        uint8_t opcode = fetchByte();
        decodeAndExecute(opcode);
    }
    void Instructions::handleADD() {
        uint16_t dest = registers.AX.value;
        uint16_t src = fetchWord();
        uint32_t result = dest + src;

        registers.AX.value = result;
        flags.setFlag(FLAGS::ZF, result == 0);
        flags.setFlag(FLAGS::CF, result < dest);
    }

    void Instructions::handleSUB() {
        uint16_t dest = registers.AX.value;
        uint16_t src = fetchWord();
        uint16_t result = dest - src;

        registers.AX.value = result;
        flags.setFlag()
    }







}