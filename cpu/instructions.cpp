#include "instructions.hpp"
#include <stdexcept>
#include <iostream>
#include "../utils/utils.h"

namespace CPU {

    static cpu::UTILS utils; 

    //--------------------------------------------------------------------------
    // Constructor: populate opcodeTable
    //--------------------------------------------------------------------------
    Instructions::Instructions(Memory &mem, Registers &reg, Flags &flg)
        : memory(mem), registers(reg), flags(flg), halted(false)
    {
        // MOV instructions (a subset of them: 88, 89, 8A, 8B)
        opcodeTable[0x88] = std::bind(&Instructions::handleMOV, this);
        opcodeTable[0x89] = std::bind(&Instructions::handleMOV, this);
        opcodeTable[0x8A] = std::bind(&Instructions::handleMOV, this);
        opcodeTable[0x8B] = std::bind(&Instructions::handleMOV, this);

        // ADD (some variants: 0x01, 0x03, etc.)
        opcodeTable[0x01] = std::bind(&Instructions::handleADD, this);
        opcodeTable[0x03] = std::bind(&Instructions::handleADD, this);

        // SUB
        opcodeTable[0x29] = std::bind(&Instructions::handleSUB, this);
        opcodeTable[0x2B] = std::bind(&Instructions::handleSUB, this);

        // INC and DEC (0x40-0x4F in many forms)
        for(uint8_t op = 0x40; op <= 0x47; ++op) {
            opcodeTable[op] = std::bind(&Instructions::handleINC, this);
        }
        for(uint8_t op = 0x48; op <= 0x4F; ++op) {
            opcodeTable[op] = std::bind(&Instructions::handleDEC, this);
        }

        // CMP
        opcodeTable[0x38] = std::bind(&Instructions::handleCMP, this);

        // Jumps
        opcodeTable[0xE9] = std::bind(&Instructions::handleJMP, this);
        opcodeTable[0x74] = std::bind(&Instructions::handleJE, this);
        opcodeTable[0x75] = std::bind(&Instructions::handleJNE, this);
        opcodeTable[0x77] = std::bind(&Instructions::handleJG, this);
        opcodeTable[0x7D] = std::bind(&Instructions::handleJGE, this);
        opcodeTable[0x7C] = std::bind(&Instructions::handleJL, this);
        opcodeTable[0x7E] = std::bind(&Instructions::handleJLE, this);

        // INT and HLT
        opcodeTable[0xCD] = std::bind(&Instructions::handleINT, this);
        opcodeTable[0xF4] = std::bind(&Instructions::handleHLT, this);

        // Logical ops: AND, OR, XOR
        opcodeTable[0x20] = std::bind(&Instructions::handleAND, this);
        opcodeTable[0x21] = std::bind(&Instructions::handleAND, this);
        opcodeTable[0x08] = std::bind(&Instructions::handleOR,  this);
        opcodeTable[0x09] = std::bind(&Instructions::handleOR,  this);
        opcodeTable[0x30] = std::bind(&Instructions::handleXOR, this);
        opcodeTable[0x31] = std::bind(&Instructions::handleXOR, this);

        // SHIFT/ROTATE (D0, D1, D2, D3 for certain ops)
        opcodeTable[0xD0] = std::bind(&Instructions::handleSHL, this); // or handle single-bit shift
        opcodeTable[0xD1] = std::bind(&Instructions::handleSHL, this);
        opcodeTable[0xD2] = std::bind(&Instructions::handleSHR, this);
        opcodeTable[0xD3] = std::bind(&Instructions::handleSHR, this);

        // PUSH/POP (examples: 0x50-0x5F for push/pop reg)
        // CALL/RET (0xE8, 0xC3, etc.)
        // For brevity, we’ll just map a couple:
        opcodeTable[0x50] = std::bind(&Instructions::handlePUSH, this); // PUSH AX
        opcodeTable[0x51] = std::bind(&Instructions::handlePUSH, this); // PUSH CX
        opcodeTable[0x58] = std::bind(&Instructions::handlePOP,  this); // POP AX
        opcodeTable[0x59] = std::bind(&Instructions::handlePOP,  this); // POP CX
        opcodeTable[0xE8] = std::bind(&Instructions::handleCALL, this);
        opcodeTable[0xC3] = std::bind(&Instructions::handleRET,  this);

        // 0xF6 (8-bit) / 0xF7 (16-bit): TEST, NOT, NEG, MUL, IMUL, DIV, IDIV
        opcodeTable[0xF6] = std::bind(&Instructions::handleF6, this);
        opcodeTable[0xF7] = std::bind(&Instructions::handleF7, this);
    }

    //--------------------------------------------------------------------------
    // Fetch + Decode
    //--------------------------------------------------------------------------
    uint8_t Instructions::fetchByte() {
        uint32_t phys = memory.calculatePhysicalAddress(registers.CS, registers.IP);
        uint8_t val   = memory.readByte(phys);
        registers.IP++;
        return val;
    }

    uint16_t Instructions::fetchWord() {
        uint32_t phys = memory.calculatePhysicalAddress(registers.CS, registers.IP);
        uint16_t val  = memory.readWord(phys);
        registers.IP += 2;
        return val;
    }

    void Instructions::decodeAndExecute(uint8_t opcode) {
        auto it = opcodeTable.find(opcode);
        if(it != opcodeTable.end()) {
            it->second();
        } else {
            throw std::runtime_error("Unknown opcode: " + std::to_string(opcode));
        }
    }

    void Instructions::executeNext() {
        if(halted) {
            return;
        }
        uint8_t opcode = fetchByte();
        decodeAndExecute(opcode);
    }

    //--------------------------------------------------------------------------
    // Helpers: getRegisterReference, getMemoryReference, setArithmeticFlags
    //--------------------------------------------------------------------------
    uint16_t* Instructions::getRegisterReference(uint8_t reg) {
        // 8086 uses AX=0, CX=1, DX=2, BX=3, SP=4, BP=5, SI=6, DI=7
        switch(reg) {
            case 0: return &registers.AX.value;
            case 1: return &registers.CX.value;
            case 2: return &registers.DX.value;
            case 3: return &registers.BX.value;
            case 4: return &registers.SP;
            case 5: return &registers.BP;
            case 6: return &registers.SI;
            case 7: return &registers.DI;
            default:
                throw std::runtime_error("Invalid register code: " + std::to_string(reg));
        }
    }

    uint16_t* Instructions::getMemoryReference(uint8_t mod, uint8_t rm) {
        /*
         * 16-bit addressing modes (8086):
         *   rm=000 => BX + SI
         *   rm=001 => BX + DI
         *   rm=010 => BP + SI
         *   rm=011 => BP + DI
         *   rm=100 => SI
         *   rm=101 => DI
         *   rm=110 => (if mod==00 => disp16) else BP + disp8/16
         *   rm=111 => BX
         *
         * 'mod' indicates how we interpret displacement:
         *   00 => no disp, except if rm=110 => disp16
         *   01 => disp8
         *   10 => disp16
         */
        uint16_t base = 0;
        uint16_t disp = 0;

        switch(rm) {
            case 0b000: base = registers.BX.value + registers.SI; break;
            case 0b001: base = registers.BX.value + registers.DI; break;
            case 0b010: base = registers.BP + registers.SI;        break;
            case 0b011: base = registers.BP + registers.DI;        break;
            case 0b100: base = registers.SI;                       break;
            case 0b101: base = registers.DI;                       break;
            case 0b110:
                if(mod == 0b00) {
                    // Direct address
                    disp = fetchWord();
                } else {
                    base = registers.BP;
                }
                break;
            case 0b111: base = registers.BX.value; break;
        }

        if(mod == 0b01) {
            disp += static_cast<int8_t>(fetchByte());  // 8-bit displacement
        } else if(mod == 0b10) {
            disp += fetchWord();  // 16-bit displacement
        }

        uint32_t phys = memory.calculatePhysicalAddress(base, disp);
        // getPointer returns a pointer to memory at address, interpreted as 16-bit
        return memory.getPointer(static_cast<uint16_t>(phys));
    }

    void Instructions::setArithmeticFlags(uint32_t result, uint16_t dest, uint16_t src) {
        // Typical 16-bit arithmetic flags:
        uint16_t res16 = static_cast<uint16_t>(result);

        // Zero
        flags.setFlag(FLAGS::ZF, (res16 == 0));

        // Carry if result didn't fit in 16 bits
        flags.setFlag(FLAGS::CF, (result > 0xFFFF));

        // Sign
        flags.setFlag(FLAGS::SF, (res16 & 0x8000) != 0);

        // Overflow (for signed)
        // 16-bit signed overflow check: 
        // (dest ^ src ^ 0x8000) & (dest ^ res16) & 0x8000
        bool of = ((dest ^ src ^ 0x8000) & (dest ^ res16) & 0x8000) != 0;
        flags.setFlag(FLAGS::OF, of);

        // Auxiliary carry (bit 3)
        bool af = ((dest ^ src ^ res16) & 0x10) != 0;
        flags.setFlag(FLAGS::AF, af);

        // Parity
        flags.setFlag(FLAGS::PF, utils.calculateParity(res16));
    }

    //--------------------------------------------------------------------------
    // Data Movement: MOV
    //--------------------------------------------------------------------------
    void Instructions::handleMOV() {
        /*
         * Example for opcodes 88, 89, 8A, 8B, etc.
         * We parse the ModR/M byte and see if it's register->mem or mem->register.
         */
        uint8_t modrm = fetchByte();
        uint8_t mod   = (modrm >> 6) & 0x03;
        uint8_t reg   = (modrm >> 3) & 0x07;
        uint8_t rm    = (modrm & 0x07);

        uint16_t* dest;
        uint16_t  srcVal;

        bool direction = false;
        // For opcodes: 
        //   0x88 => MOV r/m8,  r8
        //   0x8A => MOV r8,    r/m8
        //   0x89 => MOV r/m16, r16
        //   0x8B => MOV r16,   r/m16
        uint8_t lastOpcode = 0; 
        lastOpcode = memory.readByte(memory.calculatePhysicalAddress(registers.CS, registers.IP - 1));

        if(lastOpcode == 0x8B || lastOpcode == 0x8A) {
            // MOV reg, r/m
            direction = true;
        } else {
            // MOV r/m, reg
            direction = false;
        }

        if(direction) {
            // dest = register, src = memory/reg
            uint16_t* regPtr = getRegisterReference(reg);
            if(mod == 0b11) {
                // reg, reg
                *regPtr = *getRegisterReference(rm);
            } else {
                // reg, memory
                *regPtr = *getMemoryReference(mod, rm);
            }
        } else {
            // dest = memory/reg, src = register
            uint16_t* regPtr = getRegisterReference(reg);
            if(mod == 0b11) {
                *getRegisterReference(rm) = *regPtr;
            } else {
                *getMemoryReference(mod, rm) = *regPtr;
            }
        }
    }

    //--------------------------------------------------------------------------
    // Arithmetic: ADD, SUB, CMP, INC, DEC
    //--------------------------------------------------------------------------
    void Instructions::handleADD() {
        uint8_t modrm = fetchByte();
        uint8_t mod   = (modrm >> 6) & 0x03;
        uint8_t reg   = (modrm >> 3) & 0x07;
        uint8_t rm    = (modrm & 0x07);

        uint16_t* dest;
        uint16_t  srcVal;

        if(mod == 0b11) {
            dest   = getRegisterReference(rm);
            srcVal = *getRegisterReference(reg);
        } else {
            dest   = getMemoryReference(mod, rm);
            srcVal = *getRegisterReference(reg);
        }

        uint32_t result = static_cast<uint32_t>(*dest) + static_cast<uint32_t>(srcVal);
        setArithmeticFlags(result, *dest, srcVal);
        *dest = static_cast<uint16_t>(result);
    }

    void Instructions::handleSUB() {
        uint8_t modrm = fetchByte();
        uint8_t mod   = (modrm >> 6) & 0x03;
        uint8_t reg   = (modrm >> 3) & 0x07;
        uint8_t rm    = (modrm & 0x07);

        uint16_t* dest;
        uint16_t  srcVal;

        if(mod == 0b11) {
            dest   = getRegisterReference(rm);
            srcVal = *getRegisterReference(reg);
        } else {
            dest   = getMemoryReference(mod, rm);
            srcVal = *getRegisterReference(reg);
        }

        uint32_t result = static_cast<uint32_t>(*dest) - static_cast<uint32_t>(srcVal);
        setArithmeticFlags(result, *dest, srcVal);
        *dest = static_cast<uint16_t>(result);
    }

    void Instructions::handleCMP() {
        uint8_t modrm = fetchByte();
        uint8_t mod   = (modrm >> 6) & 0x03;
        uint8_t reg   = (modrm >> 3) & 0x07;
        uint8_t rm    = (modrm & 0x07);

        uint16_t* dest;
        uint16_t  srcVal;

        if(mod == 0b11) {
            dest   = getRegisterReference(rm);
            srcVal = *getRegisterReference(reg);
        } else {
            dest   = getMemoryReference(mod, rm);
            srcVal = *getRegisterReference(reg);
        }

        uint32_t result = static_cast<uint32_t>(*dest) - static_cast<uint32_t>(srcVal);
        setArithmeticFlags(result, *dest, srcVal);
    }

    void Instructions::handleINC() {
        // 0x40-0x47 => inc register
        uint8_t lastOpcode = memory.readByte(memory.calculatePhysicalAddress(registers.CS, registers.IP - 1));
        uint8_t regCode = lastOpcode & 0x07;  // e.g. 0x40 => 0, 0x41 => 1, etc.

        uint16_t* dest = getRegisterReference(regCode);

        uint32_t result = static_cast<uint32_t>(*dest) + 1;
        setArithmeticFlags(result, *dest, 1);
        *dest = static_cast<uint16_t>(result);
    }

    void Instructions::handleDEC() {
        // 0x48-0x4F => dec register
        uint8_t lastOpcode = memory.readByte(memory.calculatePhysicalAddress(registers.CS, registers.IP - 1));
        uint8_t regCode = lastOpcode & 0x07;

        uint16_t* dest = getRegisterReference(regCode);

        uint32_t result = static_cast<uint32_t>(*dest) - 1;
        setArithmeticFlags(result, *dest, 1);
        *dest = static_cast<uint16_t>(result);
    }

    //--------------------------------------------------------------------------
    // Logic: AND, OR, XOR, NOT
    //--------------------------------------------------------------------------
    void Instructions::handleAND() {
        uint8_t modrm = fetchByte();
        uint8_t mod   = (modrm >> 6) & 0x03;
        uint8_t reg   = (modrm >> 3) & 0x07;
        uint8_t rm    = (modrm & 0x07);

        uint16_t* dest;
        uint16_t  srcVal;

        if(mod == 0b11) {
            dest   = getRegisterReference(rm);
            srcVal = *getRegisterReference(reg);
        } else {
            dest   = getMemoryReference(mod, rm);
            srcVal = *getRegisterReference(reg);
        }

        uint16_t result = (*dest) & srcVal;
        *dest = result;

        // Set flags
        flags.setFlag(FLAGS::ZF, (result == 0));
        flags.setFlag(FLAGS::SF, (result & 0x8000) != 0);
        flags.setFlag(FLAGS::OF, false);
        flags.setFlag(FLAGS::CF, false);
        flags.setFlag(FLAGS::AF, false);
        flags.setFlag(FLAGS::PF, utils.calculateParity(result));
    }

    void Instructions::handleOR() {
        uint8_t modrm = fetchByte();
        uint8_t mod   = (modrm >> 6) & 0x03;
        uint8_t reg   = (modrm >> 3) & 0x07;
        uint8_t rm    = (modrm & 0x07);

        uint16_t* dest;
        uint16_t  srcVal;

        if(mod == 0b11) {
            dest   = getRegisterReference(rm);
            srcVal = *getRegisterReference(reg);
        } else {
            dest   = getMemoryReference(mod, rm);
            srcVal = *getRegisterReference(reg);
        }

        uint16_t result = (*dest) | srcVal;
        *dest = result;

        flags.setFlag(FLAGS::ZF, (result == 0));
        flags.setFlag(FLAGS::SF, (result & 0x8000) != 0);
        flags.setFlag(FLAGS::OF, false);
        flags.setFlag(FLAGS::CF, false);
        flags.setFlag(FLAGS::AF, false);
        flags.setFlag(FLAGS::PF, utils.calculateParity(result));
    }

    void Instructions::handleXOR() {
        uint8_t modrm = fetchByte();
        uint8_t mod   = (modrm >> 6) & 0x03;
        uint8_t reg   = (modrm >> 3) & 0x07;
        uint8_t rm    = (modrm & 0x07);

        uint16_t* dest;
        uint16_t  srcVal;

        if(mod == 0b11) {
            dest   = getRegisterReference(rm);
            srcVal = *getRegisterReference(reg);
        } else {
            dest   = getMemoryReference(mod, rm);
            srcVal = *getRegisterReference(reg);
        }

        uint16_t result = (*dest) ^ srcVal;
        *dest = result;

        flags.setFlag(FLAGS::ZF, (result == 0));
        flags.setFlag(FLAGS::SF, (result & 0x8000) != 0);
        flags.setFlag(FLAGS::OF, false);
        flags.setFlag(FLAGS::CF, false);
        flags.setFlag(FLAGS::AF, false);
        flags.setFlag(FLAGS::PF, utils.calculateParity(result));
    }

    void Instructions::handleNOT() {
        uint8_t modrm = fetchByte();
        uint8_t mod   = (modrm >> 6) & 0x03;
        uint8_t rm    = (modrm & 0x07);

        uint16_t* dest;
        if(mod == 0b11) {
            dest = getRegisterReference(rm);
        } else {
            dest = getMemoryReference(mod, rm);
        }

        *dest = ~(*dest);

        flags.setFlag(FLAGS::ZF, (*dest == 0));
        flags.setFlag(FLAGS::SF, (*dest & 0x8000) != 0);
        flags.setFlag(FLAGS::PF, utils.calculateParity(*dest));
    }

    //--------------------------------------------------------------------------
    // SHL / SHR
    //--------------------------------------------------------------------------
    void Instructions::handleSHL() {
        // For 0xD0, 0xD1, 0xD2, 0xD3, check how many bits to shift
        // We'll do a simplistic approach:
        uint8_t modrm = fetchByte();
        // maybe parse reg to see if it's CL-based shift or immediate 1
        // For demonstration, let’s do a 1-bit shift
        uint8_t reg = (modrm >> 3) & 0x07;
        uint8_t rm  = (modrm & 0x07);

        // Shift count: if opcode is D0 or D1 => 1 bit
        // if D2 or D3 => shift = CL
        // We'll just handle the 1-bit case
        uint16_t* dest;
        if(((modrm >> 6) & 0x03) == 0b11) {
            dest = getRegisterReference(rm);
        } else {
            dest = getMemoryReference((modrm >> 6) & 0x03, rm);
        }

        uint16_t before = *dest;
        uint16_t result = static_cast<uint16_t>(before << 1);

        // CF is the last bit shifted out
        bool carryOut = (before & 0x8000) != 0;
        flags.setFlag(FLAGS::CF, carryOut);

        // OF is set if sign changed (for a 1-bit shift)
        bool signChanged = ((before ^ result) & 0x8000) != 0;
        flags.setFlag(FLAGS::OF, signChanged);

        // ZF, SF, PF
        flags.setFlag(FLAGS::ZF, (result == 0));
        flags.setFlag(FLAGS::SF, (result & 0x8000) != 0);
        flags.setFlag(FLAGS::PF, utils.calculateParity(result));

        *dest = result;
    }

    void Instructions::handleSHR() {
        // Similar logic, but shifting right
        uint8_t modrm = fetchByte();
        uint8_t rm    = (modrm & 0x07);
        uint8_t mod   = (modrm >> 6) & 0x03;

        uint16_t* dest;
        if(mod == 0b11) {
            dest = getRegisterReference(rm);
        } else {
            dest = getMemoryReference(mod, rm);
        }

        uint16_t before = *dest;
        uint16_t result = static_cast<uint16_t>(before >> 1);

        // The bit shifted out is CF
        bool carryOut = (before & 0x0001) != 0;
        flags.setFlag(FLAGS::CF, carryOut);

        // OF for SHR is the old sign bit in some x86 references, but 8086 sets it to the original sign bit?
        // Typically for logical shift, OF = sign bit of original. Let's keep it simple:
        bool signBit = (before & 0x8000) != 0;
        flags.setFlag(FLAGS::OF, signBit);

        flags.setFlag(FLAGS::ZF, (result == 0));
        flags.setFlag(FLAGS::SF, (result & 0x8000) != 0); 
        flags.setFlag(FLAGS::PF, utils.calculateParity(result));

        *dest = result;
    }

    //--------------------------------------------------------------------------
    // Control Transfer: JMP, JE, JNE, etc.
    //--------------------------------------------------------------------------
    void Instructions::handleJMP() {
        int16_t offset = static_cast<int16_t>(fetchWord());
        registers.IP += offset;
    }

    void Instructions::handleJE() {
        int16_t offset = static_cast<int16_t>(fetchWord());
        if(flags.getFlag(FLAGS::ZF)) {
            registers.IP += offset;
        }
    }

    void Instructions::handleJNE() {
        int16_t offset = static_cast<int16_t>(fetchWord());
        if(!flags.getFlag(FLAGS::ZF)) {
            registers.IP += offset;
        }
    }

    void Instructions::handleJG() {
        int16_t offset = static_cast<int16_t>(fetchWord());
        // JG => ZF=0 and SF=OF
        bool cond = (!flags.getFlag(FLAGS::ZF) && (flags.getFlag(FLAGS::SF) == flags.getFlag(FLAGS::OF)));
        if(cond) {
            registers.IP += offset;
        }
    }

    void Instructions::handleJGE() {
        int16_t offset = static_cast<int16_t>(fetchWord());
        // JGE => SF=OF
        if(flags.getFlag(FLAGS::SF) == flags.getFlag(FLAGS::OF)) {
            registers.IP += offset;
        }
    }

    void Instructions::handleJL() {
        int16_t offset = static_cast<int16_t>(fetchWord());
        // JL => SF!=OF
        if(flags.getFlag(FLAGS::SF) != flags.getFlag(FLAGS::OF)) {
            registers.IP += offset;
        }
    }

    void Instructions::handleJLE() {
        int16_t offset = static_cast<int16_t>(fetchWord());
        // JLE => ZF=1 or SF!=OF
        bool cond = (flags.getFlag(FLAGS::ZF) || (flags.getFlag(FLAGS::SF) != flags.getFlag(FLAGS::OF)));
        if(cond) {
            registers.IP += offset;
        }
    }

    //--------------------------------------------------------------------------
    // Stack & Procedure: PUSH, POP, CALL, RET
    //--------------------------------------------------------------------------
    void Instructions::handlePUSH() {
        // Example: 0x50 => PUSH AX, 0x51 => PUSH CX, etc.
        // We parse the last opcode, get which reg it is, then do SP -= 2, writeWord(SS:SP, reg).
        uint8_t lastOp = memory.readByte(memory.calculatePhysicalAddress(registers.CS, registers.IP - 1));
        uint8_t regCode = lastOp & 0x07;
        uint16_t* src = getRegisterReference(regCode);

        registers.SP -= 2;
        uint32_t phys = memory.calculatePhysicalAddress(registers.SS, registers.SP);
        memory.writeWord(phys, *src);
    }

    void Instructions::handlePOP() {
        // 0x58 => POP AX, 0x59 => POP CX, etc.
        uint8_t lastOp = memory.readByte(memory.calculatePhysicalAddress(registers.CS, registers.IP - 1));
        uint8_t regCode = lastOp & 0x07;
        uint16_t* dest = getRegisterReference(regCode);

        uint32_t phys = memory.calculatePhysicalAddress(registers.SS, registers.SP);
        *dest = memory.readWord(phys);
        registers.SP += 2;
    }

    void Instructions::handleCALL() {
        // 0xE8 => CALL rel16
        // push IP, then IP += offset
        int16_t offset = static_cast<int16_t>(fetchWord());

        // push current IP onto stack
        registers.SP -= 2;
        uint32_t phys = memory.calculatePhysicalAddress(registers.SS, registers.SP);
        memory.writeWord(phys, registers.IP);

        registers.IP += offset;
    }

    void Instructions::handleRET() {
        // 0xC3 => RET near
        uint32_t phys = memory.calculatePhysicalAddress(registers.SS, registers.SP);
        registers.IP = memory.readWord(phys);
        registers.SP += 2;
    }

    //--------------------------------------------------------------------------
    // INT, HLT
    //--------------------------------------------------------------------------
    void Instructions::handleINT() {
        // 0xCD imm8 => software interrupt
        // Normally, we'd read the immediate byte, push flags, push CS, push IP, load vector from IVT, etc.
        // For now, we can stub or throw:
        throw std::runtime_error("handleINT: Not implemented. (Software interrupt)");
    }

    void Instructions::handleHLT() {
        // HLT => stop execution until interrupt
        // We'll treat it as a full halt
        halted = true;
    }

    //--------------------------------------------------------------------------
    // F6 (8-bit) and F7 (16-bit) instructions: TEST, NOT, NEG, MUL, DIV, etc.
    //--------------------------------------------------------------------------
    void Instructions::handleF6() {
        uint8_t modrm = fetchByte();
        uint8_t sub = (modrm >> 3) & 0x07;

        switch(sub) {
            case 0x00: handleTest8(modrm); break;
            case 0x02: handleNot8(modrm);  break;
            case 0x03: handleNeg8(modrm);  break;
            case 0x04: handleMul8(modrm);  break;
            case 0x05: handleIMul8(modrm); break;
            case 0x06: handleDiv8(modrm);  break;
            case 0x07: handleIDiv8(modrm); break;
            default:
                throw std::runtime_error("Unknown F6 sub-opcode (8-bit): " + std::to_string(sub));
        }
    }

    void Instructions::handleF7() {
        uint8_t modrm = fetchByte();
        uint8_t sub = (modrm >> 3) & 0x07;

        switch(sub) {
            case 0x00: handleTest16(modrm); break;
            case 0x02: handleNot16(modrm);  break;
            case 0x03: handleNeg16(modrm);  break;
            case 0x04: handleMul16(modrm);  break;
            case 0x05: handleIMul16(modrm); break;
            case 0x06: handleDiv16(modrm);  break;
            case 0x07: handleIDiv16(modrm); break;
            default:
                throw std::runtime_error("Unknown F7 sub-opcode (16-bit): " + std::to_string(sub));
        }
    }

    //--------------------------------------------------------------------------
    // F6 subroutines (8-bit)
    //--------------------------------------------------------------------------
    void Instructions::handleTest8(uint8_t modrm) {
        /*
         * TEST r/m8, imm8 => AND the operand with imm8; set flags but don't store
         * For a simple approach, we won't handle actual separate AL, AH, etc.
         * If you want to truly support 8-bit registers, you'd need to parse them from the union.
         */
        throw std::runtime_error("F6: handleTest8 not implemented");
    }

    void Instructions::handleNot8(uint8_t modrm) {
        throw std::runtime_error("F6: handleNot8 not implemented");
    }

    void Instructions::handleNeg8(uint8_t modrm) {
        throw std::runtime_error("F6: handleNeg8 not implemented");
    }

    void Instructions::handleMul8(uint8_t modrm) {
        /*
         * 8-bit MUL => AX = AL * r/m8
         * => AL:AH gets the product
         */
        throw std::runtime_error("F6: handleMul8 not implemented");
    }

    void Instructions::handleIMul8(uint8_t modrm) {
        // Signed version
        throw std::runtime_error("F6: handleIMul8 not implemented");
    }

    void Instructions::handleDiv8(uint8_t modrm) {
        /*
         * 8-bit DIV => AL = AX / r/m8, AH = remainder
         */
        throw std::runtime_error("F6: handleDiv8 not implemented");
    }

    void Instructions::handleIDiv8(uint8_t modrm) {
        // Signed version
        throw std::runtime_error("F6: handleIDiv8 not implemented");
    }

    //--------------------------------------------------------------------------
    // F7 subroutines (16-bit)
    //--------------------------------------------------------------------------
    void Instructions::handleTest16(uint8_t modrm) {
        /*
         * TEST r/m16, imm16 => AND them, set flags, no result stored
         */
        throw std::runtime_error("F7: handleTest16 not implemented");
    }

    void Instructions::handleNot16(uint8_t modrm) {
        /*
         * NOT r/m16 => invert the bits
         *   if (mod == 0b11) => it's a register
         *   else memory
         */
        uint8_t mod = (modrm >> 6) & 0x03;
        uint8_t rm  = modrm & 0x07;

        uint16_t* operand;
        if(mod == 0b11) {
            operand = getRegisterReference(rm);
        } else {
            operand = getMemoryReference(mod, rm);
        }

        *operand = ~(*operand);

        flags.setFlag(FLAGS::ZF, (*operand == 0));
        flags.setFlag(FLAGS::SF, ((*operand & 0x8000) != 0));
        flags.setFlag(FLAGS::PF, utils.calculateParity(*operand));
    }

    void Instructions::handleNeg16(uint8_t modrm) {
        /*
         * NEG r/m16 => two's complement negation => result = 0 - r/m16
         */
        uint8_t mod = (modrm >> 6) & 0x03;
        uint8_t rm  = modrm & 0x07;

        uint16_t* operand;
        if(mod == 0b11) {
            operand = getRegisterReference(rm);
        } else {
            operand = getMemoryReference(mod, rm);
        }

        uint32_t result = (uint32_t)(0) - (uint32_t)(*operand);
        setArithmeticFlags(result, 0, *operand);
        *operand = static_cast<uint16_t>(result);
    }

    void Instructions::handleMul16(uint8_t modrm) {
        /*
         * MUL r/m16 => DX:AX = AX * r/m16 (unsigned)
         */
        uint8_t mod = (modrm >> 6) & 0x03;
        uint8_t rm  = modrm & 0x07;

        uint16_t* operand;
        if(mod == 0b11) {
            operand = getRegisterReference(rm);
        } else {
            operand = getMemoryReference(mod, rm);
        }

        uint32_t product = (uint32_t)registers.AX.value * (uint32_t)(*operand);

        registers.AX.value = (uint16_t)(product & 0xFFFF);
        registers.DX.value = (uint16_t)((product >> 16) & 0xFFFF);

        // CF, OF set if product doesn't fit in AX alone
        bool overflow = (registers.DX.value != 0);
        flags.setFlag(FLAGS::CF, overflow);
        flags.setFlag(FLAGS::OF, overflow);

        flags.setFlag(FLAGS::ZF, (registers.AX.value == 0 && registers.DX.value == 0));
        flags.setFlag(FLAGS::SF, (registers.DX.value & 0x8000) != 0);
        flags.setFlag(FLAGS::PF, utils.calculateParity(registers.AX.value));
    }

    void Instructions::handleIMul16(uint8_t modrm) {
        /*
         * IMUL r/m16 => signed multiply AX * r/m16 => DX:AX
         */
        uint8_t mod = (modrm >> 6) & 0x03;
        uint8_t rm  = modrm & 0x07;

        uint16_t* operand;
        if(mod == 0b11) {
            operand = getRegisterReference(rm);
        } else {
            operand = getMemoryReference(mod, rm);
        }

        int32_t product = (int16_t)registers.AX.value * (int16_t)(*operand);

        registers.AX.value = (uint16_t)(product & 0xFFFF);
        registers.DX.value = (uint16_t)((product >> 16) & 0xFFFF);

        // CF, OF set if the sign-extended high word is not just a replicate of the sign bit
        bool overflow = false;
        // If product doesn't fit in 16 bits
        int16_t axSigned = static_cast<int16_t>(registers.AX.value);
        int16_t dxSigned = static_cast<int16_t>(registers.DX.value);

        // If dxSigned is anything other than sign-extension of axSigned, we have overflow
        // sign extension of ax means: dx = 0xFFFF if negative, or 0x0000 if positive
        if((axSigned < 0 && dxSigned != -1) || (axSigned >= 0 && dxSigned != 0)) {
            overflow = true;
        }

        flags.setFlag(FLAGS::CF, overflow);
        flags.setFlag(FLAGS::OF, overflow);
        flags.setFlag(FLAGS::ZF, (product == 0));
        flags.setFlag(FLAGS::SF, (registers.DX.value & 0x8000) != 0);
        flags.setFlag(FLAGS::PF, utils.calculateParity(registers.AX.value));
    }

    void Instructions::handleDiv16(uint8_t modrm) {
        /*
         * DIV r/m16 => (DX:AX) / r/m16 => AX=quotient, DX=remainder
         */
        uint8_t mod = (modrm >> 6) & 0x03;
        uint8_t rm  = modrm & 0x07;

        uint16_t* operand;
        if(mod == 0b11) {
            operand = getRegisterReference(rm);
        } else {
            operand = getMemoryReference(mod, rm);
        }

        uint32_t dividend = ((uint32_t)registers.DX.value << 16) | (uint32_t)registers.AX.value;
        uint16_t divisor  = *operand;

        if(divisor == 0) {
            throw std::runtime_error("Divide by zero (DIV r/m16)");
        }

        uint32_t quotient  = dividend / divisor;
        uint32_t remainder = dividend % divisor;

        // If quotient > 0xFFFF => #DE (divide error)
        if(quotient > 0xFFFF) {
            throw std::runtime_error("Divide error: quotient too large for 16-bit AX");
        }

        registers.AX.value = (uint16_t)(quotient & 0xFFFF);
        registers.DX.value = (uint16_t)(remainder & 0xFFFF);

        // If there's an overflow => set flags or throw #DE
        // For unsigned, CF/OF are undefined or set if overflow
        flags.setFlag(FLAGS::CF, false);
        flags.setFlag(FLAGS::OF, false);
        flags.setFlag(FLAGS::ZF, (quotient == 0));
        flags.setFlag(FLAGS::SF, (registers.AX.value & 0x8000) != 0);
        flags.setFlag(FLAGS::PF, utils.calculateParity(registers.AX.value));
    }

    void Instructions::handleIDiv16(uint8_t modrm) {
        /*
         * IDIV r/m16 => signed divide (DX:AX) / r/m16
         */
        uint8_t mod = (modrm >> 6) & 0x03;
        uint8_t rm  = modrm & 0x07;

        uint16_t* operand;
        if(mod == 0b11) {
            operand = getRegisterReference(rm);
        } else {
            operand = getMemoryReference(mod, rm);
        }

        int32_t dividend = ((int32_t)((int16_t)registers.DX.value) << 16) | (uint32_t)registers.AX.value;
        int16_t divisor  = (int16_t)(*operand);

        if(divisor == 0) {
            throw std::runtime_error("Divide by zero (IDIV r/m16)");
        }

        int32_t quotient  = dividend / divisor;
        int32_t remainder = dividend % divisor;

        // If quotient < -32768 or > 32767 => #DE (overflow)
        if(quotient < -32768 || quotient > 32767) {
            throw std::runtime_error("Divide error: quotient out of range for 16-bit");
        }

        registers.AX.value = (uint16_t)(quotient & 0xFFFF);
        registers.DX.value = (uint16_t)(remainder & 0xFFFF);

        // CF, OF set if overflow
        // If the quotient didn't fit in 16 bits, we would have thrown
        flags.setFlag(FLAGS::CF, false);
        flags.setFlag(FLAGS::OF, false);
        flags.setFlag(FLAGS::ZF, (quotient == 0));
        flags.setFlag(FLAGS::SF, (quotient & 0x8000) != 0);
        flags.setFlag(FLAGS::PF, utils.calculateParity(registers.AX.value));
    }

} // namespace CPU
