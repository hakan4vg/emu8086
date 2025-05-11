#include "instructions.hpp"
#include <stdexcept>
#include <iostream>
#include "../utils/utils.h"

namespace CPU {

    static cpu::UTILS utils; 

    //--------------------------------------------------------------------------
    // Constructor: populate opcodeTable
    //--------------------------------------------------------------------------
    Instructions::Instructions(Memory &mem, Registers &reg, Flags &flg, IO::IOController &ioController)
        : memory(mem), registers(reg), flags(flg), io(ioController), halted(false)
    {
        // MOV instructions (a subset of them: 88, 89, 8A, 8B)
        opcodeTable[0x88] = std::bind(&Instructions::handleMOV, this);
        opcodeTable[0x89] = std::bind(&Instructions::handleMOV, this);
        opcodeTable[0x8A] = std::bind(&Instructions::handleMOV, this);
        opcodeTable[0x8B] = std::bind(&Instructions::handleMOV, this);

        // MOV register, immediate instructions (0xB0-0xBF)
        // 8-bit registers (AL, CL, DL, BL, AH, CH, DH, BH)
        for (uint8_t op = 0xB0; op <= 0xB7; ++op) {
            opcodeTable[op] = std::bind(&Instructions::handleMOVRegImm, this);
        }
        // 16-bit registers (AX, CX, DX, BX, SP, BP, SI, DI)
        for (uint8_t op = 0xB8; op <= 0xBF; ++op) {
            opcodeTable[op] = std::bind(&Instructions::handleMOVRegImm, this);
        }

        // ADD instructions
        opcodeTable[0x00] = std::bind(&Instructions::handleADD8, this);  // ADD r/m8, r8
        opcodeTable[0x01] = std::bind(&Instructions::handleADD, this);   // ADD r/m16, r16
        opcodeTable[0x02] = std::bind(&Instructions::handleADD8, this);  // ADD r8, r/m8
        opcodeTable[0x03] = std::bind(&Instructions::handleADD, this);   // ADD r16, r/m16
        opcodeTable[0x04] = std::bind(&Instructions::handleADDImm8, this); // ADD AL, imm8
        opcodeTable[0x05] = std::bind(&Instructions::handleADDImm16, this); // ADD AX, imm16
        
        // ADC instructions (Add with Carry)
        opcodeTable[0x10] = std::bind(&Instructions::handleADC8, this);  // ADC r/m8, r8
        opcodeTable[0x11] = std::bind(&Instructions::handleADC, this);   // ADC r/m16, r16
        opcodeTable[0x12] = std::bind(&Instructions::handleADC8, this);  // ADC r8, r/m8
        opcodeTable[0x13] = std::bind(&Instructions::handleADC, this);   // ADC r16, r/m16
        
        // SUB
        opcodeTable[0x29] = std::bind(&Instructions::handleSUB, this);  // SUB r/m16, r16
        opcodeTable[0x2B] = std::bind(&Instructions::handleSUB, this);  // SUB r16, r/m16
        
        // SBB instructions (Subtract with Borrow)
        opcodeTable[0x18] = std::bind(&Instructions::handleSBB8, this);  // SBB r/m8, r8
        opcodeTable[0x19] = std::bind(&Instructions::handleSBB, this);   // SBB r/m16, r16
        opcodeTable[0x1A] = std::bind(&Instructions::handleSBB8, this);  // SBB r8, r/m8
        opcodeTable[0x1B] = std::bind(&Instructions::handleSBB, this);   // SBB r16, r/m16

        // INC and DEC (0x40-0x4F in many forms)
        for(uint8_t op = 0x40; op <= 0x47; ++op) {
            opcodeTable[op] = std::bind(&Instructions::handleINC, this);
        }
        for(uint8_t op = 0x48; op <= 0x4F; ++op) {
            opcodeTable[op] = std::bind(&Instructions::handleDEC, this);
        }

        // Flag Control Instructions
        opcodeTable[0xF8] = std::bind(&Instructions::handleCLC, this);  // CLC - Clear Carry Flag
        opcodeTable[0xF9] = std::bind(&Instructions::handleSTC, this);  // STC - Set Carry Flag
        opcodeTable[0xF5] = std::bind(&Instructions::handleCMC, this);  // CMC - Complement Carry Flag
        opcodeTable[0xFC] = std::bind(&Instructions::handleCLD, this);  // CLD - Clear Direction Flag
        opcodeTable[0xFD] = std::bind(&Instructions::handleSTD, this);  // STD - Set Direction Flag
        opcodeTable[0xFA] = std::bind(&Instructions::handleCLI, this);  // CLI - Clear Interrupt Flag
        opcodeTable[0xFB] = std::bind(&Instructions::handleSTI, this);  // STI - Set Interrupt Flag

        // CMP instructions
        opcodeTable[0x38] = std::bind(&Instructions::handleCMP, this);  // CMP r/m8, r8
        opcodeTable[0x39] = std::bind(&Instructions::handleCMP, this);  // CMP r/m16, r16
        opcodeTable[0x3A] = std::bind(&Instructions::handleCMP, this);  // CMP r8, r/m8
        opcodeTable[0x3B] = std::bind(&Instructions::handleCMP, this);  // CMP r16, r/m16
        opcodeTable[0x3C] = std::bind(&Instructions::handleCMPImm, this); // CMP AL, imm8
        opcodeTable[0x3D] = std::bind(&Instructions::handleCMPImm, this); // CMP AX, imm16
        
        // Group 1 instructions (including CMP r/m, imm)
        opcodeTable[0x80] = std::bind(&Instructions::handleGroup1, this); // CMP r/m8, imm8
        opcodeTable[0x81] = std::bind(&Instructions::handleGroup1, this); // CMP r/m16, imm16
        opcodeTable[0x83] = std::bind(&Instructions::handleGroup1, this); // CMP r/m16, imm8 (sign-extended)

        // String operations
        opcodeTable[0xA4] = std::bind(&Instructions::handleMOVS, this); // MOVSB
        opcodeTable[0xA5] = std::bind(&Instructions::handleMOVS, this); // MOVSW
        opcodeTable[0xA6] = std::bind(&Instructions::handleCMPS, this); // CMPSB
        opcodeTable[0xA7] = std::bind(&Instructions::handleCMPS, this); // CMPSW
        opcodeTable[0xAA] = std::bind(&Instructions::handleSTOS, this); // STOSB
        opcodeTable[0xAB] = std::bind(&Instructions::handleSTOS, this); // STOSW
        opcodeTable[0xAC] = std::bind(&Instructions::handleLODS, this); // LODSB
        opcodeTable[0xAD] = std::bind(&Instructions::handleLODS, this); // LODSW
        opcodeTable[0xAE] = std::bind(&Instructions::handleSCAS, this); // SCASB
        opcodeTable[0xAF] = std::bind(&Instructions::handleSCAS, this); // SCASW
        opcodeTable[0xF2] = std::bind(&Instructions::handleREP, this);  // REPNE/REPNZ
        opcodeTable[0xF3] = std::bind(&Instructions::handleREP, this);  // REP/REPE/REPZ

        // I/O operations
        opcodeTable[0xE4] = std::bind(&Instructions::handleIN, this);   // IN AL, imm8
        opcodeTable[0xE5] = std::bind(&Instructions::handleIN, this);   // IN AX, imm8
        opcodeTable[0xEC] = std::bind(&Instructions::handleIN, this);   // IN AL, DX
        opcodeTable[0xED] = std::bind(&Instructions::handleIN, this);   // IN AX, DX
        opcodeTable[0xE6] = std::bind(&Instructions::handleOUT, this);  // OUT imm8, AL
        opcodeTable[0xE7] = std::bind(&Instructions::handleOUT, this);  // OUT imm8, AX
        opcodeTable[0xEE] = std::bind(&Instructions::handleOUT, this);  // OUT DX, AL
        opcodeTable[0xEF] = std::bind(&Instructions::handleOUT, this);  // OUT DX, AX

        // Jumps
        opcodeTable[0xEB] = std::bind(&Instructions::handleJMP, this);  // Short jump
        opcodeTable[0xE9] = std::bind(&Instructions::handleJMP, this);  // Near jump
        opcodeTable[0x74] = std::bind(&Instructions::handleJE, this);
        opcodeTable[0x75] = std::bind(&Instructions::handleJNE, this);
        opcodeTable[0x77] = std::bind(&Instructions::handleJG, this);
        opcodeTable[0x7D] = std::bind(&Instructions::handleJGE, this);
        opcodeTable[0x7C] = std::bind(&Instructions::handleJL, this);
        opcodeTable[0x7E] = std::bind(&Instructions::handleJLE, this);

        // INT and HLT
        opcodeTable[0xCD] = std::bind(&Instructions::handleINT, this);
        opcodeTable[0xF4] = std::bind(&Instructions::handleHLT, this);

        // Logical ops: AND, OR, XOR, NOT
        opcodeTable[0x20] = std::bind(&Instructions::handleAND, this);
        opcodeTable[0x21] = std::bind(&Instructions::handleAND, this);
        opcodeTable[0x22] = std::bind(&Instructions::handleAND, this);
        opcodeTable[0x23] = std::bind(&Instructions::handleAND, this);
        opcodeTable[0x24] = std::bind(&Instructions::handleANDImm, this); // AND AL, imm8
        opcodeTable[0x25] = std::bind(&Instructions::handleANDImm, this); // AND AX, imm16
        
        opcodeTable[0x08] = std::bind(&Instructions::handleOR,  this);
        opcodeTable[0x09] = std::bind(&Instructions::handleOR,  this);
        opcodeTable[0x0A] = std::bind(&Instructions::handleOR,  this);
        opcodeTable[0x0B] = std::bind(&Instructions::handleOR,  this);
        opcodeTable[0x0C] = std::bind(&Instructions::handleORImm,  this); // OR AL, imm8
        opcodeTable[0x0D] = std::bind(&Instructions::handleORImm,  this); // OR AX, imm16
        
        opcodeTable[0x30] = std::bind(&Instructions::handleXOR, this);
        opcodeTable[0x31] = std::bind(&Instructions::handleXOR, this);
        opcodeTable[0x32] = std::bind(&Instructions::handleXOR, this);
        opcodeTable[0x33] = std::bind(&Instructions::handleXOR, this);
        opcodeTable[0x34] = std::bind(&Instructions::handleXORImm, this); // XOR AL, imm8
        opcodeTable[0x35] = std::bind(&Instructions::handleXORImm, this); // XOR AX, imm16

        // SHIFT/ROTATE (D0, D1, D2, D3 for certain ops)
        opcodeTable[0xD0] = std::bind(&Instructions::handleROL, this); // 8-bit shift/rotate by 1
        opcodeTable[0xD1] = std::bind(&Instructions::handleROL, this); // 16-bit shift/rotate by 1
        opcodeTable[0xD2] = std::bind(&Instructions::handleROL, this); // 8-bit shift/rotate by CL
        opcodeTable[0xD3] = std::bind(&Instructions::handleROL, this); // 16-bit shift/rotate by CL

        // PUSH/POP (examples: 0x50-0x5F for push/pop reg)
        // CALL/RET (0xE8, 0xC3, etc.)
        // For brevity, we'll just map a couple:
        opcodeTable[0x50] = std::bind(&Instructions::handlePUSH, this); // PUSH AX
        opcodeTable[0x51] = std::bind(&Instructions::handlePUSH, this); // PUSH CX
        opcodeTable[0x58] = std::bind(&Instructions::handlePOP,  this); // POP AX
        opcodeTable[0x59] = std::bind(&Instructions::handlePOP,  this); // POP CX
        opcodeTable[0xE8] = std::bind(&Instructions::handleCALL, this);
        opcodeTable[0xC3] = std::bind(&Instructions::handleRET,  this);
        opcodeTable[0xCF] = std::bind(&Instructions::handleIRET, this); // Add IRET (0xCF)

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

    // Update decodeAndExecute to return cycle count
    uint32_t Instructions::decodeAndExecute(uint8_t opcode) {
        auto it = opcodeTable.find(opcode);
        if(it != opcodeTable.end()) {
            return it->second();
        } else {
            throw std::runtime_error("Unknown opcode: " + std::to_string(opcode));
        }
    }

    // Update executeNext to return cycle count
    uint32_t Instructions::executeNext() {
        if(halted) {
            return 0;
        }
        uint8_t opcode = fetchByte();
        return decodeAndExecute(opcode);
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
        // For addition: If both operands have the same sign but the result has a different sign
        // For subtraction: If the operands have different signs and the result has the same sign as the source
        bool isAddition = ((result & 0x10000) == (result & 0xFFFF)); // Check if carry matches highest bit
        
        if (isAddition) {
            // Addition: Both operands have same sign bit but result has different sign bit
            bool of = ((dest & 0x8000) == (src & 0x8000)) && ((dest & 0x8000) != (res16 & 0x8000));
            flags.setFlag(FLAGS::OF, of);
        } else {
            // Subtraction: Operands have different sign bit, and result sign bit equals subtrahend
            bool of = ((dest & 0x8000) != (src & 0x8000)) && ((src & 0x8000) == (res16 & 0x8000));
            flags.setFlag(FLAGS::OF, of);
        }

        // Parity (number of 1-bits in low 8 bits is even)
        uint8_t lowByte = res16 & 0xFF;
        uint8_t count = 0;
        for (int i = 0; i < 8; i++) {
            if (lowByte & (1 << i)) count++;
        }
        flags.setFlag(FLAGS::PF, (count % 2) == 0);

        // Auxiliary Carry (carry from bit 3 to bit 4)
        // For addition: ((op1 & 0xF) + (op2 & 0xF)) > 0xF
        // For subtraction: ((op1 & 0xF) - (op2 & 0xF)) < 0
        if (isAddition) {
            flags.setFlag(FLAGS::AF, ((dest & 0xF) + (src & 0xF)) > 0xF);
        } else {
            flags.setFlag(FLAGS::AF, ((dest & 0xF) - (src & 0xF)) & 0x10);
        }
    }

    void Instructions::setArithmeticFlags8(uint16_t result, uint8_t dest, uint8_t src) {
        // Flags for 8-bit arithmetic
        uint8_t res8 = static_cast<uint8_t>(result);

        // Zero
        flags.setFlag(FLAGS::ZF, (res8 == 0));

        // Carry if result didn't fit in 8 bits
        flags.setFlag(FLAGS::CF, (result > 0xFF));

        // Sign
        flags.setFlag(FLAGS::SF, (res8 & 0x80) != 0);

        // Overflow (for signed)
        // 8-bit signed overflow check
        bool isAddition = ((result & 0x100) == (result & 0xFF)); // Check if carry matches highest bit
        
        if (isAddition) {
            // Addition: Both operands have same sign bit but result has different sign bit
            bool of = ((dest & 0x80) == (src & 0x80)) && ((dest & 0x80) != (res8 & 0x80));
            flags.setFlag(FLAGS::OF, of);
        } else {
            // Subtraction: Operands have different sign bit, and result sign bit equals subtrahend
            bool of = ((dest & 0x80) != (src & 0x80)) && ((src & 0x80) == (res8 & 0x80));
            flags.setFlag(FLAGS::OF, of);
        }

        // Parity (number of 1-bits is even)
        uint8_t count = 0;
        for (int i = 0; i < 8; i++) {
            if (res8 & (1 << i)) count++;
        }
        flags.setFlag(FLAGS::PF, (count % 2) == 0);

        // Auxiliary Carry (carry from bit 3 to bit 4)
        if (isAddition) {
            flags.setFlag(FLAGS::AF, ((dest & 0xF) + (src & 0xF)) > 0xF);
        } else {
            flags.setFlag(FLAGS::AF, ((dest & 0xF) - (src & 0xF)) & 0x10);
        }
    }

    //--------------------------------------------------------------------------
    // Data Movement: MOV
    //--------------------------------------------------------------------------
    uint32_t Instructions::handleMOV() {
        uint8_t modrm = fetchByte();
        uint8_t mod = (modrm >> 6) & 0x03;
        uint8_t reg = (modrm >> 3) & 0x07;
        uint8_t rm = modrm & 0x07;
        
        uint8_t lastOpcode = memory.readByte(memory.calculatePhysicalAddress(registers.CS, registers.IP - 2));
        bool isWord = (lastOpcode & 0x01) != 0;
        bool direction = (lastOpcode & 0x02) != 0;
        
        uint32_t cycleCount = 0;
        
        if (direction) {
            // MOV reg, r/m (8A/8B)
            if (mod == 0b11) {
                // Register to register (shorter cycle time)
                if (isWord) {
                    uint16_t* dest = getRegisterReference(reg);
                    uint16_t* src = getRegisterReference(rm);
                    *dest = *src;
                } else {
                    uint8_t* dest = get8BitRegisterRef(reg);
                    uint8_t* src = get8BitRegisterRef(rm);
                    *dest = *src;
                }
                cycleCount = cycles.MOV_REG_REG;
            } else {
                // Memory to register
                if (isWord) {
                    uint16_t* dest = getRegisterReference(reg);
                    uint32_t addr = getEffectiveAddress(mod, rm);
                    *dest = memory.readWord(addr);
                } else {
                    uint8_t* dest = get8BitRegisterRef(reg);
                    uint32_t addr = getEffectiveAddress(mod, rm);
                    *dest = memory.readByte(addr);
                }
                cycleCount = cycles.MOV_MEM_REG;
            }
        } else {
            // MOV r/m, reg (88/89)
            if (mod == 0b11) {
                // Register to register
                if (isWord) {
                    uint16_t* dest = getRegisterReference(rm);
                    uint16_t* src = getRegisterReference(reg);
                    *dest = *src;
                } else {
                    uint8_t* dest = get8BitRegisterRef(rm);
                    uint8_t* src = get8BitRegisterRef(reg);
                    *dest = *src;
                }
                cycleCount = cycles.MOV_REG_REG;
            } else {
                // Register to memory
                if (isWord) {
                    uint16_t* src = getRegisterReference(reg);
                    uint32_t addr = getEffectiveAddress(mod, rm);
                    memory.writeWord(addr, *src);
                } else {
                    uint8_t* src = get8BitRegisterRef(reg);
                    uint32_t addr = getEffectiveAddress(mod, rm);
                    memory.writeByte(addr, *src);
                }
                cycleCount = cycles.MOV_REG_MEM;
            }
        }
        
        return cycleCount;
    }

    //--------------------------------------------------------------------------
    // MOV register, immediate
    //--------------------------------------------------------------------------
    uint32_t Instructions::handleMOVRegImm() {
        uint8_t lastOpcode = memory.readByte(memory.calculatePhysicalAddress(registers.CS, registers.IP - 1));
        uint8_t regCode = lastOpcode & 0x07;
        bool isWord = (lastOpcode >= 0xB8);
        
        if (isWord) {
            // 16-bit
            uint16_t* reg = getRegisterReference(regCode);
            uint16_t imm = fetchWord();
            *reg = imm;
        } else {
            // 8-bit
            uint8_t* reg = get8BitRegisterRef(regCode);
            uint8_t imm = fetchByte();
            *reg = imm;
        }
        
        return cycles.MOV_IMM_REG; // Return cycle count
    }

    //--------------------------------------------------------------------------
    // Arithmetic: ADD, SUB, CMP, INC, DEC
    //--------------------------------------------------------------------------
    uint32_t Instructions::handleADD() {
        uint8_t modrm = fetchByte();
        uint8_t mod   = (modrm >> 6) & 0x03;
        uint8_t reg   = (modrm >> 3) & 0x07;
        uint8_t rm    = (modrm & 0x07);

        uint16_t* dest;
        uint16_t  srcVal;
        uint32_t cycleCount = 0;

        if(mod == 0b11) {
            dest   = getRegisterReference(rm);
            srcVal = *getRegisterReference(reg);
            cycleCount = cycles.ALU_REG_REG;
        } else {
            dest   = getMemoryReference(mod, rm);
            srcVal = *getRegisterReference(reg);
            cycleCount = cycles.ALU_REG_MEM;
        }

        uint32_t result = static_cast<uint32_t>(*dest) + static_cast<uint32_t>(srcVal);
        setArithmeticFlags(result, *dest, srcVal);
        *dest = static_cast<uint16_t>(result);
        
        return cycleCount;
    }

    uint32_t Instructions::handleSUB() {
        uint8_t modrm = fetchByte();
        uint8_t mod   = (modrm >> 6) & 0x03;
        uint8_t reg   = (modrm >> 3) & 0x07;
        uint8_t rm    = (modrm & 0x07);

        uint16_t* dest;
        uint16_t  srcVal;
        uint32_t cycleCount = 0;

        if(mod == 0b11) {
            dest   = getRegisterReference(rm);
            srcVal = *getRegisterReference(reg);
            cycleCount = cycles.ALU_REG_REG;
        } else {
            dest   = getMemoryReference(mod, rm);
            srcVal = *getRegisterReference(reg);
            cycleCount = cycles.ALU_REG_MEM;
        }

        uint32_t result = static_cast<uint32_t>(*dest) - static_cast<uint32_t>(srcVal);
        setArithmeticFlags(result, *dest, srcVal);
        *dest = static_cast<uint16_t>(result);
        
        return cycleCount;
    }

    uint32_t Instructions::handleCMP() {
        uint8_t modrm = fetchByte();
        uint8_t mod   = (modrm >> 6) & 0x03;
        uint8_t reg   = (modrm >> 3) & 0x07;
        uint8_t rm    = (modrm & 0x07);

        uint16_t* dest;
        uint16_t  srcVal;
        uint32_t cycleCount = 0;

        if(mod == 0b11) {
            dest   = getRegisterReference(rm);
            srcVal = *getRegisterReference(reg);
            cycleCount = cycles.ALU_REG_REG;
        } else {
            dest   = getMemoryReference(mod, rm);
            srcVal = *getRegisterReference(reg);
            cycleCount = cycles.ALU_MEM_REG;
        }

        uint32_t result = static_cast<uint32_t>(*dest) - static_cast<uint32_t>(srcVal);
        setArithmeticFlags(result, *dest, srcVal);
        
        return cycleCount;
    }

    uint32_t Instructions::handleCMPImm() {
        // Handle CMP AL, imm8 (3C) and CMP AX, imm16 (3D)
        uint8_t opcode = memory.readByte(memory.calculatePhysicalAddress(registers.CS, registers.IP - 1));
        uint32_t cycleCount = cycles.ALU_IMM_REG;
        
        if (opcode == 0x3C) {
            // CMP AL, imm8
            uint8_t imm8 = fetchByte();
            uint8_t al = registers.AX.low;
            
            uint16_t result = al - imm8;
            setArithmeticFlags8(result, al, imm8);
        } else if (opcode == 0x3D) {
            // CMP AX, imm16
            uint16_t imm16 = fetchWord();
            uint16_t ax = registers.AX.value;
            
            uint32_t result = static_cast<uint32_t>(ax) - static_cast<uint32_t>(imm16);
            setArithmeticFlags(result, ax, imm16);
        }
        
        return cycleCount;
    }

    uint32_t Instructions::handleGroup1() {
        uint8_t opcode = memory.readByte(memory.calculatePhysicalAddress(registers.CS, registers.IP - 1));
        uint8_t modrm = fetchByte();
        uint8_t mod = (modrm >> 6) & 0x03;
        uint8_t reg = (modrm >> 3) & 0x07;
        uint8_t rm = modrm & 0x07;
        
        uint32_t cycleCount = 0;
        
        // Group 1 operations: ADD(0), OR(1), ADC(2), SBB(3), AND(4), SUB(5), XOR(6), CMP(7)
        
        if (opcode == 0x80) {  // 8-bit operands
            uint8_t imm8 = fetchByte();
            
            if (mod == 0b11) {  // Register operand
                uint8_t* dest = get8BitRegisterRef(rm);
                uint8_t oldValue = *dest;
                uint16_t result = 0;
                
                // Process based on operation type
                switch (reg) {
                    case 0: // ADD
                        result = oldValue + imm8;
                        *dest = result & 0xFF;
                        break;
                    case 1: // OR
                        result = oldValue | imm8;
                        *dest = result & 0xFF;
                        break;
                    case 2: // ADC (Add with Carry)
                        result = oldValue + imm8 + (flags.getFlag(FLAGS::CF) ? 1 : 0);
                        *dest = result & 0xFF;
                        break;
                    case 3: // SBB (Subtract with Borrow)
                        result = oldValue - imm8 - (flags.getFlag(FLAGS::CF) ? 1 : 0);
                        *dest = result & 0xFF;
                        break;
                    case 4: // AND
                        result = oldValue & imm8;
                        *dest = result & 0xFF;
                        break;
                    case 5: // SUB
                        result = oldValue - imm8;
                        *dest = result & 0xFF;
                        break;
                    case 6: // XOR
                        result = oldValue ^ imm8;
                        *dest = result & 0xFF;
                        break;
                    case 7: // CMP (don't update destination)
                        result = oldValue - imm8;
                        break;
                }
                
                setArithmeticFlags8(result, oldValue, imm8);
                cycleCount = cycles.ALU_IMM_REG;
            } else {  // Memory operand
                uint32_t addr = getEffectiveAddress(mod, rm);
                uint8_t value = memory.readByte(addr);
                uint16_t result = 0;
                
                // Process based on operation type
                switch (reg) {
                    case 0: // ADD
                        result = value + imm8;
                        memory.writeByte(addr, result & 0xFF);
                        break;
                    case 1: // OR
                        result = value | imm8;
                        memory.writeByte(addr, result & 0xFF);
                        break;
                    case 2: // ADC (Add with Carry)
                        result = value + imm8 + (flags.getFlag(FLAGS::CF) ? 1 : 0);
                        memory.writeByte(addr, result & 0xFF);
                        break;
                    case 3: // SBB (Subtract with Borrow)
                        result = value - imm8 - (flags.getFlag(FLAGS::CF) ? 1 : 0);
                        memory.writeByte(addr, result & 0xFF);
                        break;
                    case 4: // AND
                        result = value & imm8;
                        memory.writeByte(addr, result & 0xFF);
                        break;
                    case 5: // SUB
                        result = value - imm8;
                        memory.writeByte(addr, result & 0xFF);
                        break;
                    case 6: // XOR
                        result = value ^ imm8;
                        memory.writeByte(addr, result & 0xFF);
                        break;
                    case 7: // CMP (don't update destination)
                        result = value - imm8;
                        break;
                }
                
                setArithmeticFlags8(result, value, imm8);
                cycleCount = cycles.ALU_IMM_MEM;
            }
        } else if (opcode == 0x81) {  // 16-bit operands
            uint16_t imm16 = fetchWord();
            
            if (mod == 0b11) {  // Register operand
                uint16_t* dest = getRegisterReference(rm);
                uint16_t oldValue = *dest;
                uint32_t result = 0;
                
                // Process based on operation type
                switch (reg) {
                    case 0: // ADD
                        result = static_cast<uint32_t>(oldValue) + static_cast<uint32_t>(imm16);
                        *dest = result & 0xFFFF;
                        break;
                    case 1: // OR
                        result = oldValue | imm16;
                        *dest = result & 0xFFFF;
                        break;
                    case 2: // ADC (Add with Carry)
                        result = static_cast<uint32_t>(oldValue) + static_cast<uint32_t>(imm16) + (flags.getFlag(FLAGS::CF) ? 1 : 0);
                        *dest = result & 0xFFFF;
                        break;
                    case 3: // SBB (Subtract with Borrow)
                        result = static_cast<uint32_t>(oldValue) - static_cast<uint32_t>(imm16) - (flags.getFlag(FLAGS::CF) ? 1 : 0);
                        *dest = result & 0xFFFF;
                        break;
                    case 4: // AND
                        result = oldValue & imm16;
                        *dest = result & 0xFFFF;
                        break;
                    case 5: // SUB
                        result = static_cast<uint32_t>(oldValue) - static_cast<uint32_t>(imm16);
                        *dest = result & 0xFFFF;
                        break;
                    case 6: // XOR
                        result = oldValue ^ imm16;
                        *dest = result & 0xFFFF;
                        break;
                    case 7: // CMP (don't update destination)
                        result = static_cast<uint32_t>(oldValue) - static_cast<uint32_t>(imm16);
                        break;
                }
                
                setArithmeticFlags(result, oldValue, imm16);
                cycleCount = cycles.ALU_IMM_REG;
            } else {  // Memory operand
                uint32_t addr = getEffectiveAddress(mod, rm);
                uint16_t value = memory.readWord(addr);
                uint32_t result = 0;
                
                // Process based on operation type
                switch (reg) {
                    case 0: // ADD
                        result = static_cast<uint32_t>(value) + static_cast<uint32_t>(imm16);
                        memory.writeWord(addr, result & 0xFFFF);
                        break;
                    case 1: // OR
                        result = value | imm16;
                        memory.writeWord(addr, result & 0xFFFF);
                        break;
                    case 2: // ADC (Add with Carry)
                        result = static_cast<uint32_t>(value) + static_cast<uint32_t>(imm16) + (flags.getFlag(FLAGS::CF) ? 1 : 0);
                        memory.writeWord(addr, result & 0xFFFF);
                        break;
                    case 3: // SBB (Subtract with Borrow)
                        result = static_cast<uint32_t>(value) - static_cast<uint32_t>(imm16) - (flags.getFlag(FLAGS::CF) ? 1 : 0);
                        memory.writeWord(addr, result & 0xFFFF);
                        break;
                    case 4: // AND
                        result = value & imm16;
                        memory.writeWord(addr, result & 0xFFFF);
                        break;
                    case 5: // SUB
                        result = static_cast<uint32_t>(value) - static_cast<uint32_t>(imm16);
                        memory.writeWord(addr, result & 0xFFFF);
                        break;
                    case 6: // XOR
                        result = value ^ imm16;
                        memory.writeWord(addr, result & 0xFFFF);
                        break;
                    case 7: // CMP (don't update destination)
                        result = static_cast<uint32_t>(value) - static_cast<uint32_t>(imm16);
                        break;
                }
                
                setArithmeticFlags(result, value, imm16);
                cycleCount = cycles.ALU_IMM_MEM;
            }
        } else if (opcode == 0x83) {  // 16-bit operands, sign-extended 8-bit immediate
            uint8_t imm8 = fetchByte();
            int16_t signExtImm = static_cast<int8_t>(imm8);  // Sign extend 8-bit to 16-bit
            
            if (mod == 0b11) {  // Register operand
                uint16_t* dest = getRegisterReference(rm);
                uint16_t oldValue = *dest;
                uint32_t result = 0;
                
                // Process based on operation type
                switch (reg) {
                    case 0: // ADD
                        result = static_cast<uint32_t>(oldValue) + static_cast<uint32_t>(signExtImm);
                        *dest = result & 0xFFFF;
                        break;
                    case 1: // OR
                        result = oldValue | signExtImm;
                        *dest = result & 0xFFFF;
                        break;
                    case 2: // ADC (Add with Carry)
                        result = static_cast<uint32_t>(oldValue) + static_cast<uint32_t>(signExtImm) + (flags.getFlag(FLAGS::CF) ? 1 : 0);
                        *dest = result & 0xFFFF;
                        break;
                    case 3: // SBB (Subtract with Borrow)
                        result = static_cast<uint32_t>(oldValue) - static_cast<uint32_t>(signExtImm) - (flags.getFlag(FLAGS::CF) ? 1 : 0);
                        *dest = result & 0xFFFF;
                        break;
                    case 4: // AND
                        result = oldValue & signExtImm;
                        *dest = result & 0xFFFF;
                        break;
                    case 5: // SUB
                        result = static_cast<uint32_t>(oldValue) - static_cast<uint32_t>(signExtImm);
                        *dest = result & 0xFFFF;
                        break;
                    case 6: // XOR
                        result = oldValue ^ signExtImm;
                        *dest = result & 0xFFFF;
                        break;
                    case 7: // CMP (don't update destination)
                        result = static_cast<uint32_t>(oldValue) - static_cast<uint32_t>(signExtImm);
                        break;
                }
                
                setArithmeticFlags(result, oldValue, signExtImm);
                cycleCount = cycles.ALU_IMM_REG;
            } else {  // Memory operand
                uint32_t addr = getEffectiveAddress(mod, rm);
                uint16_t value = memory.readWord(addr);
                uint32_t result = 0;
                
                // Process based on operation type
                switch (reg) {
                    case 0: // ADD
                        result = static_cast<uint32_t>(value) + static_cast<uint32_t>(signExtImm);
                        memory.writeWord(addr, result & 0xFFFF);
                        break;
                    case 1: // OR
                        result = value | signExtImm;
                        memory.writeWord(addr, result & 0xFFFF);
                        break;
                    case 2: // ADC (Add with Carry)
                        result = static_cast<uint32_t>(value) + static_cast<uint32_t>(signExtImm) + (flags.getFlag(FLAGS::CF) ? 1 : 0);
                        memory.writeWord(addr, result & 0xFFFF);
                        break;
                    case 3: // SBB (Subtract with Borrow)
                        result = static_cast<uint32_t>(value) - static_cast<uint32_t>(signExtImm) - (flags.getFlag(FLAGS::CF) ? 1 : 0);
                        memory.writeWord(addr, result & 0xFFFF);
                        break;
                    case 4: // AND
                        result = value & signExtImm;
                        memory.writeWord(addr, result & 0xFFFF);
                        break;
                    case 5: // SUB
                        result = static_cast<uint32_t>(value) - static_cast<uint32_t>(signExtImm);
                        memory.writeWord(addr, result & 0xFFFF);
                        break;
                    case 6: // XOR
                        result = value ^ signExtImm;
                        memory.writeWord(addr, result & 0xFFFF);
                        break;
                    case 7: // CMP (don't update destination)
                        result = static_cast<uint32_t>(value) - static_cast<uint32_t>(signExtImm);
                        break;
                }
                
                setArithmeticFlags(result, value, signExtImm);
                cycleCount = cycles.ALU_IMM_MEM;
            }
        }
        
        return cycleCount;
    }

    uint8_t* Instructions::get8BitRegisterRef(uint8_t reg) {
        switch (reg) {
            case 0: return &registers.AX.low;    // AL
            case 1: return &registers.CX.low;    // CL
            case 2: return &registers.DX.low;    // DL
            case 3: return &registers.BX.low;    // BL
            case 4: return &registers.AX.high;   // AH
            case 5: return &registers.CX.high;   // CH
            case 6: return &registers.DX.high;   // DH
            case 7: return &registers.BX.high;   // BH
            default:
                throw std::runtime_error("Invalid 8-bit register code");
        }
    }

    //--------------------------------------------------------------------------
    // INC and DEC
    //--------------------------------------------------------------------------
    uint32_t Instructions::handleINC() {
        // 0x40-0x47 => inc register
        uint8_t lastOpcode = memory.readByte(memory.calculatePhysicalAddress(registers.CS, registers.IP - 1));
        uint8_t regCode = lastOpcode & 0x07;  // e.g. 0x40 => 0, 0x41 => 1, etc.

        uint16_t* dest = getRegisterReference(regCode);

        uint32_t result = static_cast<uint32_t>(*dest) + 1;
        setArithmeticFlags(result, *dest, 1);
        *dest = static_cast<uint16_t>(result);
        
        return cycles.INC_REG;
    }

    uint32_t Instructions::handleDEC() {
        // 0x48-0x4F => dec register
        uint8_t lastOpcode = memory.readByte(memory.calculatePhysicalAddress(registers.CS, registers.IP - 1));
        uint8_t regCode = lastOpcode & 0x07;

        uint16_t* dest = getRegisterReference(regCode);

        uint32_t result = static_cast<uint32_t>(*dest) - 1;
        setArithmeticFlags(result, *dest, 1);
        *dest = static_cast<uint16_t>(result);
        
        return cycles.INC_REG;
    }

    //--------------------------------------------------------------------------
    // Logic: AND, OR, XOR, NOT
    //--------------------------------------------------------------------------
    uint32_t Instructions::handleAND() {
        uint8_t modrm = fetchByte();
        uint8_t mod   = (modrm >> 6) & 0x03;
        uint8_t reg   = (modrm >> 3) & 0x07;
        uint8_t rm    = (modrm & 0x07);

        uint16_t* dest;
        uint16_t  srcVal;
        uint32_t cycleCount = 0;

        if(mod == 0b11) {
            dest   = getRegisterReference(rm);
            srcVal = *getRegisterReference(reg);
            cycleCount = cycles.ALU_REG_REG;
        } else {
            dest   = getMemoryReference(mod, rm);
            srcVal = *getRegisterReference(reg);
            cycleCount = cycles.ALU_REG_MEM;
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
        
        return cycleCount;
    }

    uint32_t Instructions::handleOR() {
        uint8_t modrm = fetchByte();
        uint8_t mod   = (modrm >> 6) & 0x03;
        uint8_t reg   = (modrm >> 3) & 0x07;
        uint8_t rm    = (modrm & 0x07);

        uint16_t* dest;
        uint16_t  srcVal;
        uint32_t cycleCount = 0;

        if(mod == 0b11) {
            dest   = getRegisterReference(rm);
            srcVal = *getRegisterReference(reg);
            cycleCount = cycles.ALU_REG_REG;
        } else {
            dest   = getMemoryReference(mod, rm);
            srcVal = *getRegisterReference(reg);
            cycleCount = cycles.ALU_REG_MEM;
        }

        uint16_t result = (*dest) | srcVal;
        *dest = result;

        flags.setFlag(FLAGS::ZF, (result == 0));
        flags.setFlag(FLAGS::SF, (result & 0x8000) != 0);
        flags.setFlag(FLAGS::OF, false);
        flags.setFlag(FLAGS::CF, false);
        flags.setFlag(FLAGS::AF, false);
        flags.setFlag(FLAGS::PF, utils.calculateParity(result));
        
        return cycleCount;
    }

    uint32_t Instructions::handleXOR() {
        uint8_t modrm = fetchByte();
        uint8_t mod   = (modrm >> 6) & 0x03;
        uint8_t reg   = (modrm >> 3) & 0x07;
        uint8_t rm    = (modrm & 0x07);

        uint16_t* dest;
        uint16_t  srcVal;
        uint32_t cycleCount = 0;

        if(mod == 0b11) {
            dest   = getRegisterReference(rm);
            srcVal = *getRegisterReference(reg);
            cycleCount = cycles.ALU_REG_REG;
        } else {
            dest   = getMemoryReference(mod, rm);
            srcVal = *getRegisterReference(reg);
            cycleCount = cycles.ALU_REG_MEM;
        }

        uint16_t result = (*dest) ^ srcVal;
        *dest = result;

        flags.setFlag(FLAGS::ZF, (result == 0));
        flags.setFlag(FLAGS::SF, (result & 0x8000) != 0);
        flags.setFlag(FLAGS::OF, false);
        flags.setFlag(FLAGS::CF, false);
        flags.setFlag(FLAGS::AF, false);
        flags.setFlag(FLAGS::PF, utils.calculateParity(result));
        
        return cycleCount;
    }

    uint32_t Instructions::handleNOT() {
        uint8_t modrm = fetchByte();
        uint8_t mod   = (modrm >> 6) & 0x03;
        uint8_t rm    = (modrm & 0x07);
        uint32_t cycleCount = 0;

        uint16_t* dest;
        if(mod == 0b11) {
            dest = getRegisterReference(rm);
            cycleCount = cycles.ALU_REG_REG;
        } else {
            dest = getMemoryReference(mod, rm);
            cycleCount = cycles.ALU_MEM_REG;
        }

        *dest = ~(*dest);

        flags.setFlag(FLAGS::ZF, (*dest == 0));
        flags.setFlag(FLAGS::SF, (*dest & 0x8000) != 0);
        flags.setFlag(FLAGS::PF, utils.calculateParity(*dest));
        
        return cycleCount;
    }

    //--------------------------------------------------------------------------
    // SHL / SHR
    //--------------------------------------------------------------------------
    uint32_t Instructions::handleSHL() {
        // For 0xD0, 0xD1, 0xD2, 0xD3, check how many bits to shift
        uint8_t modrm = fetchByte();
        uint8_t reg = (modrm >> 3) & 0x07;
        uint8_t rm  = (modrm & 0x07);
        uint8_t mod = (modrm >> 6) & 0x03;
        uint32_t cycleCount = 0;

        // Shift count: if opcode is D0 or D1 => 1 bit
        // if D2 or D3 => shift = CL
        uint16_t* dest;
        if(mod == 0b11) {
            dest = getRegisterReference(rm);
            cycleCount = cycles.SHIFT_REG_1;
        } else {
            dest = getMemoryReference(mod, rm);
            cycleCount = cycles.SHIFT_MEM_1;
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
        
        return cycleCount;
    }

    uint32_t Instructions::handleSHR() {
        uint8_t modrm = fetchByte();
        uint8_t rm    = (modrm & 0x07);
        uint8_t mod   = (modrm >> 6) & 0x03;
        uint32_t cycleCount = 0;

        uint16_t* dest;
        if(mod == 0b11) {
            dest = getRegisterReference(rm);
            cycleCount = cycles.SHIFT_REG_1;
        } else {
            dest = getMemoryReference(mod, rm);
            cycleCount = cycles.SHIFT_MEM_1;
        }

        uint16_t before = *dest;
        uint16_t result = static_cast<uint16_t>(before >> 1);

        bool carryOut = (before & 0x0001) != 0;
        flags.setFlag(FLAGS::CF, carryOut);

        bool signBit = (before & 0x8000) != 0;
        flags.setFlag(FLAGS::OF, signBit);

        flags.setFlag(FLAGS::ZF, (result == 0));
        flags.setFlag(FLAGS::SF, (result & 0x8000) != 0); 
        flags.setFlag(FLAGS::PF, utils.calculateParity(result));

        *dest = result;
        
        return cycleCount;
    }

    //--------------------------------------------------------------------------
    // Control Transfer: JMP, JE, JNE, etc.
    //--------------------------------------------------------------------------
    uint32_t Instructions::handleJMP() {
        // Check if it's a short (EB) or near (E9) jump
        uint8_t opcode = memory.readByte(memory.calculatePhysicalAddress(registers.CS, registers.IP - 1));
        uint32_t cycleCount = 0;
        
        if (opcode == 0xEB) {
            // Short jump (8-bit displacement)
            int8_t disp8 = static_cast<int8_t>(fetchByte());
            registers.IP += disp8;
            cycleCount = cycles.JMP_SHORT;
        } else if (opcode == 0xE9) {
            // Near jump (16-bit displacement)
            int16_t disp16 = static_cast<int16_t>(fetchWord());
            registers.IP += disp16;
            cycleCount = cycles.JMP_NEAR;
        }
        
        return cycleCount;
    }

    uint32_t Instructions::handleJE() {
        int8_t offset = static_cast<int8_t>(fetchByte());
        // Jump if ZF=1 (equal)
        if (flags.getFlag(FLAGS::ZF)) {
            registers.IP += offset;
            return cycles.JCOND_TAKEN;
        }
        return cycles.JCOND_NOT_TAKEN;
    }

    uint32_t Instructions::handleJNE() {
        int16_t offset = static_cast<int16_t>(fetchWord());
        if(!flags.getFlag(FLAGS::ZF)) {
            registers.IP += offset;
            return cycles.JCOND_TAKEN;
        }
        return cycles.JCOND_NOT_TAKEN;
    }

    uint32_t Instructions::handleJG() {
        int16_t offset = static_cast<int16_t>(fetchWord());
        // JG => ZF=0 and SF=OF
        bool cond = (!flags.getFlag(FLAGS::ZF) && (flags.getFlag(FLAGS::SF) == flags.getFlag(FLAGS::OF)));
        if(cond) {
            registers.IP += offset;
            return cycles.JCOND_TAKEN;
        }
        return cycles.JCOND_NOT_TAKEN;
    }

    uint32_t Instructions::handleJGE() {
        int16_t offset = static_cast<int16_t>(fetchWord());
        // JGE => SF=OF
        if(flags.getFlag(FLAGS::SF) == flags.getFlag(FLAGS::OF)) {
            registers.IP += offset;
            return cycles.JCOND_TAKEN;
        }
        return cycles.JCOND_NOT_TAKEN;
    }

    uint32_t Instructions::handleJL() {
        int16_t offset = static_cast<int16_t>(fetchWord());
        // JL => SF!=OF
        if(flags.getFlag(FLAGS::SF) != flags.getFlag(FLAGS::OF)) {
            registers.IP += offset;
            return cycles.JCOND_TAKEN;
        }
        return cycles.JCOND_NOT_TAKEN;
    }

    uint32_t Instructions::handleJLE() {
        int16_t offset = static_cast<int16_t>(fetchWord());
        // JLE => ZF=1 or SF!=OF
        bool cond = (flags.getFlag(FLAGS::ZF) || (flags.getFlag(FLAGS::SF) != flags.getFlag(FLAGS::OF)));
        if(cond) {
            registers.IP += offset;
            return cycles.JCOND_TAKEN;
        }
        return cycles.JCOND_NOT_TAKEN;
    }

    //--------------------------------------------------------------------------
    // Stack & Procedure: PUSH, POP, CALL, RET
    //--------------------------------------------------------------------------
    uint32_t Instructions::handlePUSH() {
        // Example: 0x50 => PUSH AX, 0x51 => PUSH CX, etc.
        // Parse the last opcode, get which reg it is, then do SP -= 2, writeWord(SS:SP, reg).
        uint8_t lastOp = memory.readByte(memory.calculatePhysicalAddress(registers.CS, registers.IP - 1));
        uint8_t regCode = lastOp & 0x07;
        uint16_t* src = getRegisterReference(regCode);

        registers.SP -= 2;
        uint32_t phys = memory.calculatePhysicalAddress(registers.SS, registers.SP);
        memory.writeWord(phys, *src);
        
        return cycles.PUSH_REG;
    }

    uint32_t Instructions::handlePOP() {
        // 0x58 => POP AX, 0x59 => POP CX, etc.
        uint8_t lastOp = memory.readByte(memory.calculatePhysicalAddress(registers.CS, registers.IP - 1));
        uint8_t regCode = lastOp & 0x07;
        uint16_t* dest = getRegisterReference(regCode);

        uint32_t phys = memory.calculatePhysicalAddress(registers.SS, registers.SP);
        *dest = memory.readWord(phys);
        registers.SP += 2;
        
        return cycles.POP_REG;
    }

    uint32_t Instructions::handleCALL() {
        // 0xE8 => CALL rel16
        // push IP, then IP += offset
        int16_t offset = static_cast<int16_t>(fetchWord());

        // push current IP onto stack
        registers.SP -= 2;
        uint32_t phys = memory.calculatePhysicalAddress(registers.SS, registers.SP);
        memory.writeWord(phys, registers.IP);

        registers.IP += offset;
        
        return cycles.CALL_NEAR;
    }

    uint32_t Instructions::handleRET() {
        // 0xC3 => RET near
        uint32_t phys = memory.calculatePhysicalAddress(registers.SS, registers.SP);
        registers.IP = memory.readWord(phys);
        registers.SP += 2;
        
        return cycles.RET_NEAR;
    }

    //--------------------------------------------------------------------------
    // INT, HLT
    //--------------------------------------------------------------------------
    uint32_t Instructions::handleINT() {
        uint8_t intNum = fetchByte();
        
        // Calculate the address of the interrupt vector in the IVT
        // The IVT is located at physical address 0x0000:0x0000
        // Each interrupt vector is 4 bytes (2 for IP, 2 for CS)
        uint32_t ivtEntryAddress = intNum * 4;
        
        // Check if we're emulating certain interrupts directly
        bool emulatedInterrupt = false;
        
        // Handle specific interrupts directly (BIOS/DOS services)
        if (intNum == 0x10 || intNum == 0x16 || intNum == 0x21) {
            emulatedInterrupt = true;
            
            // Save state if using the real IVT mechanism later
            uint16_t oldFlags = 0;
            if (flags.getFlag(FLAGS::IF)) oldFlags |= FLAGS::IF;
            if (flags.getFlag(FLAGS::TF)) oldFlags |= FLAGS::TF;
            
            // Clear IF (interrupt) and TF (trap) flags as per 8086 behavior
            flags.setFlag(FLAGS::IF, false);
            flags.setFlag(FLAGS::TF, false);
            
            // Handle specific interrupts
            switch (intNum) {
                case 0x10: {  // BIOS Video Services
                    uint8_t ah = registers.AX.high;  // Function number
                    
                    switch (ah) {
                        case 0x0E: {  // Teletype output
                            char character = registers.AX.low;  // Character to print
                            std::cout << character;  // Print to console
                            break;
                        }
                        case 0x00: {  // Set video mode
                            std::cout << "INT 10h: Set video mode " << static_cast<int>(registers.AX.low) << std::endl;
                            break;
                        }
                        case 0x02: {  // Set cursor position
                            std::cout << "INT 10h: Set cursor position to row " << static_cast<int>(registers.DX.high)
                                    << ", col " << static_cast<int>(registers.DX.low) << std::endl;
                            break;
                        }
                        case 0x09: {  // Write character and attribute
                            char character = registers.AX.low;
                            std::cout << "INT 10h: Write character '" << character << "' with attribute "
                                    << static_cast<int>(registers.BX.low) << std::endl;
                            break;
                        }
                        case 0x13: {  // Write string
                            std::cout << "INT 10h: Write string (not fully implemented)" << std::endl;
                            break;
                        }
                        default:
                            std::cout << "INT 10h: Function " << static_cast<int>(ah) << " (not implemented)" << std::endl;
                            break;
                    }
                    break;
                }
                case 0x16: {  // Keyboard services
                    uint8_t ah = registers.AX.high;  // Function number
                    
                    switch (ah) {
                        case 0x00:  // Wait for keystroke and read
                            registers.AX.low = 'A';  // Dummy: simulate user pressed 'A'
                            break;
                        case 0x01:  // Check for keystroke
                            flags.setFlag(FLAGS::ZF, false);  // Indicate keypress available
                            registers.AX.low = 'A';  // Dummy: simulate user pressed 'A'
                            break;
                        default:
                            std::cout << "INT 16h: Function " << static_cast<int>(ah) << " (not implemented)" << std::endl;
                            break;
                    }
                    break;
                }
                case 0x21: {  // DOS services
                    uint8_t ah = registers.AX.high;  // Function number
                    
                    switch (ah) {
                        case 0x01:  // Character input with echo
                            registers.AX.low = 'A';  // Dummy: simulate user input 'A'
                            std::cout << "A";  // Echo character
                            break;
                        case 0x02:  // Character output
                            std::cout << static_cast<char>(registers.DX.low);
                            break;
                        case 0x09:  // Print string (terminated by '$')
                            {
                                uint32_t addr = memory.calculatePhysicalAddress(registers.DS, registers.DX.value);
                                char c;
                                while ((c = memory.readByte(addr++)) != '$') {
                                    std::cout << c;
                                }
                            }
                            break;
                        case 0x4C:  // Exit program
                            halted = true;
                            break;
                        default:
                            std::cout << "INT 21h: Function " << static_cast<int>(ah) << " (not implemented)" << std::endl;
                            break;
                    }
                    break;
                }
            }
        } else {
            // Use IVT for other interrupts
            // 1. Push flags
            registers.SP -= 2;
            uint32_t stackAddr = memory.calculatePhysicalAddress(registers.SS, registers.SP);
            uint16_t flagsValue = 0;
            // Set all flags bits
            for (int i = 0; i < 16; i++) {
                if (flags.getFlag(1 << i)) {
                    flagsValue |= (1 << i);
                }
            }
            memory.writeWord(stackAddr, flagsValue);
            
            // 2. Push CS (current code segment)
            registers.SP -= 2;
            stackAddr = memory.calculatePhysicalAddress(registers.SS, registers.SP);
            memory.writeWord(stackAddr, registers.CS);
            
            // 3. Push IP (return address)
            registers.SP -= 2;
            stackAddr = memory.calculatePhysicalAddress(registers.SS, registers.SP);
            memory.writeWord(stackAddr, registers.IP);
            
            // 4. Clear IF and TF flags
            flags.setFlag(FLAGS::IF, false);
            flags.setFlag(FLAGS::TF, false);
            
            // 5. Load CS:IP from IVT
            registers.IP = memory.readWord(ivtEntryAddress);
            registers.CS = memory.readWord(ivtEntryAddress + 2);
        }
        
        return cycles.INT;
    }

    uint32_t Instructions::handleHLT() {
        halted = true;
        return cycles.HLT;
    }

    //--------------------------------------------------------------------------
    // String operations
    //--------------------------------------------------------------------------
    uint32_t Instructions::handleMOVS() {
        // Get the opcode to determine if it's byte or word operation
        uint8_t lastOpcode = memory.readByte(memory.calculatePhysicalAddress(registers.CS, registers.IP - 1));
        bool isWord = (lastOpcode == 0xA5); // MOVSW = 0xA5, MOVSB = 0xA4
        
        // Calculate source and destination addresses
        uint32_t srcAddr = memory.calculatePhysicalAddress(registers.DS, registers.SI);
        uint32_t destAddr = memory.calculatePhysicalAddress(registers.ES, registers.DI);
        
        if (isWord) {
            // Word operation
            uint16_t value = memory.readWord(srcAddr);
            memory.writeWord(destAddr, value);
            
            // Update SI and DI based on direction flag
            if (flags.getFlag(FLAGS::DF)) {
                registers.SI -= 2;
                registers.DI -= 2;
            } else {
                registers.SI += 2;
                registers.DI += 2;
            }
        } else {
            // Byte operation
            uint8_t value = memory.readByte(srcAddr);
            memory.writeByte(destAddr, value);
            
            // Update SI and DI based on direction flag
            if (flags.getFlag(FLAGS::DF)) {
                registers.SI--;
                registers.DI--;
            } else {
                registers.SI++;
                registers.DI++;
            }
        }
        
        return 18; // MOVS takes about 18 cycles on 8086
    }

    uint32_t Instructions::handleCMPS() {
        // Get the opcode to determine if it's byte or word operation
        uint8_t lastOpcode = memory.readByte(memory.calculatePhysicalAddress(registers.CS, registers.IP - 1));
        bool isWord = (lastOpcode == 0xA7); // CMPSW = 0xA7, CMPSB = 0xA6
        
        // Calculate source and destination addresses
        uint32_t srcAddr = memory.calculatePhysicalAddress(registers.DS, registers.SI);
        uint32_t destAddr = memory.calculatePhysicalAddress(registers.ES, registers.DI);
        
        if (isWord) {
            // Word operation
            uint16_t src = memory.readWord(srcAddr);
            uint16_t dest = memory.readWord(destAddr);
            
            // Perform comparison and set flags
            uint32_t result = static_cast<uint32_t>(dest) - static_cast<uint32_t>(src);
            setArithmeticFlags(result, dest, src);
            
            // Update SI and DI based on direction flag
            if (flags.getFlag(FLAGS::DF)) {
                registers.SI -= 2;
                registers.DI -= 2;
            } else {
                registers.SI += 2;
                registers.DI += 2;
            }
        } else {
            // Byte operation
            uint8_t src = memory.readByte(srcAddr);
            uint8_t dest = memory.readByte(destAddr);
            
            // Perform comparison and set flags
            uint16_t result = static_cast<uint16_t>(dest) - static_cast<uint16_t>(src);
            setArithmeticFlags8(result, dest, src);
            
            // Update SI and DI based on direction flag
            if (flags.getFlag(FLAGS::DF)) {
                registers.SI--;
                registers.DI--;
            } else {
                registers.SI++;
                registers.DI++;
            }
        }
        
        return 22; // CMPS takes 22 cycles on 8086
    }

    uint32_t Instructions::handleSTOS() {
        // Get the opcode to determine if it's byte or word operation
        uint8_t lastOpcode = memory.readByte(memory.calculatePhysicalAddress(registers.CS, registers.IP - 1));
        bool isWord = (lastOpcode == 0xAB); // STOSW = 0xAB, STOSB = 0xAA
        
        // Calculate destination address (ES:DI)
        uint32_t destAddr = memory.calculatePhysicalAddress(registers.ES, registers.DI);
        
        if (isWord) {
            // Word operation - Store AX to ES:DI
            memory.writeWord(destAddr, registers.AX.value);
            
            // Update DI based on direction flag
            if (flags.getFlag(FLAGS::DF)) {
                registers.DI -= 2;
            } else {
                registers.DI += 2;
            }
        } else {
            // Byte operation - Store AL to ES:DI
            memory.writeByte(destAddr, registers.AX.low);
            
            // Update DI based on direction flag
            if (flags.getFlag(FLAGS::DF)) {
                registers.DI--;
            } else {
                registers.DI++;
            }
        }
        
        return 11; // STOS takes about 11 cycles on 8086
    }

    uint32_t Instructions::handleLODS() {
        // Get the opcode to determine if it's byte or word operation
        uint8_t lastOpcode = memory.readByte(memory.calculatePhysicalAddress(registers.CS, registers.IP - 1));
        bool isWord = (lastOpcode == 0xAD); // LODSW = 0xAD, LODSB = 0xAC
        
        // Calculate source address (DS:SI)
        uint32_t srcAddr = memory.calculatePhysicalAddress(registers.DS, registers.SI);
        
        if (isWord) {
            // Word operation - Load DS:SI into AX
            registers.AX.value = memory.readWord(srcAddr);
            
            // Update SI based on direction flag
            if (flags.getFlag(FLAGS::DF)) {
                registers.SI -= 2;
            } else {
                registers.SI += 2;
            }
        } else {
            // Byte operation - Load DS:SI into AL
            registers.AX.low = memory.readByte(srcAddr);
            
            // Update SI based on direction flag
            if (flags.getFlag(FLAGS::DF)) {
                registers.SI--;
            } else {
                registers.SI++;
            }
        }
        
        return 12; // LODS takes 12 cycles on 8086
    }

    uint32_t Instructions::handleSCAS() {
        // Get the opcode to determine if it's byte or word operation
        uint8_t lastOpcode = memory.readByte(memory.calculatePhysicalAddress(registers.CS, registers.IP - 1));
        bool isWord = (lastOpcode == 0xAF); // SCASW = 0xAF, SCASB = 0xAE
        
        // Calculate destination address (ES:DI)
        uint32_t destAddr = memory.calculatePhysicalAddress(registers.ES, registers.DI);
        
        if (isWord) {
            // Word operation - Compare AX with word at ES:DI
            uint16_t dest = memory.readWord(destAddr);
            uint16_t src = registers.AX.value;
            
            // Perform comparison and set flags
            uint32_t result = static_cast<uint32_t>(src) - static_cast<uint32_t>(dest);
            setArithmeticFlags(result, src, dest);
            
            // Update DI based on direction flag
            if (flags.getFlag(FLAGS::DF)) {
                registers.DI -= 2;
            } else {
                registers.DI += 2;
            }
        } else {
            // Byte operation - Compare AL with byte at ES:DI
            uint8_t dest = memory.readByte(destAddr);
            uint8_t src = registers.AX.low;
            
            // Perform comparison and set flags
            uint16_t result = static_cast<uint16_t>(src) - static_cast<uint16_t>(dest);
            setArithmeticFlags8(result, src, dest);
            
            // Update DI based on direction flag
            if (flags.getFlag(FLAGS::DF)) {
                registers.DI--;
            } else {
                registers.DI++;
            }
        }
        
        return 15; // SCAS takes 15 cycles on 8086
    }

    uint32_t Instructions::handleREP() {
        // Get the REP prefix opcode
        uint8_t prefixOpcode = memory.readByte(memory.calculatePhysicalAddress(registers.CS, registers.IP - 1));
        bool isREPZ = (prefixOpcode == 0xF3); // REPZ/REPE = 0xF3, REPNZ/REPNE = 0xF2
        
        // Fetch the string operation opcode
        uint8_t stringOpcode = fetchByte();
        
        // Save the current IP to return to the string operation
        uint16_t savedIP = registers.IP;
        
        uint32_t totalCycles = 2; // Initial REP prefix overhead
        
        // Execute the string operation until CX = 0
        while (registers.CX.value != 0) {
            // Reset IP to the string operation
            registers.IP = savedIP;
            
            // Execute the string operation and accumulate cycles
            totalCycles += decodeAndExecute(stringOpcode);
            
            // Decrement CX
            registers.CX.value--;
            
            // For REPZ/REPE, continue if ZF=1; for REPNZ/REPNE, continue if ZF=0
            if (isREPZ) {
                if (!flags.getFlag(FLAGS::ZF)) {
                    break;
                }
            } else {
                if (flags.getFlag(FLAGS::ZF)) {
                    break;
                }
            }
            
            // If CX is 0, we're done
            if (registers.CX.value == 0) {
                break;
            }
        }
        
        return totalCycles;
    }

    //--------------------------------------------------------------------------
    // I/O operations
    //--------------------------------------------------------------------------
    uint32_t Instructions::handleIN() {
        uint8_t opcode = memory.readByte(memory.calculatePhysicalAddress(registers.CS, registers.IP - 1));
        uint32_t cycles_count = 0;
        
        if (opcode == 0xE4) {
            // IN AL, imm8
            uint8_t port = fetchByte();
            registers.AX.low = io.readPort(port);
            cycles_count = 10; // Typical IN AL, port cycles
        }
        else if (opcode == 0xE5) {
            // IN AX, imm8
            uint8_t port = fetchByte();
            registers.AX.value = io.readPortWord(port);
            cycles_count = 14; // Typical IN AX, port cycles
        }
        else if (opcode == 0xEC) {
            // IN AL, DX
            registers.AX.low = io.readPort(registers.DX.value);
            cycles_count = 8; // Typical IN AL, DX cycles
        }
        else if (opcode == 0xED) {
            // IN AX, DX
            registers.AX.value = io.readPortWord(registers.DX.value);
            cycles_count = 12; // Typical IN AX, DX cycles
        }
        
        return cycles_count; // Return the cycle count
    }

    uint32_t Instructions::handleOUT() {
        // Get the opcode to determine the operation type
        uint8_t lastOpcode = memory.readByte(memory.calculatePhysicalAddress(registers.CS, registers.IP - 1));
        uint32_t cycleCount = 0;
        
        if (lastOpcode == 0xE6) {  // OUT imm8, AL
            // Read port address from the instruction stream
            uint8_t port = fetchByte();
            
            // Write AL to the I/O port
            io.writePort(port, registers.AX.low);
            cycleCount = 10; // Approximate cycles
        }
        else if (lastOpcode == 0xE7) {  // OUT imm8, AX
            // Read port address from the instruction stream
            uint8_t port = fetchByte();
            
            // Write AX to the I/O port
            io.writePortWord(port, registers.AX.value);
            cycleCount = 14; // Approximate cycles
        }
        else if (lastOpcode == 0xEE) {  // OUT DX, AL
            // Port address is in DX register
            uint16_t port = registers.DX.value;
            
            // Write AL to the I/O port
            io.writePort(port, registers.AX.low);
            cycleCount = 8; // Approximate cycles
        }
        else if (lastOpcode == 0xEF) {  // OUT DX, AX
            // Port address is in DX register
            uint16_t port = registers.DX.value;
            
            // Write AX to the I/O port
            io.writePortWord(port, registers.AX.value);
            cycleCount = 12; // Approximate cycles
        }
        
        return cycleCount;
    }

    uint32_t Instructions::handleADD8() {
        uint8_t modrm = fetchByte();
        uint8_t mod = (modrm >> 6) & 0x03;
        uint8_t reg = (modrm >> 3) & 0x07;
        uint8_t rm = modrm & 0x07;
        
        uint8_t lastOpcode = memory.readByte(memory.calculatePhysicalAddress(registers.CS, registers.IP - 2));
        bool direction = (lastOpcode & 0x02) != 0; // 0x00 or 0x02
        uint32_t cycleCount = 0;
        
        if (direction) {
            // ADD r8, r/m8 (0x02)
            uint8_t* destReg = get8BitRegisterRef(reg);
            
            if (mod == 0b11) {
                // Register to register
                uint8_t* srcReg = get8BitRegisterRef(rm);
                uint16_t result = *destReg + *srcReg;
                setArithmeticFlags8(result, *destReg, *srcReg);
                *destReg = static_cast<uint8_t>(result);
                cycleCount = cycles.ALU_REG_REG;
            } else {
                // Memory to register
                uint32_t addr = getEffectiveAddress(mod, rm);
                uint8_t value = memory.readByte(addr);
                uint16_t result = *destReg + value;
                setArithmeticFlags8(result, *destReg, value);
                *destReg = static_cast<uint8_t>(result);
                cycleCount = cycles.ALU_MEM_REG;
            }
        } else {
            // ADD r/m8, r8 (0x00)
            uint8_t* srcReg = get8BitRegisterRef(reg);
            
            if (mod == 0b11) {
                // Register to register
                uint8_t* destReg = get8BitRegisterRef(rm);
                uint16_t result = *destReg + *srcReg;
                setArithmeticFlags8(result, *destReg, *srcReg);
                *destReg = static_cast<uint8_t>(result);
                cycleCount = cycles.ALU_REG_REG;
            } else {
                // Register to memory
                uint32_t addr = getEffectiveAddress(mod, rm);
                uint8_t value = memory.readByte(addr);
                uint16_t result = value + *srcReg;
                setArithmeticFlags8(result, value, *srcReg);
                memory.writeByte(addr, static_cast<uint8_t>(result));
                cycleCount = cycles.ALU_REG_MEM;
            }
        }
        
        return cycleCount;
    }

    uint32_t Instructions::handleADDImm8() {
        // ADD AL, imm8 (0x04)
        uint8_t imm8 = fetchByte();
        uint8_t al = registers.AX.low;
        uint16_t result = al + imm8;
        
        setArithmeticFlags8(result, al, imm8);
        registers.AX.low = static_cast<uint8_t>(result);
        
        return cycles.ALU_IMM_REG;
    }

    uint32_t Instructions::handleADDImm16() {
        // ADD AX, imm16 (0x05)
        uint16_t imm16 = fetchWord();
        uint16_t ax = registers.AX.value;
        uint32_t result = ax + imm16;
        
        setArithmeticFlags(result, ax, imm16);
        registers.AX.value = static_cast<uint16_t>(result);
        
        return cycles.ALU_IMM_REG;
    }

    uint32_t Instructions::handleADC8() {
        uint8_t opcode = memory.readByte(memory.calculatePhysicalAddress(registers.CS, registers.IP - 1));
        uint32_t cycleCount = 0;
        
        if (opcode == 0x10) {
            // ADC r/m8, r8
            uint8_t modrm = fetchByte();
            uint8_t mod = (modrm >> 6) & 0x03;
            uint8_t reg = (modrm >> 3) & 0x07;
            uint8_t rm = modrm & 0x07;
            
            uint8_t* dest;
            uint8_t src = *get8BitRegisterRef(reg);
            bool carry = flags.getFlag(FLAGS::CF);
            
            if (mod == 0b11) {
                // Register destination
                dest = get8BitRegisterRef(rm);
                uint16_t result = *dest + src + carry;
                setArithmeticFlags8(result, *dest, src);
                *dest = result & 0xFF;
                cycleCount = cycles.ALU_REG_REG;
            } else {
                // Memory destination
                uint32_t addr = getEffectiveAddress(mod, rm);
                uint8_t destValue = memory.readByte(addr);
                uint16_t result = destValue + src + carry;
                setArithmeticFlags8(result, destValue, src);
                memory.writeByte(addr, result & 0xFF);
                cycleCount = cycles.ALU_REG_MEM;
            }
        } else if (opcode == 0x12) {
            // ADC r8, r/m8
            uint8_t modrm = fetchByte();
            uint8_t mod = (modrm >> 6) & 0x03;
            uint8_t reg = (modrm >> 3) & 0x07;
            uint8_t rm = modrm & 0x07;
            
            uint8_t* dest = get8BitRegisterRef(reg);
            uint8_t src;
            bool carry = flags.getFlag(FLAGS::CF);
            
            if (mod == 0b11) {
                // Register source
                src = *get8BitRegisterRef(rm);
                cycleCount = cycles.ALU_REG_REG;
            } else {
                // Memory source
                uint32_t addr = getEffectiveAddress(mod, rm);
                src = memory.readByte(addr);
                cycleCount = cycles.ALU_MEM_REG;
            }
            
            uint16_t result = *dest + src + carry;
            setArithmeticFlags8(result, *dest, src);
            *dest = result & 0xFF;
        }
        
        return cycleCount; // Return the cycle count
    }

    uint32_t Instructions::handleSBB8() {
        uint8_t modrm = fetchByte();
        uint8_t mod = (modrm >> 6) & 0x03;
        uint8_t reg = (modrm >> 3) & 0x07;
        uint8_t rm = modrm & 0x07;
        
        uint8_t lastOpcode = memory.readByte(memory.calculatePhysicalAddress(registers.CS, registers.IP - 2));
        bool direction = (lastOpcode & 0x02) != 0; // 0x18 or 0x1A
        uint32_t cycleCount = 0;
        
        if (direction) {
            // SBB r8, r/m8 (0x1A)
            uint8_t* destReg = get8BitRegisterRef(reg);
            
            if (mod == 0b11) {
                // Register to register
                uint8_t* srcReg = get8BitRegisterRef(rm);
                uint16_t result = *destReg - *srcReg - static_cast<uint8_t>(flags.getFlag(FLAGS::CF));
                setArithmeticFlags8(result, *destReg, *srcReg);
                *destReg = static_cast<uint8_t>(result);
                cycleCount = cycles.ALU_REG_REG;
            } else {
                // Memory to register
                uint32_t addr = getEffectiveAddress(mod, rm);
                uint8_t value = memory.readByte(addr);
                uint16_t result = *destReg - value - static_cast<uint8_t>(flags.getFlag(FLAGS::CF));
                setArithmeticFlags8(result, *destReg, value);
                *destReg = static_cast<uint8_t>(result);
                cycleCount = cycles.ALU_MEM_REG;
            }
        } else {
            // SBB r/m8, r8 (0x18)
            uint8_t* srcReg = get8BitRegisterRef(reg);
            
            if (mod == 0b11) {
                // Register to register
                uint8_t* destReg = get8BitRegisterRef(rm);
                uint16_t result = *destReg - *srcReg - static_cast<uint8_t>(flags.getFlag(FLAGS::CF));
                setArithmeticFlags8(result, *destReg, *srcReg);
                *destReg = static_cast<uint8_t>(result);
                cycleCount = cycles.ALU_REG_REG;
            } else {
                // Register to memory
                uint32_t addr = getEffectiveAddress(mod, rm);
                uint8_t value = memory.readByte(addr);
                uint16_t result = value - *srcReg - static_cast<uint8_t>(flags.getFlag(FLAGS::CF));
                setArithmeticFlags8(result, value, *srcReg);
                memory.writeByte(addr, static_cast<uint8_t>(result));
                cycleCount = cycles.ALU_REG_MEM;
            }
        }
        
        return cycleCount;
    }

    uint32_t Instructions::handleCLC() {
        flags.setFlag(FLAGS::CF, false);
        return cycles.FLAG_OP;
    }

    uint32_t Instructions::handleSTC() {
        flags.setFlag(FLAGS::CF, true);
        return cycles.FLAG_OP;
    }

    uint32_t Instructions::handleCMC() {
        flags.setFlag(FLAGS::CF, !flags.getFlag(FLAGS::CF));
        return cycles.FLAG_OP;
    }

    uint32_t Instructions::handleCLD() {
        flags.setFlag(FLAGS::DF, false);
        return cycles.FLAG_OP;
    }

    uint32_t Instructions::handleSTD() {
        flags.setFlag(FLAGS::DF, true);
        return cycles.FLAG_OP;
    }

    uint32_t Instructions::handleCLI() {
        flags.setFlag(FLAGS::IF, false);
        return cycles.FLAG_OP;
    }

    uint32_t Instructions::handleSTI() {
        flags.setFlag(FLAGS::IF, true);
        return cycles.FLAG_OP;
    }

    uint32_t Instructions::handleADC() {
        uint8_t modrm = fetchByte();
        uint8_t mod = (modrm >> 6) & 0x03;
        uint8_t reg = (modrm >> 3) & 0x07;
        uint8_t rm = modrm & 0x07;
        
        uint8_t lastOpcode = memory.readByte(memory.calculatePhysicalAddress(registers.CS, registers.IP - 2));
        bool direction = (lastOpcode & 0x02) != 0; // 0x11 or 0x13
        uint32_t cycleCount = 0;
        
        if (direction) {
            // ADC r16, r/m16 (0x13)
            uint16_t* destReg = getRegisterReference(reg);
            
            if (mod == 0b11) {
                // Register to register
                uint16_t* srcReg = getRegisterReference(rm);
                uint32_t result = *destReg + *srcReg + static_cast<uint16_t>(flags.getFlag(FLAGS::CF));
                setArithmeticFlags(result, *destReg, *srcReg);
                *destReg = static_cast<uint16_t>(result);
                cycleCount = cycles.ALU_REG_REG;
            } else {
                // Memory to register
                uint32_t addr = getEffectiveAddress(mod, rm);
                uint16_t value = memory.readWord(addr);
                uint32_t result = *destReg + value + static_cast<uint16_t>(flags.getFlag(FLAGS::CF));
                setArithmeticFlags(result, *destReg, value);
                *destReg = static_cast<uint16_t>(result);
                cycleCount = cycles.ALU_MEM_REG;
            }
        } else {
            // ADC r/m16, r16 (0x11)
            uint16_t* srcReg = getRegisterReference(reg);
            
            if (mod == 0b11) {
                // Register to register
                uint16_t* destReg = getRegisterReference(rm);
                uint32_t result = *destReg + *srcReg + static_cast<uint16_t>(flags.getFlag(FLAGS::CF));
                setArithmeticFlags(result, *destReg, *srcReg);
                *destReg = static_cast<uint16_t>(result);
                cycleCount = cycles.ALU_REG_REG;
            } else {
                // Register to memory
                uint32_t addr = getEffectiveAddress(mod, rm);
                uint16_t value = memory.readWord(addr);
                uint32_t result = value + *srcReg + static_cast<uint16_t>(flags.getFlag(FLAGS::CF));
                setArithmeticFlags(result, value, *srcReg);
                memory.writeWord(addr, static_cast<uint16_t>(result));
                cycleCount = cycles.ALU_REG_MEM;
            }
        }
        
        return cycleCount;
    }

    uint32_t Instructions::handleSBB() {
        uint8_t modrm = fetchByte();
        uint8_t mod = (modrm >> 6) & 0x03;
        uint8_t reg = (modrm >> 3) & 0x07;
        uint8_t rm = modrm & 0x07;
        
        uint8_t lastOpcode = memory.readByte(memory.calculatePhysicalAddress(registers.CS, registers.IP - 2));
        bool direction = (lastOpcode & 0x02) != 0; // 0x19 or 0x1B
        uint32_t cycleCount = 0;
        
        if (direction) {
            // SBB r16, r/m16 (0x1B)
            uint16_t* destReg = getRegisterReference(reg);
            
            if (mod == 0b11) {
                // Register to register
                uint16_t* srcReg = getRegisterReference(rm);
                uint32_t result = *destReg - *srcReg - static_cast<uint16_t>(flags.getFlag(FLAGS::CF));
                setArithmeticFlags(result, *destReg, *srcReg);
                *destReg = static_cast<uint16_t>(result);
                cycleCount = cycles.ALU_REG_REG;
            } else {
                // Memory to register
                uint32_t addr = getEffectiveAddress(mod, rm);
                uint16_t value = memory.readWord(addr);
                uint32_t result = *destReg - value - static_cast<uint16_t>(flags.getFlag(FLAGS::CF));
                setArithmeticFlags(result, *destReg, value);
                *destReg = static_cast<uint16_t>(result);
                cycleCount = cycles.ALU_MEM_REG;
            }
        } else {
            // SBB r/m16, r16 (0x19)
            uint16_t* srcReg = getRegisterReference(reg);
            
            if (mod == 0b11) {
                // Register to register
                uint16_t* destReg = getRegisterReference(rm);
                uint32_t result = *destReg - *srcReg - static_cast<uint16_t>(flags.getFlag(FLAGS::CF));
                setArithmeticFlags(result, *destReg, *srcReg);
                *destReg = static_cast<uint16_t>(result);
                cycleCount = cycles.ALU_REG_REG;
            } else {
                // Register to memory
                uint32_t addr = getEffectiveAddress(mod, rm);
                uint16_t value = memory.readWord(addr);
                uint32_t result = value - *srcReg - static_cast<uint16_t>(flags.getFlag(FLAGS::CF));
                setArithmeticFlags(result, value, *srcReg);
                memory.writeWord(addr, static_cast<uint16_t>(result));
                cycleCount = cycles.ALU_REG_MEM;
            }
        }
        
        return cycleCount;
    }

    uint32_t Instructions::handleROL() {
        uint8_t modrm = fetchByte();
        uint8_t mod = (modrm >> 6) & 0x03;
        uint8_t op = (modrm >> 3) & 0x07;
        uint8_t rm = modrm & 0x07;
        
        // Get the last opcode to determine operation size and count
        uint8_t lastOp = memory.readByte(memory.calculatePhysicalAddress(registers.CS, registers.IP - 2));
        bool is16Bit = (lastOp == 0xD1 || lastOp == 0xD3);
        bool useCount = (lastOp == 0xD2 || lastOp == 0xD3);
        
        // Get shift/rotate count
        uint8_t count = 1;
        if (useCount) {
            count = registers.CX.low;  // Count in CL register
        }
        
        // Early return if count is 0
        if (count == 0) {
            return useCount ? cycles.SHIFT_REG_CL : cycles.SHIFT_REG_1;
        }
        
        uint32_t cycleCount = 0;
        
        // Process according to op code (ROL, ROR, RCL, RCR, SHL/SAL, SHR, SAR)
        switch (op) {
            case 0: // ROL - Rotate Left
                if (is16Bit) {
                    cycleCount = handleROL16(modrm, count, mod, rm);
                } else {
                    cycleCount = handleROL8(modrm, count, mod, rm);
                }
                break;
                
            case 1: // ROR - Rotate Right
                if (is16Bit) {
                    cycleCount = handleROR16(modrm, count, mod, rm);
                } else {
                    cycleCount = handleROR8(modrm, count, mod, rm);
                }
                break;
                
            case 2: // RCL - Rotate through Carry Left
                if (is16Bit) {
                    cycleCount = handleRCL16(modrm, count, mod, rm);
                } else {
                    cycleCount = handleRCL8(modrm, count, mod, rm);
                }
                break;
                
            case 3: // RCR - Rotate through Carry Right
                if (is16Bit) {
                    cycleCount = handleRCR16(modrm, count, mod, rm);
                } else {
                    cycleCount = handleRCR8(modrm, count, mod, rm);
                }
                break;
                
            case 4: // SHL/SAL - Shift Left
                if (is16Bit) {
                    cycleCount = handleSAL16(modrm, count, mod, rm);
                } else {
                    cycleCount = handleSAL8(modrm, count, mod, rm);
                }
                break;
                
            case 5: // SHR - Shift Right (logical)
                if (is16Bit) {
                    cycleCount = handleSHR16(modrm, count, mod, rm);
                } else {
                    cycleCount = handleSHR8(modrm, count, mod, rm);
                }
                break;
                
            case 7: // SAR - Shift Arithmetic Right
                if (is16Bit) {
                    cycleCount = handleSAR16(modrm, count, mod, rm);
                } else {
                    cycleCount = handleSAR8(modrm, count, mod, rm);
                }
                break;
                
            default:
                throw std::runtime_error("Unknown shift/rotate operation: " + std::to_string(op));
        }
        
        return cycleCount;
    }

    uint32_t Instructions::handleROL8(uint8_t modrm, uint8_t count, uint8_t mod, uint8_t rm) {
        uint8_t value;
        uint32_t addr = 0;
        uint32_t cycleCount = 0;
        
        // Get the operand
        if (mod == 0b11) {
            // Register operand
            uint8_t* reg = get8BitRegisterRef(rm);
            value = *reg;
            cycleCount = cycles.SHIFT_REG_1 + (count > 1 ? (count-1) * 4 : 0);
        } else {
            // Memory operand
            addr = getEffectiveAddress(mod, rm);
            value = memory.readByte(addr);
            cycleCount = cycles.SHIFT_MEM_1 + (count > 1 ? (count-1) * 4 : 0);
        }
        
        // Normalize count for 8-bit (count % 8)
        count &= 0x07;
        if (count == 0) return cycleCount;
        
        // Rotate Left
        uint8_t result = (value << count) | (value >> (8 - count));
        
        // Update CF with the last bit shifted out
        flags.setFlag(FLAGS::CF, (result & 0x01) != 0);
        
        // Update OF for count==1 only (as per 8086 spec)
        if (count == 1) {
            // OF = XOR of the two most significant bits
            flags.setFlag(FLAGS::OF, ((result & 0x80) >> 7) ^ ((result & 0x01)));
        }
        
        // Store the result
        if (mod == 0b11) {
            *get8BitRegisterRef(rm) = result;
        } else {
            memory.writeByte(addr, result);
        }
        
        return cycleCount;
    }

    uint32_t Instructions::handleROL16(uint8_t modrm, uint8_t count, uint8_t mod, uint8_t rm) {
        uint16_t value;
        uint32_t addr = 0;
        uint32_t cycleCount = 0;
        
        // Get the operand
        if (mod == 0b11) {
            // Register operand
            uint16_t* reg = getRegisterReference(rm);
            value = *reg;
            cycleCount = cycles.SHIFT_REG_1 + (count > 1 ? (count-1) * 4 : 0);
        } else {
            // Memory operand
            addr = getEffectiveAddress(mod, rm);
            value = memory.readWord(addr);
            cycleCount = cycles.SHIFT_MEM_1 + (count > 1 ? (count-1) * 4 : 0);
        }
        
        // Normalize count for 16-bit (count % 16)
        count &= 0x0F;
        if (count == 0) return cycleCount;
        
        // Rotate Left
        uint16_t result = (value << count) | (value >> (16 - count));
        
        // Update CF with the last bit shifted out
        flags.setFlag(FLAGS::CF, (result & 0x01) != 0);
        
        // Update OF for count==1 only
        if (count == 1) {
            // OF = XOR of the two most significant bits
            flags.setFlag(FLAGS::OF, ((result & 0x8000) >> 15) ^ ((result & 0x01)));
        }
        
        // Store the result
        if (mod == 0b11) {
            *getRegisterReference(rm) = result;
        } else {
            memory.writeWord(addr, result);
        }
        
        return cycleCount;
    }

    uint32_t Instructions::handleROR8(uint8_t modrm, uint8_t count, uint8_t mod, uint8_t rm) {
        uint8_t value;
        uint32_t addr = 0;
        uint32_t cycleCount = 0;
        
        // Get the operand
        if (mod == 0b11) {
            // Register operand
            uint8_t* reg = get8BitRegisterRef(rm);
            value = *reg;
            cycleCount = cycles.SHIFT_REG_1 + (count > 1 ? (count-1) * 4 : 0);
        } else {
            // Memory operand
            addr = getEffectiveAddress(mod, rm);
            value = memory.readByte(addr);
            cycleCount = cycles.SHIFT_MEM_1 + (count > 1 ? (count-1) * 4 : 0);
        }
        
        // Normalize count
        count &= 0x07;
        if (count == 0) return cycleCount;
        
        // Rotate Right
        uint8_t result = (value >> count) | (value << (8 - count));
        
        // Update CF with the last bit shifted out
        flags.setFlag(FLAGS::CF, (result & 0x80) != 0);
        
        // Update OF for count==1 only
        if (count == 1) {
            // OF = XOR of the two most significant bits
            flags.setFlag(FLAGS::OF, ((result & 0x80) >> 7) ^ ((result & 0x40) >> 6));
        }
        
        // Store the result
        if (mod == 0b11) {
            *get8BitRegisterRef(rm) = result;
        } else {
            memory.writeByte(addr, result);
        }
        
        return cycleCount;
    }

    uint32_t Instructions::handleROR16(uint8_t modrm, uint8_t count, uint8_t mod, uint8_t rm) {
        uint16_t value;
        uint32_t addr = 0;
        uint32_t cycleCount = 0;
        
        // Get the operand
        if (mod == 0b11) {
            // Register operand
            uint16_t* reg = getRegisterReference(rm);
            value = *reg;
            cycleCount = cycles.SHIFT_REG_1 + (count > 1 ? (count-1) * 4 : 0);
        } else {
            // Memory operand
            addr = getEffectiveAddress(mod, rm);
            value = memory.readWord(addr);
            cycleCount = cycles.SHIFT_MEM_1 + (count > 1 ? (count-1) * 4 : 0);
        }
        
        // Normalize count
        count &= 0x0F;
        if (count == 0) return cycleCount;
        
        // Rotate Right
        uint16_t result = (value >> count) | (value << (16 - count));
        
        // Update CF with the last bit shifted out
        flags.setFlag(FLAGS::CF, (result & 0x8000) != 0);
        
        // Update OF for count==1 only
        if (count == 1) {
            // OF = XOR of the two most significant bits
            flags.setFlag(FLAGS::OF, ((result & 0x8000) >> 15) ^ ((result & 0x4000) >> 14));
        }
        
        // Store the result
        if (mod == 0b11) {
            *getRegisterReference(rm) = result;
        } else {
            memory.writeWord(addr, result);
        }
        
        return cycleCount;
    }

    uint32_t Instructions::handleRCL8(uint8_t modrm, uint8_t count, uint8_t mod, uint8_t rm) {
        uint8_t value;
        uint32_t addr = 0;
        uint32_t cycleCount = 0;
        
        // Get the operand
        if (mod == 0b11) {
            // Register operand
            uint8_t* reg = get8BitRegisterRef(rm);
            value = *reg;
            cycleCount = cycles.SHIFT_REG_1 + (count > 1 ? (count-1) * 4 : 0);
        } else {
            // Memory operand
            addr = getEffectiveAddress(mod, rm);
            value = memory.readByte(addr);
            cycleCount = cycles.SHIFT_MEM_1 + (count > 1 ? (count-1) * 4 : 0);
        }
        
        // Normalize count (9 bits total rotation: 8 bits + CF)
        count %= 9;
        if (count == 0) return cycleCount;
        
        // Get CF
        bool carryFlag = flags.getFlag(FLAGS::CF);
        
        // Perform rotation through carry
        uint16_t temp = (static_cast<uint16_t>(value) << 1) | (carryFlag ? 1 : 0);
        for (uint8_t i = 0; i < count; i++) {
            // Save MSB in CF
            bool newCarry = (temp & 0x100) != 0;
            
            // Rotate left with carry
            temp = ((temp << 1) & 0x1FF) | (newCarry ? 1 : 0);
        }
        
        // Get the result (bottom 8 bits)
        uint8_t result = temp & 0xFF;
        
        // Update CF with the bit rotated out
        flags.setFlag(FLAGS::CF, (temp & 0x100) != 0);
        
        // Update OF for count==1 only
        if (count == 1) {
            // OF = XOR of the two most significant bits
            flags.setFlag(FLAGS::OF, ((result & 0x80) >> 7) ^ flags.getFlag(FLAGS::CF));
        }
        
        // Store the result
        if (mod == 0b11) {
            *get8BitRegisterRef(rm) = result;
        } else {
            memory.writeByte(addr, result);
        }
        
        return cycleCount;
    }

    uint32_t Instructions::handleRCL16(uint8_t modrm, uint8_t count, uint8_t mod, uint8_t rm) {
        uint16_t value;
        uint32_t addr = 0;
        uint32_t cycleCount = 0;
        
        // Get the operand
        if (mod == 0b11) {
            // Register operand
            uint16_t* reg = getRegisterReference(rm);
            value = *reg;
            cycleCount = cycles.SHIFT_REG_1 + (count > 1 ? (count-1) * 4 : 0);
        } else {
            // Memory operand
            addr = getEffectiveAddress(mod, rm);
            value = memory.readWord(addr);
            cycleCount = cycles.SHIFT_MEM_1 + (count > 1 ? (count-1) * 4 : 0);
        }
        
        // Normalize count (17 bits total rotation: 16 bits + CF)
        count %= 17;
        if (count == 0) return cycleCount;
        
        // Get CF
        bool carryFlag = flags.getFlag(FLAGS::CF);
        
        // Perform rotation through carry
        uint32_t temp = (static_cast<uint32_t>(value) << 1) | (carryFlag ? 1 : 0);
        for (uint8_t i = 0; i < count; i++) {
            // Save MSB in CF
            bool newCarry = (temp & 0x10000) != 0;
            
            // Rotate left with carry
            temp = ((temp << 1) & 0x1FFFF) | (newCarry ? 1 : 0);
        }
        
        // Get the result (bottom 16 bits)
        uint16_t result = temp & 0xFFFF;
        
        // Update CF with the bit rotated out
        flags.setFlag(FLAGS::CF, (temp & 0x10000) != 0);
        
        // Update OF for count==1 only
        if (count == 1) {
            // OF = XOR of the two most significant bits
            flags.setFlag(FLAGS::OF, ((result & 0x8000) >> 15) ^ flags.getFlag(FLAGS::CF));
        }
        
        // Store the result
        if (mod == 0b11) {
            *getRegisterReference(rm) = result;
        } else {
            memory.writeWord(addr, result);
        }
        
        return cycleCount;
    }

    uint32_t Instructions::handleRCR8(uint8_t modrm, uint8_t count, uint8_t mod, uint8_t rm) {
        uint8_t value;
        uint32_t addr = 0;
        uint32_t cycleCount = 0;
        
        // Get the operand
        if (mod == 0b11) {
            // Register operand
            uint8_t* reg = get8BitRegisterRef(rm);
            value = *reg;
            cycleCount = cycles.SHIFT_REG_1 + (count > 1 ? (count-1) * 4 : 0);
        } else {
            // Memory operand
            addr = getEffectiveAddress(mod, rm);
            value = memory.readByte(addr);
            cycleCount = cycles.SHIFT_MEM_1 + (count > 1 ? (count-1) * 4 : 0);
        }
        
        // Normalize count (9 bits total rotation: 8 bits + CF)
        count %= 9;
        if (count == 0) return cycleCount;
        
        // Create a 9-bit rotate buffer with original value and CF
        uint16_t buffer = value;
        if (flags.getFlag(FLAGS::CF)) {
            buffer |= 0x100;  // Set the 9th bit (CF) if CF is set
        }
        
        // Store original MSB for OF calculation
        bool originalMsb = (value & 0x80) != 0;
        
        // Perform rotation
        for (uint8_t i = 0; i < count; i++) {
            // Get the lowest bit for CF
            bool newCF = (buffer & 0x01) != 0;
            
            // Rotate right
            buffer = (buffer >> 1) | (newCF ? 0x100 : 0);
        }
        
        // Update CF and result
        flags.setFlag(FLAGS::CF, (buffer & 0x100) != 0);
        uint8_t result = buffer & 0xFF;
        
        // Update OF for count==1 only
        if (count == 1) {
            // OF = MSB ^ MSB-1
            flags.setFlag(FLAGS::OF, originalMsb != ((result & 0x80) != 0));
        }
        
        // Store the result
        if (mod == 0b11) {
            *get8BitRegisterRef(rm) = result;
        } else {
            memory.writeByte(addr, result);
        }
        
        return cycleCount;
    }

    uint32_t Instructions::handleRCR16(uint8_t modrm, uint8_t count, uint8_t mod, uint8_t rm) {
        uint16_t value;
        uint32_t addr = 0;
        bool oldCF = flags.getFlag(FLAGS::CF);
        
        // Get the operand
        if (mod == 0b11) {
            // Register operand
            uint16_t* reg = getRegisterReference(rm);
            value = *reg;
        } else {
            // Memory operand
            addr = getEffectiveAddress(mod, rm);
            value = memory.readWord(addr);
        }
        
        // Normalize count (for RCR, each rotation is modulo 17 on 8086)
        if (count > 0) count = count % 17;
        if (count == 0) return (mod == 0b11) ? cycles.SHIFT_REG_CL : cycles.SHIFT_MEM_CL;
        
        // Perform rotation through carry by specified count
        uint16_t result = value;
        bool carryOut;
        bool oldMSB = (result & 0x8000) != 0;
        
        for (uint8_t i = 0; i < count; i++) {
            carryOut = (result & 0x01) != 0;  // Will become new CF
            result = (result >> 1) | (oldCF ? 0x8000 : 0);  // Right shift with oldCF as new MSB
            oldCF = carryOut;  // Update CF for next iteration
        }
        
        // Set flags
        flags.setFlag(FLAGS::CF, carryOut);
        
        // For 8086, OF is only defined for count=1, and is set if MSB changes
        if (count == 1) {
            bool newMSB = (result & 0x8000) != 0;
            flags.setFlag(FLAGS::OF, oldMSB != newMSB);
        }
        
        // Store result
        if (mod == 0b11) {
            *getRegisterReference(rm) = result;
        } else {
            memory.writeWord(addr, result);
        }
        
        return (mod == 0b11) ? cycles.SHIFT_REG_CL : cycles.SHIFT_MEM_CL;
    }

    uint32_t Instructions::handleSAR8(uint8_t modrm, uint8_t count, uint8_t mod, uint8_t rm) {
        uint8_t value;
        uint32_t addr = 0;
        
        // Get the operand
        if (mod == 0b11) {
            // Register operand
            uint8_t* reg = get8BitRegisterRef(rm);
            value = *reg;
        } else {
            // Memory operand
            addr = getEffectiveAddress(mod, rm);
            value = memory.readByte(addr);
        }
        
        // Skip operation if count is 0
        if (count == 0) return (mod == 0b11) ? cycles.SHIFT_REG_CL : cycles.SHIFT_MEM_CL;
        
        // Store MSB for sign extension and OF
        bool signBit = (value & 0x80) != 0;
        
        // For counts > 8, result becomes either 0 or -1 (all 1s)
        if (count > 8) {
            count = 8;
        }
        
        // Perform arithmetic shift right (preserves sign bit)
        uint8_t result = value;
        bool carryOut = false;
        
        for (uint8_t i = 0; i < count; i++) {
            carryOut = (result & 0x01) != 0;  // LSB goes to carry
            result = (result >> 1) | (signBit ? 0x80 : 0);  // Preserve sign bit
        }
        
        // Set flags
        flags.setFlag(FLAGS::CF, carryOut);
        
        // OF is only defined for count=1, set if MSB changes (which it can't for SAR - always 0)
        if (count == 1) {
            flags.setFlag(FLAGS::OF, false);  // MSB doesn't change for SAR
        }
        
        // ZF, SF, PF as usual
        flags.setFlag(FLAGS::ZF, result == 0);
        flags.setFlag(FLAGS::SF, (result & 0x80) != 0);
        flags.setFlag(FLAGS::PF, utils.calculateParity(result));
        
        // Store the result
        if (mod == 0b11) {
            *get8BitRegisterRef(rm) = result;
        } else {
            memory.writeByte(addr, result);
        }
        
        return (mod == 0b11) ? cycles.SHIFT_REG_CL : cycles.SHIFT_MEM_CL;
    }
    

    uint32_t Instructions::handleSAR16(uint8_t modrm, uint8_t count, uint8_t mod, uint8_t rm) {
        uint16_t value;
        uint32_t addr = 0;
        
        // Get the operand
        if (mod == 0b11) {
            // Register operand
            uint16_t* reg = getRegisterReference(rm);
            value = *reg;
        } else {
            // Memory operand
            addr = getEffectiveAddress(mod, rm);
            value = memory.readWord(addr);
        }
        
        // Skip operation if count is 0
        if (count == 0) return (mod == 0b11) ? cycles.SHIFT_REG_CL : cycles.SHIFT_MEM_CL;
        
        // Store MSB for sign extension and OF
        bool signBit = (value & 0x8000) != 0;
        
        // For counts > 16, result becomes either 0 or -1 (all 1s)
        if (count > 16) {
            count = 16;
        }
        
        // Perform arithmetic shift right (preserves sign bit)
        uint16_t result = value;
        bool carryOut = false;
        
        for (uint8_t i = 0; i < count; i++) {
            carryOut = (result & 0x0001) != 0;  // LSB goes to carry
            result = (result >> 1) | (signBit ? 0x8000 : 0);  // Preserve sign bit
        }
        
        // Set flags
        flags.setFlag(FLAGS::CF, carryOut);
        
        // OF is only defined for count=1, set if MSB changes (which it can't for SAR - always 0)
        if (count == 1) {
            flags.setFlag(FLAGS::OF, false);  // MSB doesn't change for SAR
        }
        
        // ZF, SF, PF as usual
        flags.setFlag(FLAGS::ZF, result == 0);
        flags.setFlag(FLAGS::SF, (result & 0x8000) != 0);
        flags.setFlag(FLAGS::PF, utils.calculateParity(result));
        
        // Store the result
        if (mod == 0b11) {
            *getRegisterReference(rm) = result;
        } else {
            memory.writeWord(addr, result);
        }
        
        return (mod == 0b11) ? cycles.SHIFT_REG_CL : cycles.SHIFT_MEM_CL;
    }

    uint32_t Instructions::handleSHR8(uint8_t modrm, uint8_t count, uint8_t mod, uint8_t rm) {
        uint8_t value;
        uint32_t addr = 0;
        
        // Get the operand
        if (mod == 0b11) {
            // Register operand
            uint8_t* reg = get8BitRegisterRef(rm);
            value = *reg;
        } else {
            // Memory operand
            addr = getEffectiveAddress(mod, rm);
            value = memory.readByte(addr);
        }
        
        // Normalize count
        if (count > 8) count = 8;  // Shifts by more than operand size become 0 (8086 behavior)
        if (count == 0) return (mod == 0b11) ? cycles.SHIFT_REG_CL : cycles.SHIFT_MEM_CL;
        
        // Store MSB for OF
        bool msb = (value & 0x80) != 0;
        
        // For count > 1, CF = last bit shifted out, OF = undefined
        // Compute the result by shifting
        uint8_t result = value;
        bool carryOut = false;
        
        // SHR: Shift Logical Right (fills with 0s)
        for (uint8_t i = 0; i < count; i++) {
            carryOut = (result & 0x01) != 0;
            result >>= 1; // Logical shift, no sign extension
        }
        
        // Update Flags
        flags.setFlag(FLAGS::CF, carryOut);
        
        // OF on 8086: Only defined for count=1, OF = MSB of original operand
        if (count == 1) {
            flags.setFlag(FLAGS::OF, msb);
        }
        
        // ZF, SF, PF as usual
        flags.setFlag(FLAGS::ZF, result == 0);
        flags.setFlag(FLAGS::SF, (result & 0x80) != 0);
        
        // Parity (number of 1-bits is even)
        uint8_t bitCount = 0;
        for (int i = 0; i < 8; i++) {
            if (result & (1 << i)) bitCount++;
        }
        flags.setFlag(FLAGS::PF, (bitCount % 2) == 0);
        
        // Store the result
        if (mod == 0b11) {
            *get8BitRegisterRef(rm) = result;
        } else {
            memory.writeByte(addr, result);
        }
        return (mod == 0b11) ? cycles.SHIFT_REG_CL : cycles.SHIFT_MEM_CL;
    }
    
    uint32_t Instructions::handleSHR16(uint8_t modrm, uint8_t count, uint8_t mod, uint8_t rm) {
        uint16_t value;
        uint32_t addr = 0;
        
        // Get the operand
        if (mod == 0b11) {
            // Register operand
            uint16_t* reg = getRegisterReference(rm);
            value = *reg;
        } else {
            // Memory operand
            addr = getEffectiveAddress(mod, rm);
            value = memory.readWord(addr);
        }
        
        // Normalize count
        if (count > 16) count = 16;  // Shifts by more than operand size become 0 (8086 behavior)
        if (count == 0) return (mod == 0b11) ? cycles.SHIFT_REG_CL : cycles.SHIFT_MEM_CL;
        
        // Store MSB for OF
        bool msb = (value & 0x8000) != 0;
        
        // For count > 1, CF = last bit shifted out, OF = undefined
        // Compute the result by shifting
        uint16_t result = value;
        bool carryOut = false;
        
        // SHR: Shift Logical Right (fills with 0s)
        for (uint8_t i = 0; i < count; i++) {
            carryOut = (result & 0x0001) != 0;
            result >>= 1; // Logical shift, no sign extension
        }
        
        // Update Flags
        flags.setFlag(FLAGS::CF, carryOut);
        
        // OF on 8086: Only defined for count=1, OF = MSB of original operand
        if (count == 1) {
            flags.setFlag(FLAGS::OF, msb);
        }
        
        // ZF, SF, PF as usual
        flags.setFlag(FLAGS::ZF, result == 0);
        flags.setFlag(FLAGS::SF, (result & 0x8000) != 0);
        
        // Parity (number of 1-bits in low byte is even)
        uint8_t lowByte = result & 0xFF;
        uint8_t bitCount = 0;
        for (int i = 0; i < 8; i++) {
            if (lowByte & (1 << i)) bitCount++;
        }
        flags.setFlag(FLAGS::PF, (bitCount % 2) == 0);
        
        // Store the result
        if (mod == 0b11) {
            *getRegisterReference(rm) = result;
        } else {
            memory.writeWord(addr, result);
        }
        return (mod == 0b11) ? cycles.SHIFT_REG_CL : cycles.SHIFT_MEM_CL;
    }

    //--------------------------------------------------------------------------
    // Handle IRET
    //--------------------------------------------------------------------------
    uint32_t Instructions::handleIRET() {
        // 1. Pop IP from stack
        uint32_t stackAddr = memory.calculatePhysicalAddress(registers.SS, registers.SP);
        registers.IP = memory.readWord(stackAddr);
        registers.SP += 2;
        
        // 2. Pop CS from stack
        stackAddr = memory.calculatePhysicalAddress(registers.SS, registers.SP);
        registers.CS = memory.readWord(stackAddr);
        registers.SP += 2;
        
        // 3. Pop FLAGS from stack
        stackAddr = memory.calculatePhysicalAddress(registers.SS, registers.SP);
        uint16_t flagsValue = memory.readWord(stackAddr);
        registers.SP += 2;
        
        // 4. Set all FLAGS bits from the popped value
        for (int i = 0; i < 16; i++) {
            uint16_t flag = 1 << i;
            flags.setFlag(flag, (flagsValue & flag) != 0);
        }
        
        // Return cycle count for IRET
        return 32; // IRET typically takes ~32 cycles on 8086
    }

    // Helper to calculate effective address for ModR/M memory operations
    uint32_t Instructions::getEffectiveAddress(uint8_t mod, uint8_t rm) {
        uint16_t address = 0;
        
        if (mod == 0b00) {
            // No displacement, except for rm=110
            switch (rm) {
                case 0: // [BX + SI]
                    address = registers.BX.value + registers.SI;
                    break;
                case 1: // [BX + DI]
                    address = registers.BX.value + registers.DI;
                    break;
                case 2: // [BP + SI]
                    address = registers.BP + registers.SI;
                    break;
                case 3: // [BP + DI]
                    address = registers.BP + registers.DI;
                    break;
                case 4: // [SI]
                    address = registers.SI;
                    break;
                case 5: // [DI]
                    address = registers.DI;
                    break;
                case 6: // [disp16] or [BP + disp] if mod != 00
                    address = fetchWord(); // Direct address
                    break;
                case 7: // [BX]
                    address = registers.BX.value;
                    break;
            }
        } else if (mod == 0b01) {
            // 8-bit displacement
            int8_t disp8 = static_cast<int8_t>(fetchByte());
            
            switch (rm) {
                case 0: // [BX + SI + disp8]
                    address = registers.BX.value + registers.SI + disp8;
                    break;
                case 1: // [BX + DI + disp8]
                    address = registers.BX.value + registers.DI + disp8;
                    break;
                case 2: // [BP + SI + disp8]
                    address = registers.BP + registers.SI + disp8;
                    break;
                case 3: // [BP + DI + disp8]
                    address = registers.BP + registers.DI + disp8;
                    break;
                case 4: // [SI + disp8]
                    address = registers.SI + disp8;
                    break;
                case 5: // [DI + disp8]
                    address = registers.DI + disp8;
                    break;
                case 6: // [BP + disp8]
                    address = registers.BP + disp8;
                    break;
                case 7: // [BX + disp8]
                    address = registers.BX.value + disp8;
                    break;
            }
        } else if (mod == 0b10) {
            // 16-bit displacement
            int16_t disp16 = static_cast<int16_t>(fetchWord());
            
            switch (rm) {
                case 0: // [BX + SI + disp16]
                    address = registers.BX.value + registers.SI + disp16;
                    break;
                case 1: // [BX + DI + disp16]
                    address = registers.BX.value + registers.DI + disp16;
                    break;
                case 2: // [BP + SI + disp16]
                    address = registers.BP + registers.SI + disp16;
                    break;
                case 3: // [BP + DI + disp16]
                    address = registers.BP + registers.DI + disp16;
                    break;
                case 4: // [SI + disp16]
                    address = registers.SI + disp16;
                    break;
                case 5: // [DI + disp16]
                    address = registers.DI + disp16;
                    break;
                case 6: // [BP + disp16]
                    address = registers.BP + disp16;
                    break;
                case 7: // [BX + disp16]
                    address = registers.BX.value + disp16;
                    break;
            }
        }
        
        // Calculate physical address using the data segment (DS)
        // Note: Some instructions use different segment registers (ES, SS)
        return memory.calculatePhysicalAddress(registers.DS, address);
    }

    // Implementation for handleF6 function (0xF6 group)
    uint32_t Instructions::handleF6() {
        uint8_t modrm = fetchByte();
        uint8_t mod = (modrm >> 6) & 0x03;
        uint8_t op = (modrm >> 3) & 0x07;  // This is the specific operation within group F6
        uint8_t rm = modrm & 0x07;

        // Placeholder implementation to allow build
        std::cerr << "F6 group operation " << static_cast<int>(op) << " not fully implemented" << std::endl;
        return 10; // Default cycle count
    }

    // Implementation for handleF7 function (0xF7 group)
    uint32_t Instructions::handleF7() {
        uint8_t modrm = fetchByte();
        uint8_t mod = (modrm >> 6) & 0x03;
        uint8_t op = (modrm >> 3) & 0x07;  // This is the specific operation within group F7
        uint8_t rm = modrm & 0x07;

        // Placeholder implementation to allow build
        std::cerr << "F7 group operation " << static_cast<int>(op) << " not fully implemented" << std::endl;
        return 10; // Default cycle count
    }

    // Implementation for handleSAL8 function (Shift Arithmetic Left for 8-bit operands)
    uint32_t Instructions::handleSAL8(uint8_t modrm, uint8_t count, uint8_t mod, uint8_t rm) {
        uint8_t value;
        uint32_t addr = 0;
        
        // Get the operand
        if (mod == 0b11) {
            // Register operand
            uint8_t* reg = get8BitRegisterRef(rm);
            value = *reg;
        } else {
            // Memory operand
            addr = getEffectiveAddress(mod, rm);
            value = memory.readByte(addr);
        }
        
        // Skip operation if count is 0
        if (count == 0) return (mod == 0b11) ? cycles.SHIFT_REG_CL : cycles.SHIFT_MEM_CL;
        
        // Store MSB for OF
        bool msb = (value & 0x80) != 0;
        
        // Compute the result by shifting
        uint8_t result = value;
        bool carryOut = false;
        
        // SAL: Shift Arithmetic Left (same as SHL, fills with 0s)
        for (uint8_t i = 0; i < count; i++) {
            carryOut = (result & 0x80) != 0;
            result <<= 1;
        }
        
        // Update Flags
        flags.setFlag(FLAGS::CF, carryOut);
        
        // OF on 8086: Only defined for count=1, OF = MSB changed
        if (count == 1) {
            bool newMsb = (result & 0x80) != 0;
            flags.setFlag(FLAGS::OF, msb != newMsb);
        }
        
        // ZF, SF, PF as usual
        flags.setFlag(FLAGS::ZF, result == 0);
        flags.setFlag(FLAGS::SF, (result & 0x80) != 0);
        flags.setFlag(FLAGS::PF, utils.calculateParity(result));
        
        // Store the result
        if (mod == 0b11) {
            *get8BitRegisterRef(rm) = result;
        } else {
            memory.writeByte(addr, result);
        }
        
        return (mod == 0b11) ? cycles.SHIFT_REG_CL : cycles.SHIFT_MEM_CL;
    }

    // Implementation for handleSAL16 function (Shift Arithmetic Left for 16-bit operands)
    uint32_t Instructions::handleSAL16(uint8_t modrm, uint8_t count, uint8_t mod, uint8_t rm) {
        uint16_t value;
        uint32_t addr = 0;
        
        // Get the operand
        if (mod == 0b11) {
            // Register operand
            uint16_t* reg = getRegisterReference(rm);
            value = *reg;
        } else {
            // Memory operand
            addr = getEffectiveAddress(mod, rm);
            value = memory.readWord(addr);
        }
        
        // Skip operation if count is 0
        if (count == 0) return (mod == 0b11) ? cycles.SHIFT_REG_CL : cycles.SHIFT_MEM_CL;
        
        // Store MSB for OF
        bool msb = (value & 0x8000) != 0;
        
        // Compute the result by shifting
        uint16_t result = value;
        bool carryOut = false;
        
        // SAL: Shift Arithmetic Left (same as SHL, fills with 0s)
        for (uint8_t i = 0; i < count; i++) {
            carryOut = (result & 0x8000) != 0;
            result <<= 1;
        }
        
        // Update Flags
        flags.setFlag(FLAGS::CF, carryOut);
        
        // OF on 8086: Only defined for count=1, OF = MSB changed
        if (count == 1) {
            bool newMsb = (result & 0x8000) != 0;
            flags.setFlag(FLAGS::OF, msb != newMsb);
        }
        
        // ZF, SF, PF as usual
        flags.setFlag(FLAGS::ZF, result == 0);
        flags.setFlag(FLAGS::SF, (result & 0x8000) != 0);
        flags.setFlag(FLAGS::PF, utils.calculateParity(result));
        
        // Store the result
        if (mod == 0b11) {
            *getRegisterReference(rm) = result;
        } else {
            memory.writeWord(addr, result);
        }
        
        return (mod == 0b11) ? cycles.SHIFT_REG_CL : cycles.SHIFT_MEM_CL;
    }

    uint32_t Instructions::handleANDImm() {
        // Handle AND AL, imm8 (24) and AND AX, imm16 (25)
        uint8_t opcode = memory.readByte(memory.calculatePhysicalAddress(registers.CS, registers.IP - 1));
        uint32_t cycleCount = cycles.ALU_IMM_REG;
        
        if (opcode == 0x24) {
            // AND AL, imm8
            uint8_t imm8 = fetchByte();
            uint8_t al = registers.AX.low;
            
            uint8_t result = al & imm8;
            registers.AX.low = result;  // Update AL register
            
            // Set flags
            flags.setFlag(FLAGS::ZF, (result == 0));
            flags.setFlag(FLAGS::SF, (result & 0x80) != 0);
            flags.setFlag(FLAGS::OF, false);
            flags.setFlag(FLAGS::CF, false);
            flags.setFlag(FLAGS::AF, false);
            flags.setFlag(FLAGS::PF, utils.calculateParity(result));
        } else if (opcode == 0x25) {
            // AND AX, imm16
            uint16_t imm16 = fetchWord();
            uint16_t ax = registers.AX.value;
            
            uint32_t result = static_cast<uint32_t>(ax) & static_cast<uint32_t>(imm16);
            registers.AX.value = result & 0xFFFF;  // Update AX register
            setArithmeticFlags(result, ax, imm16);
        }
        
        return cycleCount;
    }

    uint32_t Instructions::handleORImm() {
        // Handle OR AL, imm8 (0C) and OR AX, imm16 (0D)
        uint8_t opcode = memory.readByte(memory.calculatePhysicalAddress(registers.CS, registers.IP - 1));
        uint32_t cycleCount = cycles.ALU_IMM_REG;
        
        if (opcode == 0x0C) {
            // OR AL, imm8
            uint8_t imm8 = fetchByte();
            uint8_t al = registers.AX.low;
            
            uint16_t result = al | imm8;
            registers.AX.low = result & 0xFF;  // Update AL register with result
            setArithmeticFlags8(result, al, imm8);
        } else if (opcode == 0x0D) {
            // OR AX, imm16
            uint16_t imm16 = fetchWord();
            uint16_t ax = registers.AX.value;
            
            uint32_t result = static_cast<uint32_t>(ax) | static_cast<uint32_t>(imm16);
            registers.AX.value = result & 0xFFFF;  // Update AX register with result
            setArithmeticFlags(result, ax, imm16);
        }
        
        return cycleCount;
    }

    uint32_t Instructions::handleXORImm() {
        // Handle XOR AL, imm8 (34) and XOR AX, imm16 (35)
        uint8_t opcode = memory.readByte(memory.calculatePhysicalAddress(registers.CS, registers.IP - 1));
        uint32_t cycleCount = cycles.ALU_IMM_REG;
        
        if (opcode == 0x34) {
            // XOR AL, imm8
            uint8_t imm8 = fetchByte();
            uint8_t al = registers.AX.low;
            
            uint16_t result = al ^ imm8;
            registers.AX.low = result & 0xFF;  // Update AL register with result
            setArithmeticFlags8(result, al, imm8);
        } else if (opcode == 0x35) {
            // XOR AX, imm16
            uint16_t imm16 = fetchWord();
            uint16_t ax = registers.AX.value;
            
            uint32_t result = static_cast<uint32_t>(ax) ^ static_cast<uint32_t>(imm16);
            registers.AX.value = result & 0xFFFF;  // Update AX register with result
            setArithmeticFlags(result, ax, imm16);
        }
        
        return cycleCount;
    }

} // namespace CPU
