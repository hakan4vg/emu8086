//
// Created by Hakan Avgın on 21.12.2024.
//
#include "memory.hpp"
#include "registers.hpp"
#include "flags.hpp"
#include "instructions.hpp"
#include <iostream>
#include "../utils/utils.h"


namespace CPU {
    cpu::UTILS utils;
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

        opcodeTable[0xF6] = std::bind(&Instructions::handleF6, this); //MUL
        opcodeTable[0xF7] = std::bind(&Instructions::handleF6, this); //MUL

        opcodeTable[0xF6] = std::bind(&Instructions::handleF6, this);  //DIV
        opcodeTable[0xF7] = std::bind(&Instructions::handleF6, this);  //DIV

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

        opcodeTable[0xF6] = std::bind(&Instructions::handleF6, this); //NOT
        opcodeTable[0xF7] = std::bind(&Instructions::handleF6, this); //NOT

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

     /**
     * Handles arithmetic ADD instruction
     * Performs dest = dest + src and sets appropriate flags:
     * - ZF: Set if result is zero
     * - CF: Set if unsigned overflow occurred
     * - OF: Set if signed overflow occurred
     * - SF: Set if result is negative
     * - AF: Set if auxiliary carry occurred
     */
    void Instructions::setArithmeticFlags(uint16_t result, uint16_t dest, uint16_t src) {
        flags.setFlag(FLAGS::ZF, result == 0);
        flags.setFlag(FLAGS::CF, result < dest);
        flags.setFlag(FLAGS::OF, ((dest ^ src ^ 0x8000) & (dest ^ result) & 0x8000) != 0);
        flags.setFlag(FLAGS::AF, ((dest ^ src ^ result) & 0x10) != 0);
        flags.setFlag(FLAGS::SF, (result & 0x8000) != 0);
    }
    uint16_t* Instructions::getMemoryReference(uint8_t mod, uint8_t rm) {
        uint16_t base = 0;
        uint16_t offset = 0;

        switch (rm) {
            case 0b000: base = registers.BX.value + registers.SI; break;
            case 0b001: base = registers.BX.value + registers.DI; break;
            case 0b010: base = registers.BP + registers.SI; break;
            case 0b011: base = registers.BP + registers.DI; break;
            case 0b100: base = registers.SI; break;
            case 0b101: base = registers.DI; break;
            case 0b110:
                if (mod == 0b00) {
                    offset = fetchWord(); // Direct address mode
                } else {
                    base = registers.BP;
                }
                break;
            case 0b111: base = registers.BX.value; break;
        }

        if (mod == 0b01) offset += static_cast<int8_t>(fetchByte()); // 8-bit displacement
        if (mod == 0b10) offset += fetchWord(); // 16-bit displacement

        uint32_t physicalAddress = memory.calculatePhysicalAddress(base, offset);
        return memory.getPointer(physicalAddress);
}
    
    void Instructions::executeNext() {
        uint8_t opcode = fetchByte();
        decodeAndExecute(opcode);
    }
    void Instructions::handleF6()
    {
        uint8_t modrm = fetchByte();
        uint8_t subOpcode = (modrm >> 3) & 0x07;

        switch (subOpcode)
        {
        case 0x04: handleMUL(); break;
        case 0x06: handleDIV(); break;
        case 0x02: handleNOT(); break;
        default: throw std::runtime_error("Unknown F6 opcode: " + std::to_string(subOpcode));
        }
    }

    uint16_t* Instructions::getRegisterReference(uint8_t reg) {
        switch (reg) {
            case 0: return &registers.AX.value;
            case 1: return &registers.CX.value;
            case 2: return &registers.DX.value;
            case 3: return &registers.BX.value;
            case 4: return &registers.SP;
            case 5: return &registers.BP;
            case 6: return &registers.SI;
            case 7: return &registers.DI;
            default: throw std::runtime_error("Invalid register code");
        }
    }

    void Instructions::handleADD() {
        uint8_t modrm = fetchByte();
        uint8_t mod = (modrm >> 6) & 0x03;
        uint8_t reg = (modrm >> 3) & 0x07;
        uint8_t rm = modrm & 0x07;

        uint16_t* dest;
        uint16_t src;

        if (mod == 0b11) { 
            // Register to Register
            dest = getRegisterReference(rm);
            src = *getRegisterReference(reg);
        } else { 
            // Memory involved
            dest = getMemoryReference(mod, rm);
            src = *getRegisterReference(reg);
        }

        uint32_t result = *dest + src;
        setArithmeticFlags(result, *dest, src);
        *dest = result;
    }
    void Instructions::handleSUB() {
        uint8_t modrm = fetchByte();
        uint8_t mod = (modrm >> 6) & 0x03;
        uint8_t reg = (modrm >> 3) & 0x07;
        uint8_t rm = modrm & 0x07;

        uint16_t* dest;
        uint16_t src;

        if (mod == 0b11) { 
            // Register to Register
            dest = getRegisterReference(rm);
            src = *getRegisterReference(reg);
        } else { 
            // Memory involved
            dest = getMemoryReference(mod, rm);
            src = *getRegisterReference(reg);
        }

        uint32_t result = *dest - src;
        setArithmeticFlags(result, *dest, src);
        *dest = result;
    }


    void Instructions::handleMUL() {
        uint8_t modrm = fetchByte();
        uint8_t mod = (modrm >> 6) & 0x03;
        uint8_t reg = (modrm >> 3) & 0x07;
        uint8_t rm = modrm & 0x07;

        uint16_t* dest = getRegisterReference(rm);
        uint16_t src = fetchWord();

        uint32_t result = *dest * src;

        setArithmeticFlags(result, *dest ,src);

        // flags.setFlag(FLAGS::ZF, result == 0);
        // flags.setFlag(FLAGS::CF, result > 0xFFFF);
        // flags.setFlag(FLAGS::OF, result > 0xFFFF);

        *dest = result;
    }

    void Instructions::handleDIV(){
        uint8_t modrm = fetchByte();
        uint8_t mod = (modrm >> 6) & 0x03;
        uint8_t reg = (modrm >> 3) & 0x07;
        uint8_t rm = modrm & 0x07;

        uint16_t* dest = getRegisterReference(rm);
        uint16_t src = fetchWord();

        if (src == 0) {
            throw std::runtime_error("Division by zero");
        }

        uint32_t result = *dest / src;

        setArithmeticFlags(result, *dest ,src);

        // flags.setFlag(FLAGS::ZF, result == 0);
        // flags.setFlag(FLAGS::CF, result > 0xFFFF);
        // flags.setFlag(FLAGS::OF, result > 0xFFFF);

        *dest = result;
    }


    void Instructions::handleINC() {
        uint8_t reg = fetchByte() & 0x07;
        uint16_t* dest = getRegisterReference(reg);
        uint16_t result = *dest + 1;

        flags.setFlag(FLAGS::ZF, result == 0);
        flags.setFlag(FLAGS::SF, (result & 0x8000) != 0);
        flags.setFlag(FLAGS::OF, result == 0x8000);
        flags.setFlag(FLAGS::AF, ((*dest ^ result) & 0x10) != 0);

        *dest = result;
    }

    void Instructions::handleDEC() {
        uint8_t reg = fetchByte() & 0x07;
        uint16_t* dest = getRegisterReference(reg);
        uint16_t result = *dest - 1;

        flags.setFlag(FLAGS::ZF, result == 0);
        flags.setFlag(FLAGS::SF, (result & 0x8000) != 0);
        flags.setFlag(FLAGS::OF, result == 0x7FFF);
        flags.setFlag(FLAGS::AF, ((*dest ^ result) & 0x10) != 0);

        *dest = result;
    }


    void Instructions::handleCMP() {
        uint8_t modrm = fetchByte();
        uint8_t mod = (modrm >> 6) & 0x03;
        uint8_t reg = (modrm >> 3) & 0x07;
        uint8_t rm = modrm & 0x07;

        uint16_t* dest = getRegisterReference(rm);
        uint16_t src = fetchWord();

        uint16_t result = *dest - src;

        setArithmeticFlags(result, *dest ,src);

        // flags.setFlag(FLAGS::ZF, result == 0);
        // flags.setFlag(FLAGS::CF, *dest < src);
        // flags.setFlag(FLAGS::OF, ((*dest ^ src) & (*dest ^ result) & 0x8000) != 0);
        // flags.setFlag(FLAGS::AF, ((*dest ^ src ^ result) & 0x10) != 0);
        // flags.setFlag(FLAGS::SF, (result & 0x8000) != 0);
    }

    void Instructions::handleINT() //Interrupt
    {
        //TODO Implement interrupt handling
    }
    void Instructions::handleHLT() //Halt
    {
        //TODO Implement halt
    }

    void Instructions::handleAND() {
        uint8_t modrm = fetchByte();
        uint8_t mod = (modrm >> 6) & 0x03;
        uint8_t reg = (modrm >> 3) & 0x07;
        uint8_t rm = modrm & 0x07;

        uint16_t* dest = getRegisterReference(rm);
        uint16_t src = fetchWord();

        *dest &= src;

       // setArithmeticFlags(result, *dest ,src);

        flags.setFlag(FLAGS::ZF, *dest == 0);
        flags.setFlag(FLAGS::SF, (*dest & 0x8000) != 0);
        flags.setFlag(FLAGS::PF, utils.calculateParity(*dest));
        flags.setFlag(FLAGS::CF, false); // AND does not affect carry flag
        flags.setFlag(FLAGS::OF, false); // AND does not affect overflow flag
    }

    void Instructions::handleOR() {
        uint8_t modrm = fetchByte();
        uint8_t mod = (modrm >> 6) & 0x03;
        uint8_t reg = (modrm >> 3) & 0x07;
        uint8_t rm = modrm & 0x07;

        uint16_t* dest = getRegisterReference(rm);
        uint16_t src = fetchWord();

        *dest |= src;


        flags.setFlag(FLAGS::ZF, *dest == 0);
        flags.setFlag(FLAGS::SF, (*dest & 0x8000) != 0);
        flags.setFlag(FLAGS::PF, utils.calculateParity(*dest));
        flags.setFlag(FLAGS::CF, false); // OR does not affect carry flag
        flags.setFlag(FLAGS::OF, false); // OR does not affect overflow flag
    }

    void Instructions::handleXOR() {
        uint8_t modrm = fetchByte();
        uint8_t mod = (modrm >> 6) & 0x03;
        uint8_t reg = (modrm >> 3) & 0x07;
        uint8_t rm = modrm & 0x07;

        uint16_t* dest = getRegisterReference(rm);
        uint16_t src = fetchWord();

        *dest ^= src;


        flags.setFlag(FLAGS::ZF, *dest == 0);
        flags.setFlag(FLAGS::SF, (*dest & 0x8000) != 0);
        flags.setFlag(FLAGS::PF, utils.calculateParity(*dest));
        flags.setFlag(FLAGS::CF, false); // XOR does not affect carry flag
        flags.setFlag(FLAGS::OF, false); // XOR does not affect overflow flag
    }

    void Instructions::handleNOT() {
        uint8_t reg = fetchByte() & 0x07;
        uint16_t* dest = getRegisterReference(reg);

        *dest = ~(*dest);

        flags.setFlag(FLAGS::ZF, *dest == 0);
        flags.setFlag(FLAGS::SF, (*dest & 0x8000) != 0);
        flags.setFlag(FLAGS::PF, utils.calculateParity(*dest));
        flags.setFlag(FLAGS::CF, false); // NOT does not affect carry flag
        flags.setFlag(FLAGS::OF, false); // NOT does not affect overflow flag
    }
    
    void Instructions::handleSHL() {
        uint8_t modrm = fetchByte();
        uint8_t reg = (modrm >> 3) & 0x07;
        uint8_t rm = modrm & 0x07;

        uint16_t* dest = getRegisterReference(rm);
        uint8_t count = fetchByte() & 0x1F; // Only lower 5 bits are used for the count

        uint16_t result = *dest << count;

        flags.setFlag(FLAGS::ZF, result == 0);
        flags.setFlag(FLAGS::SF, (result & 0x8000) != 0);
        flags.setFlag(FLAGS::PF, utils.calculateParity(*dest));
        flags.setFlag(FLAGS::CF, (*dest & (1 << (16 - count))) != 0);
        flags.setFlag(FLAGS::OF, ((*dest ^ result) & 0x8000) != 0);

        *dest = result;
    }

    void Instructions::handleSHR() {
        uint8_t modrm = fetchByte();
        uint8_t reg = (modrm >> 3) & 0x07;
        uint8_t rm = modrm & 0x07;

        uint16_t* dest = getRegisterReference(rm);
        uint8_t count = fetchByte() & 0x1F; // Only lower 5 bits are used for the count

        uint16_t result = *dest >> count;

        flags.setFlag(FLAGS::ZF, result == 0);
        flags.setFlag(FLAGS::SF, (result & 0x8000) != 0);
        flags.setFlag(FLAGS::PF, utils.calculateParity(*dest));
        flags.setFlag(FLAGS::CF, (*dest & (1 << (count - 1))) != 0);
        flags.setFlag(FLAGS::OF, ((*dest ^ result) & 0x8000) != 0);

        *dest = result;
    }
    

    void Instructions::handleJE() {
        int16_t offset = static_cast<int16_t>(fetchWord());
        if (flags.getFlag(FLAGS::ZF)) {
            registers.IP += offset;
        }
    }

    void Instructions::handleJNE() {
        int16_t offset = static_cast<int16_t>(fetchWord());
        if (!flags.getFlag(FLAGS::ZF)) {
            registers.IP += offset;
        }
    }

    void Instructions::handleJG() {
        int16_t offset = static_cast<int16_t>(fetchWord());
        if (!flags.getFlag(FLAGS::ZF) && (flags.getFlag(FLAGS::SF) == flags.getFlag(FLAGS::OF))) {
            registers.IP += offset;
        }
    }

    void Instructions::handleJGE() {
        int16_t offset = static_cast<int16_t>(fetchWord());
        if (flags.getFlag(FLAGS::SF) == flags.getFlag(FLAGS::OF)) {
            registers.IP += offset;
        }
    }

    void Instructions::handleJL() {
        int16_t offset = static_cast<int16_t>(fetchWord());
        if (flags.getFlag(FLAGS::SF) != flags.getFlag(FLAGS::OF)) {
            registers.IP += offset;
        }
    }

    void Instructions::handleJLE() {
        int16_t offset = static_cast<int16_t>(fetchWord());
        if (flags.getFlag(FLAGS::ZF) || (flags.getFlag(FLAGS::SF) != flags.getFlag(FLAGS::OF))) {
            registers.IP += offset;
        }
    }

    void Instructions::handleJMP() {
        int16_t offset = static_cast<int16_t>(fetchWord());
        registers.IP += offset;
    }

    void Instructions::handleMOV() {
        uint8_t modrm = fetchByte();
        uint8_t mod = (modrm >> 6) & 0x03;
        uint8_t reg = (modrm >> 3) & 0x07;
        uint8_t rm = modrm & 0x07;

        uint16_t* dest;
        uint16_t src;

        if (mod == 0b11) { 
            // Register to Register
            dest = getRegisterReference(rm);
            src = *getRegisterReference(reg);
        } else { 
            // Memory involved
            dest = getMemoryReference(mod, rm);
            src = *getRegisterReference(reg);
        }

        *dest = src;
    }

    void Instructions::handlePUSH() {
        uint8_t reg = fetchByte() & 0x07;
        uint16_t* src = getRegisterReference(reg);

        registers.SP -= 2;
        memory.writeWord(memory.calculatePhysicalAddress(registers.SS, registers.SP), *src);
    }
    void Instructions::handlePOP() {
        uint8_t reg = fetchByte() & 0x07;
        uint16_t* dest = getRegisterReference(reg);

        *dest = memory.readWord(memory.calculatePhysicalAddress(registers.SS, registers.SP));
        registers.SP += 2;
    }
    void Instructions::handleCALL() {
        int16_t offset = static_cast<int16_t>(fetchWord());
        registers.SP -= 2;
        memory.writeWord(memory.calculatePhysicalAddress(registers.SS, registers.SP), registers.IP);
        registers.IP += offset;
    }
    void Instructions::handleRET() {
        registers.IP = memory.readWord(memory.calculatePhysicalAddress(registers.SS, registers.SP));
        registers.SP += 2;
    }

}