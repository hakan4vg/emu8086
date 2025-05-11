#include "disassembler.hpp"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <algorithm>

namespace Disassembler {

    std::string Instruction::toString() const {
        std::stringstream ss;
        
        // Format: ADDRESS: BYTES MNEMONIC OPERANDS
        ss << std::hex << std::setw(8) << std::setfill('0') << address << ": ";
        
        // Bytes
        for (auto byte : bytes) {
            ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte) << " ";
        }
        
        // Padding to align mnemonics
        size_t padLength = 20 - bytes.size() * 3;
        ss << std::string(padLength, ' ');
        
        // Mnemonic and operands
        ss << mnemonic;
        if (!operands.empty()) {
            ss << " " << operands;
        }
        
        return ss.str();
    }
    
    Disassembler::Disassembler() {
        initializeOpcodeTables();
        initializeRegisterTables();
    }
    
    void Disassembler::initializeOpcodeTables() {
        // MOV instructions
        singleByteOpcodes[0x88] = "MOV";  // MOV r/m8, r8
        singleByteOpcodes[0x89] = "MOV";  // MOV r/m16, r16
        singleByteOpcodes[0x8A] = "MOV";  // MOV r8, r/m8
        singleByteOpcodes[0x8B] = "MOV";  // MOV r16, r/m16
        
        // MOV register immediate opcodes (B0-BF)
        for (uint8_t i = 0; i <= 7; i++) {
            singleByteOpcodes[0xB0 + i] = "MOV";  // MOV r8, imm8 (B0-B7)
            singleByteOpcodes[0xB8 + i] = "MOV";  // MOV r16, imm16 (B8-BF)
        }
        
        singleByteOpcodes[0xC6] = "MOV";  // MOV r/m8, imm8
        singleByteOpcodes[0xC7] = "MOV";  // MOV r/m16, imm16
        
        // Arithmetic instructions
        singleByteOpcodes[0x00] = "ADD";  // ADD r/m8, r8
        singleByteOpcodes[0x01] = "ADD";  // ADD r/m16, r16
        singleByteOpcodes[0x02] = "ADD";  // ADD r8, r/m8
        singleByteOpcodes[0x03] = "ADD";  // ADD r16, r/m16
        singleByteOpcodes[0x04] = "ADD";  // ADD AL, imm8
        singleByteOpcodes[0x05] = "ADD";  // ADD AX, imm16
        
        singleByteOpcodes[0x10] = "ADC";  // ADC r/m8, r8
        singleByteOpcodes[0x11] = "ADC";  // ADC r/m16, r16
        singleByteOpcodes[0x12] = "ADC";  // ADC r8, r/m8
        singleByteOpcodes[0x13] = "ADC";  // ADC r16, r/m16
        singleByteOpcodes[0x14] = "ADC";  // ADC AL, imm8
        singleByteOpcodes[0x15] = "ADC";  // ADC AX, imm16
        
        singleByteOpcodes[0x28] = "SUB";  // SUB r/m8, r8
        singleByteOpcodes[0x29] = "SUB";  // SUB r/m16, r16
        singleByteOpcodes[0x2A] = "SUB";  // SUB r8, r/m8
        singleByteOpcodes[0x2B] = "SUB";  // SUB r16, r/m16
        singleByteOpcodes[0x2C] = "SUB";  // SUB AL, imm8
        singleByteOpcodes[0x2D] = "SUB";  // SUB AX, imm16
        
        singleByteOpcodes[0x18] = "SBB";  // SBB r/m8, r8
        singleByteOpcodes[0x19] = "SBB";  // SBB r/m16, r16
        singleByteOpcodes[0x1A] = "SBB";  // SBB r8, r/m8
        singleByteOpcodes[0x1B] = "SBB";  // SBB r16, r/m16
        singleByteOpcodes[0x1C] = "SBB";  // SBB AL, imm8
        singleByteOpcodes[0x1D] = "SBB";  // SBB AX, imm16
        
        // Logical operations
        singleByteOpcodes[0x20] = "AND";  // AND r/m8, r8
        singleByteOpcodes[0x21] = "AND";  // AND r/m16, r16
        singleByteOpcodes[0x22] = "AND";  // AND r8, r/m8
        singleByteOpcodes[0x23] = "AND";  // AND r16, r/m16
        singleByteOpcodes[0x24] = "AND";  // AND AL, imm8
        singleByteOpcodes[0x25] = "AND";  // AND AX, imm16
        
        singleByteOpcodes[0x08] = "OR";   // OR r/m8, r8
        singleByteOpcodes[0x09] = "OR";   // OR r/m16, r16
        singleByteOpcodes[0x0A] = "OR";   // OR r8, r/m8
        singleByteOpcodes[0x0B] = "OR";   // OR r16, r/m16
        singleByteOpcodes[0x0C] = "OR";   // OR AL, imm8
        singleByteOpcodes[0x0D] = "OR";   // OR AX, imm16
        
        singleByteOpcodes[0x30] = "XOR";  // XOR r/m8, r8
        singleByteOpcodes[0x31] = "XOR";  // XOR r/m16, r16
        singleByteOpcodes[0x32] = "XOR";  // XOR r8, r/m8
        singleByteOpcodes[0x33] = "XOR";  // XOR r16, r/m16
        singleByteOpcodes[0x34] = "XOR";  // XOR AL, imm8
        singleByteOpcodes[0x35] = "XOR";  // XOR AX, imm16
        
        // Compare instructions
        singleByteOpcodes[0x38] = "CMP";  // CMP r/m8, r8
        singleByteOpcodes[0x39] = "CMP";  // CMP r/m16, r16
        singleByteOpcodes[0x3A] = "CMP";  // CMP r8, r/m8
        singleByteOpcodes[0x3B] = "CMP";  // CMP r16, r/m16
        singleByteOpcodes[0x3C] = "CMP";  // CMP AL, imm8
        singleByteOpcodes[0x3D] = "CMP";  // CMP AX, imm16
        
        // INC/DEC instructions
        for (uint8_t i = 0x40; i <= 0x47; i++) {
            singleByteOpcodes[i] = "INC";  // INC r16
        }
        for (uint8_t i = 0x48; i <= 0x4F; i++) {
            singleByteOpcodes[i] = "DEC";  // DEC r16
        }
        
        // Jump instructions
        singleByteOpcodes[0xEB] = "JMP";  // JMP rel8
        singleByteOpcodes[0xE9] = "JMP";  // JMP rel16
        singleByteOpcodes[0x74] = "JE";   // JE rel8
        singleByteOpcodes[0x75] = "JNE";  // JNE rel8
        singleByteOpcodes[0x7C] = "JL";   // JL rel8
        singleByteOpcodes[0x7D] = "JGE";  // JGE rel8
        singleByteOpcodes[0x7E] = "JLE";  // JLE rel8
        singleByteOpcodes[0x7F] = "JG";   // JG rel8
        
        // String operations
        singleByteOpcodes[0xA4] = "MOVSB"; // MOVSB
        singleByteOpcodes[0xA5] = "MOVSW"; // MOVSW
        singleByteOpcodes[0xA6] = "CMPSB"; // CMPSB
        singleByteOpcodes[0xA7] = "CMPSW"; // CMPSW
        singleByteOpcodes[0xAA] = "STOSB"; // STOSB
        singleByteOpcodes[0xAB] = "STOSW"; // STOSW
        singleByteOpcodes[0xAC] = "LODSB"; // LODSB
        singleByteOpcodes[0xAD] = "LODSW"; // LODSW
        singleByteOpcodes[0xAE] = "SCASB"; // SCASB
        singleByteOpcodes[0xAF] = "SCASW"; // SCASW
        singleByteOpcodes[0xF2] = "REPNE"; // REPNE/REPNZ prefix
        singleByteOpcodes[0xF3] = "REP";  // REP/REPE/REPZ prefix
        
        // Stack operations
        for (uint8_t i = 0x50; i <= 0x57; i++) {
            singleByteOpcodes[i] = "PUSH"; // PUSH r16
        }
        for (uint8_t i = 0x58; i <= 0x5F; i++) {
            singleByteOpcodes[i] = "POP";  // POP r16
        }
        
        // Misc operations
        singleByteOpcodes[0xF4] = "HLT";  // HLT
        singleByteOpcodes[0xCD] = "INT";  // INT imm8
        singleByteOpcodes[0xCF] = "IRET"; // IRET
        
        // I/O instructions
        singleByteOpcodes[0xE4] = "IN";   // IN AL, imm8
        singleByteOpcodes[0xE5] = "IN";   // IN AX, imm8
        singleByteOpcodes[0xEC] = "IN";   // IN AL, DX
        singleByteOpcodes[0xED] = "IN";   // IN AX, DX
        singleByteOpcodes[0xE6] = "OUT";  // OUT imm8, AL
        singleByteOpcodes[0xE7] = "OUT";  // OUT imm8, AX
        singleByteOpcodes[0xEE] = "OUT";  // OUT DX, AL
        singleByteOpcodes[0xEF] = "OUT";  // OUT DX, AX
        
        // Flag operations
        singleByteOpcodes[0xF8] = "CLC";  // CLC - Clear Carry Flag
        singleByteOpcodes[0xF9] = "STC";  // STC - Set Carry Flag
        singleByteOpcodes[0xF5] = "CMC";  // CMC - Complement Carry Flag
        singleByteOpcodes[0xFC] = "CLD";  // CLD - Clear Direction Flag
        singleByteOpcodes[0xFD] = "STD";  // STD - Set Direction Flag
        singleByteOpcodes[0xFA] = "CLI";  // CLI - Clear Interrupt Flag
        singleByteOpcodes[0xFB] = "STI";  // STI - Set Interrupt Flag
        
        // Shift and Rotate operations
        // These will be handled by the special Group 2 handler since they share opcodes
        singleByteOpcodes[0xD0] = "ROL";  // Group 2 opcodes - 8-bit rotates/shifts by 1
        singleByteOpcodes[0xD1] = "ROL";  // Group 2 opcodes - 16-bit rotates/shifts by 1
        singleByteOpcodes[0xD2] = "ROL";  // Group 2 opcodes - 8-bit rotates/shifts by CL
        singleByteOpcodes[0xD3] = "ROL";  // Group 2 opcodes - 16-bit rotates/shifts by CL
        
        // Group 1 opcodes (ADD, OR, ADC, SBB, AND, SUB, XOR, CMP with immediate operand)
        singleByteOpcodes[0x80] = "ADD";  // Group 1 opcodes - 8-bit immediate arithmetic
        singleByteOpcodes[0x81] = "ADD";  // Group 1 opcodes - 16-bit immediate arithmetic
        singleByteOpcodes[0x83] = "ADD";  // Group 1 opcodes - 16-bit sign-extended immediate arithmetic
    }
    
    void Disassembler::initializeRegisterTables() {
        // 8-bit registers
        registers8[0] = "AL";
        registers8[1] = "CL";
        registers8[2] = "DL";
        registers8[3] = "BL";
        registers8[4] = "AH";
        registers8[5] = "CH";
        registers8[6] = "DH";
        registers8[7] = "BH";
        
        // 16-bit registers
        registers16[0] = "AX";
        registers16[1] = "CX";
        registers16[2] = "DX";
        registers16[3] = "BX";
        registers16[4] = "SP";
        registers16[5] = "BP";
        registers16[6] = "SI";
        registers16[7] = "DI";
        
        // Segment registers
        segmentRegisters[0] = "ES";
        segmentRegisters[1] = "CS";
        segmentRegisters[2] = "SS";
        segmentRegisters[3] = "DS";
    }
    
    void Disassembler::setBaseAddress(uint32_t addr) {
        baseAddress = addr;
    }
    
    bool Disassembler::loadBinaryFile(const std::string& filename) {
        std::ifstream file(filename, std::ios::binary | std::ios::ate);
        if (!file.is_open()) {
            return false;
        }
        
        try {
            std::streamsize size = file.tellg();
            if (size <= 0 || size > 1024 * 1024) { // Sanity check: max 1MB file
                std::cerr << "File size invalid or too large: " << size << " bytes" << std::endl;
                return false;
            }
            
            file.seekg(0, std::ios::beg);
            
            binaryData.resize(size);
            if (!file.read(reinterpret_cast<char*>(binaryData.data()), size)) {
                binaryData.clear(); // Clean up if read fails
                return false;
            }
            
            position = 0;
            return true;
        } catch (const std::exception& e) {
            std::cerr << "Error loading binary file: " << e.what() << std::endl;
            binaryData.clear(); // Clean up on exception
            return false;
        }
    }
    
    void Disassembler::loadBinaryData(const std::vector<uint8_t>& data) {
        binaryData = data;
        position = 0;
    }
    
    uint8_t Disassembler::readByte() {
        if (position >= binaryData.size()) {
            throw std::out_of_range("End of binary data reached");
        }
        return binaryData[position++];
    }
    
    uint16_t Disassembler::readWord() {
        if (position + 1 >= binaryData.size()) {
            throw std::out_of_range("Not enough data to read a word");
        }
        uint8_t low = binaryData[position++];
        uint8_t high = binaryData[position++];
        return (high << 8) | low;
    }
    
    int8_t Disassembler::readSignedByte() {
        return static_cast<int8_t>(readByte());
    }
    
    int16_t Disassembler::readSignedWord() {
        return static_cast<int16_t>(readWord());
    }
    
    std::string Disassembler::getRegisterName(uint8_t reg, bool is16Bit) {
        if (is16Bit) {
            auto it = registers16.find(reg);
            if (it != registers16.end()) {
                return it->second;
            }
        } else {
            auto it = registers8.find(reg);
            if (it != registers8.end()) {
                return it->second;
            }
        }
        return "unknown";
    }
    
    std::string Disassembler::decodeModRM(uint8_t modrm, bool is16Bit) {
        try {
            uint8_t mod = (modrm >> 6) & 0x03;
            uint8_t rm = modrm & 0x07;
            
            if (mod == 0) {
                if (rm == 0) return "[BX+SI]";
                else if (rm == 1) return "[BX+DI]";
                else if (rm == 2) return "[BP+SI]";
                else if (rm == 3) return "[BP+DI]";
                else if (rm == 4) return "[SI]";
                else if (rm == 5) return "[DI]";
                else if (rm == 6) {
                    uint16_t disp = readWord();
                    std::stringstream ss;
                    ss << "[" << std::hex << disp << "h]";
                    return ss.str();
                }
                else if (rm == 7) return "[BX]";
            } else if (mod == 1) {
                int8_t disp = readSignedByte();
                std::stringstream ss;
                std::string baseReg;
                
                if (rm == 0) baseReg = "[BX+SI";
                else if (rm == 1) baseReg = "[BX+DI";
                else if (rm == 2) baseReg = "[BP+SI";
                else if (rm == 3) baseReg = "[BP+DI";
                else if (rm == 4) baseReg = "[SI";
                else if (rm == 5) baseReg = "[DI";
                else if (rm == 6) baseReg = "[BP";
                else if (rm == 7) baseReg = "[BX";
                
                ss << baseReg;
                if (disp >= 0) {
                    ss << "+" << static_cast<int>(disp) << "h]";
                } else {
                    ss << "-" << static_cast<int>(-disp) << "h]";
                }
                return ss.str();
            } else if (mod == 2) {
                int16_t disp = readSignedWord();
                std::stringstream ss;
                std::string baseReg;
                
                if (rm == 0) baseReg = "[BX+SI";
                else if (rm == 1) baseReg = "[BX+DI";
                else if (rm == 2) baseReg = "[BP+SI";
                else if (rm == 3) baseReg = "[BP+DI";
                else if (rm == 4) baseReg = "[SI";
                else if (rm == 5) baseReg = "[DI";
                else if (rm == 6) baseReg = "[BP";
                else if (rm == 7) baseReg = "[BX";
                
                ss << baseReg;
                if (disp >= 0) {
                    ss << "+" << std::hex << disp << "h]";
                } else {
                    ss << "-" << std::hex << -disp << "h]";
                }
                return ss.str();
            } else if (mod == 3) {
                return getRegisterName(rm, is16Bit);
            }
            
            return "UNKNOWN_MODRM";
        } catch (const std::out_of_range& e) {
            std::cerr << "Error decoding ModR/M: End of binary data reached" << std::endl;
            return "TRUNCATED_MODRM";
        } catch (const std::exception& e) {
            std::cerr << "Error decoding ModR/M: " << e.what() << std::endl;
            return "ERROR_MODRM";
        }
    }
    
    bool Disassembler::handleMOV(Instruction& instr, uint8_t opcode) {
        try {
            if (opcode >= 0xB8 && opcode <= 0xBF) {
                uint8_t reg = opcode & 0x07;
                
                if (position + 1 >= binaryData.size()) {
                    throw std::out_of_range("Not enough data for 16-bit immediate");
                }
                
                uint8_t low = readByte();
                uint8_t high = readByte();
                uint16_t imm = (high << 8) | low;
                
                instr.bytes.push_back(low);
                instr.bytes.push_back(high);
                
                std::stringstream ss;
                ss << getRegisterName(reg, true) << ", " << std::hex << imm << "h";
                instr.operands = ss.str();
                
                return true;
            }
            
            if (opcode >= 0xB0 && opcode <= 0xB7) {
                uint8_t reg = opcode & 0x07;
                
                if (position >= binaryData.size()) {
                    throw std::out_of_range("Not enough data for 8-bit immediate");
                }
                
                uint8_t imm = readByte();
                instr.bytes.push_back(imm);
                
                std::stringstream ss;
                ss << getRegisterName(reg, false) << ", " << std::hex << static_cast<int>(imm) << "h";
                instr.operands = ss.str();
                
                return true;
            }
            
            if (opcode >= 0x88 && opcode <= 0x8B) {
                uint8_t modrm = readByte();
                instr.bytes.push_back(modrm);
                
                bool isRegDest = (opcode & 0x02) != 0;
                bool is16Bit = (opcode & 0x01) != 0;
                
                uint8_t mod = (modrm >> 6) & 0x03;
                uint8_t reg = (modrm >> 3) & 0x07;
                uint8_t rm = modrm & 0x07;
                
                std::string regName = getRegisterName(reg, is16Bit);
                std::string rmString = decodeModRM(modrm, is16Bit);
                
                uint32_t basePos = position;
                for (size_t i = instr.bytes.size(); i < (basePos - (instr.address - baseAddress)); i++) {
                    if ((instr.address - baseAddress + i) < binaryData.size()) {
                        instr.bytes.push_back(binaryData[instr.address - baseAddress + i]);
                    }
                }
                
                if (isRegDest) {
                    instr.operands = regName + ", " + rmString;
                } else {
                    instr.operands = rmString + ", " + regName;
                }
                
                return true;
            } else if (opcode == 0xC6 || opcode == 0xC7) {
                uint8_t modrm = readByte();
                instr.bytes.push_back(modrm);
                
                bool is16Bit = (opcode == 0xC7);
                uint8_t reg = (modrm >> 3) & 0x07;
                
                if (reg != 0) {
                    return false;
                }
                
                std::string rmString = decodeModRM(modrm, is16Bit);
                
                uint32_t basePos = position;
                for (size_t i = instr.bytes.size(); i < (basePos - (instr.address - baseAddress)); i++) {
                    if ((instr.address - baseAddress + i) < binaryData.size()) {
                        instr.bytes.push_back(binaryData[instr.address - baseAddress + i]);
                    }
                }
                
                if (is16Bit) {
                    uint16_t imm = readWord();
                    instr.bytes.push_back(imm & 0xFF);
                    instr.bytes.push_back((imm >> 8) & 0xFF);
                    
                    std::stringstream ss;
                    ss << rmString << ", " << std::hex << imm << "h";
                    instr.operands = ss.str();
                } else {
                    uint8_t imm = readByte();
                    instr.bytes.push_back(imm);
                    
                    std::stringstream ss;
                    ss << rmString << ", " << std::hex << static_cast<int>(imm) << "h";
                    instr.operands = ss.str();
                }
                
                return true;
            }
            
            return false;
        } catch (const std::exception& e) {
            std::cerr << "Error handling MOV instruction: " << e.what() << std::endl;
            return false;
        }
    }
    
    bool Disassembler::handleArithmetic(Instruction& instr, uint8_t opcode) {
        // Record opcode
        instr.bytes.push_back(opcode);
        
        // Handle ADD, SUB, CMP with common logic
        if ((opcode >= 0x00 && opcode <= 0x03) ||   // ADD
            (opcode >= 0x28 && opcode <= 0x2B) ||   // SUB
            (opcode >= 0x38 && opcode <= 0x3B)) {   // CMP
            
            bool toReg = (opcode & 0x02) != 0;
            bool is16Bit = (opcode & 0x01) != 0;
            
            // Read ModRM byte
            uint8_t modrm = readByte();
            instr.bytes.push_back(modrm);
            
            uint8_t mod = (modrm >> 6) & 0x03;
            uint8_t reg = (modrm >> 3) & 0x07;
            uint8_t rm = modrm & 0x07;
            
            std::string regName = getRegisterName(reg, is16Bit);
            std::string rmStr = decodeModRM(modrm, is16Bit);
            
            // Add any displacement bytes that were read
            uint32_t instruction_start_index_in_binary = instr.address - baseAddress;
            size_t bytes_already_in_instr_vector = instr.bytes.size(); // Should be 2 (opcode + modrm)
            size_t start_of_displacement_in_binary = instruction_start_index_in_binary + bytes_already_in_instr_vector;

            for (size_t i = start_of_displacement_in_binary; i < position; ++i) {
                if (i < binaryData.size()) { // Ensure we do not read out of bounds
                    instr.bytes.push_back(binaryData[i]);
                } else {
                    break; // Similar to above, indicates an issue or truncated instruction
                }
            }
            
            if (toReg) {
                instr.operands = regName + ", " + rmStr;
            } else {
                instr.operands = rmStr + ", " + regName;
            }
            return true;
        } else if (opcode >= 0x40 && opcode <= 0x4F) {
            // INC/DEC r16
            uint8_t reg = opcode & 0x07;
            instr.operands = getRegisterName(reg, true);
            return true;
        }
        
        return false;
    }
    
    bool Disassembler::handleJump(Instruction& instr, uint8_t opcode) {
        // Record opcode
        instr.bytes.push_back(opcode);
        
        if (opcode == 0xEB || // JMP rel8
            opcode == 0x74 || // JE rel8
            opcode == 0x75 || // JNE rel8
            opcode == 0x7C || // JL rel8
            opcode == 0x7D || // JGE rel8
            opcode == 0x7E || // JLE rel8
            opcode == 0x7F) { // JG rel8
            
            int8_t offset = readSignedByte();
            instr.bytes.push_back(static_cast<uint8_t>(offset));
            
            uint32_t targetAddr = baseAddress + position + offset;
            
            std::stringstream ss;
            ss << std::hex << targetAddr;
            instr.operands = ss.str() + "h";
            return true;
        } else if (opcode == 0xE9) { // JMP rel16
            int16_t offset = readSignedWord();
            instr.bytes.push_back(offset & 0xFF);
            instr.bytes.push_back((offset >> 8) & 0xFF);
            
            uint32_t targetAddr = baseAddress + position + offset;
            
            std::stringstream ss;
            ss << std::hex << targetAddr;
            instr.operands = ss.str() + "h";
            return true;
        }
        
        return false;
    }
    
    bool Disassembler::handleStack(Instruction& instr, uint8_t opcode) {
        // Record opcode
        instr.bytes.push_back(opcode);
        
        if (opcode >= 0x50 && opcode <= 0x57) { // PUSH r16
            uint8_t reg = opcode & 0x07;
            instr.operands = getRegisterName(reg, true);
            return true;
        } else if (opcode >= 0x58 && opcode <= 0x5F) { // POP r16
            uint8_t reg = opcode & 0x07;
            instr.operands = getRegisterName(reg, true);
            return true;
        }
        
        return false;
    }
    
    bool Disassembler::handleString(Instruction& instr, uint8_t opcode) {
        instr.bytes.push_back(opcode);
        
        if (opcode == 0xA4 || opcode == 0xA5 || // MOVSB, MOVSW
            opcode == 0xA6 || opcode == 0xA7 || // CMPSB, CMPSW
            opcode == 0xAA || opcode == 0xAB || // STOSB, STOSW
            opcode == 0xAC || opcode == 0xAD || // LODSB, LODSW
            opcode == 0xAE || opcode == 0xAF) { // SCASB, SCASW
            
            return true;
        } else if (opcode == 0xF2 || opcode == 0xF3) { // REP prefixes
            uint8_t nextOpcode = readByte();
            instr.bytes.push_back(nextOpcode);
            
            auto it = singleByteOpcodes.find(nextOpcode);
            if (it != singleByteOpcodes.end()) {
                instr.mnemonic += " " + it->second;
            } else {
                instr.mnemonic += " ???";
            }
            
            return true;
        }
        
        return false;
    }
    
    bool Disassembler::handleMisc(Instruction& instr, uint8_t opcode) {
        // Record opcode
        instr.bytes.push_back(opcode);
        
        if (opcode == 0xCD) { // INT imm8
            uint8_t intNum = readByte();
            instr.bytes.push_back(intNum);
            
            std::stringstream ss;
            ss << std::hex << static_cast<int>(intNum) << "h";
            instr.operands = ss.str();
            return true;
        } else if (opcode == 0xF4) { // HLT
            // No operands
            return true;
        } else if (opcode >= 0xE4 && opcode <= 0xE7) { // IN/OUT with immediate port
            uint8_t port = readByte();
            instr.bytes.push_back(port);
            
            std::stringstream ss;
            if (opcode == 0xE4) { // IN AL, imm8
                ss << "AL, " << static_cast<int>(port);
            } else if (opcode == 0xE5) { // IN AX, imm8
                ss << "AX, " << static_cast<int>(port);
            } else if (opcode == 0xE6) { // OUT imm8, AL
                ss << static_cast<int>(port) << ", AL";
            } else if (opcode == 0xE7) { // OUT imm8, AX
                ss << static_cast<int>(port) << ", AX";
            }
            instr.operands = ss.str();
            return true;
        } else if (opcode >= 0xEC && opcode <= 0xEF) { // IN/OUT with DX port
            if (opcode == 0xEC) { // IN AL, DX
                instr.operands = "AL, DX";
            } else if (opcode == 0xED) { // IN AX, DX
                instr.operands = "AX, DX";
            } else if (opcode == 0xEE) { // OUT DX, AL
                instr.operands = "DX, AL";
            } else if (opcode == 0xEF) { // OUT DX, AX
                instr.operands = "DX, AX";
            }
            return true;
        }
        
        return false;
    }
    
    bool Disassembler::handleROL(Instruction& instr, uint8_t opcode) {
        try {
            uint8_t modrm = readByte();
            instr.bytes.push_back(modrm);
            
            uint8_t mod = (modrm >> 6) & 0x03;
            uint8_t reg = (modrm >> 3) & 0x07;
            uint8_t rm = modrm & 0x07;
            
            std::string operation;
            switch (reg) {
                case 0: operation = "ROL"; break;
                case 1: operation = "ROR"; break;
                case 2: operation = "RCL"; break;
                case 3: operation = "RCR"; break;
                case 4: operation = "SHL"; break;
                case 5: operation = "SHR"; break;
                case 7: operation = "SAR"; break;
                default: operation = "???"; break;
            }
            
            instr.mnemonic = operation;
            
            bool is16Bit = (opcode == 0xD1 || opcode == 0xD3);
            bool useCL = (opcode == 0xD2 || opcode == 0xD3);
            
            std::string rmStr = decodeModRM(modrm, is16Bit);
            
            uint32_t basePos = position;
            for (size_t i = instr.bytes.size(); i < (basePos - (instr.address - baseAddress)); i++) {
                if ((instr.address - baseAddress + i) < binaryData.size()) {
                    instr.bytes.push_back(binaryData[instr.address - baseAddress + i]);
                }
            }
            
            if (useCL) {
                instr.operands = rmStr + ", CL";
            } else {
                instr.operands = rmStr + ", 1";
            }
            
            return true;
        } catch (const std::out_of_range& e) {
            std::cerr << "Truncated shift/rotate instruction at address 0x" << std::hex << instr.address << ": " << e.what() << std::endl;
            instr.mnemonic = "DB";
            std::stringstream ss;
            for (const auto& byte : instr.bytes) {
                ss << std::hex << static_cast<int>(byte) << "h ";
            }
            instr.operands = ss.str();
            return true;
        } catch (const std::exception& e) {
            std::cerr << "Error processing shift/rotate instruction: " << e.what() << std::endl;
            return false;
        }
    }
    
    bool Disassembler::handleGroup1(Instruction& instr, uint8_t opcode) {
        // Read ModR/M byte
        uint8_t modrm = readByte();
        instr.bytes.push_back(modrm);
        
        // Extract fields from ModR/M
        uint8_t mod = (modrm >> 6) & 0x03;
        uint8_t reg = (modrm >> 3) & 0x07;  // This determines the operation
        uint8_t rm = modrm & 0x07;
        
        // Determine the operation based on reg field
        std::string operation;
        switch (reg) {
            case 0: operation = "ADD"; break;
            case 1: operation = "OR"; break;
            case 2: operation = "ADC"; break;
            case 3: operation = "SBB"; break;
            case 4: operation = "AND"; break;
            case 5: operation = "SUB"; break;
            case 6: operation = "XOR"; break;
            case 7: operation = "CMP"; break;
            default: operation = "???"; break;
        }
        
        // Update the mnemonic to the actual operation
        instr.mnemonic = operation;
        
        // Determine operand size and immediate size
        bool is16Bit = (opcode != 0x80);
        bool signExtImm = (opcode == 0x83);
        
        // Get the destination operand
        std::string rmStr = decodeModRM(modrm, is16Bit);
        
        // Add any displacement bytes that were read in decodeModRM
        uint32_t basePos = position;
        for (size_t i = instr.bytes.size(); i < (basePos - (instr.address - baseAddress)); i++) {
            if ((instr.address - baseAddress + i) < binaryData.size()) {
                instr.bytes.push_back(binaryData[instr.address - baseAddress + i]);
            }
        }
        
        // Read the immediate value
        std::stringstream ss;
        if (signExtImm) {
            // 8-bit sign-extended immediate for 16-bit operand
            int8_t imm8 = static_cast<int8_t>(readByte());
            instr.bytes.push_back(static_cast<uint8_t>(imm8));
            ss << rmStr << ", " << std::hex << static_cast<int>(imm8) << "h";
        } else if (is16Bit) {
            // 16-bit immediate
            uint16_t imm16 = readWord();
            instr.bytes.push_back(imm16 & 0xFF);
            instr.bytes.push_back((imm16 >> 8) & 0xFF);
            ss << rmStr << ", " << std::hex << imm16 << "h";
        } else {
            // 8-bit immediate
            uint8_t imm8 = readByte();
            instr.bytes.push_back(imm8);
            ss << rmStr << ", " << std::hex << static_cast<int>(imm8) << "h";
        }
        
        instr.operands = ss.str();
        
        return true;
    }
    
    bool Disassembler::decodeInstruction(Instruction& instr) {
        if (position >= binaryData.size()) {
            return false;
        }
        
        // Set address
        instr.address = baseAddress + position;
        
        try {
            // Read opcode
            uint8_t opcode = binaryData[position++];
            instr.bytes.push_back(opcode);
            
            // Get mnemonic
            auto it = singleByteOpcodes.find(opcode);
            if (it != singleByteOpcodes.end()) {
                instr.mnemonic = it->second;
            } else {
                instr.mnemonic = "DB";
                std::stringstream ss;
                ss << std::hex << static_cast<int>(opcode) << "h";
                instr.operands = ss.str();
                return true;
            }
            
            // Handle specific instruction types
            if ((opcode >= 0x88 && opcode <= 0x8B) || // MOV r/m, r or r, r/m
                (opcode >= 0xB0 && opcode <= 0xBF) || // MOV r, imm
                opcode == 0xC6 || opcode == 0xC7) {   // MOV r/m, imm
                return handleMOV(instr, opcode);
            } else if ((opcode >= 0x00 && opcode <= 0x03) || // ADD
                    (opcode >= 0x28 && opcode <= 0x2B) || // SUB
                    (opcode >= 0x38 && opcode <= 0x3B)) { // CMP
                return handleArithmetic(instr, opcode);
            } else if (opcode == 0xEB || opcode == 0xE9 || // JMP
                    (opcode >= 0x74 && opcode <= 0x7F)) { // Jcc
                return handleJump(instr, opcode);
            } else if ((opcode >= 0x50 && opcode <= 0x5F)) { // PUSH/POP
                return handleStack(instr, opcode);
            } else if ((opcode >= 0xA4 && opcode <= 0xAF) || // String ops
                    opcode == 0xF2 || opcode == 0xF3) {    // REP prefixes
                return handleString(instr, opcode);
            } else if (opcode == 0xCD || opcode == 0xF4 || // INT, HLT
                    (opcode >= 0xE4 && opcode <= 0xEF)) { // IN/OUT
                return handleMisc(instr, opcode);
            } else if (opcode == 0xD0 || opcode == 0xD1 || opcode == 0xD2 || opcode == 0xD3) { // ROL, ROR, RCL, RCR, SHL, SHR, SAR
                return handleROL(instr, opcode);
            } else if (opcode >= 0x80 && opcode <= 0x83) { // Group 1 opcodes
                return handleGroup1(instr, opcode);
            } else if ((opcode >= 0x20 && opcode <= 0x23) || // AND
                      (opcode >= 0x08 && opcode <= 0x0B) || // OR
                      (opcode >= 0x30 && opcode <= 0x33) || // XOR
                      opcode == 0x24 || opcode == 0x25 ||   // AND AL/AX
                      opcode == 0x0C || opcode == 0x0D ||   // OR AL/AX
                      opcode == 0x34 || opcode == 0x35) {   // XOR AL/AX
                // These are logical operations
                return handleArithmetic(instr, opcode);
            } else {
                // Unknown or unhandled opcode, just mark as data byte
                instr.mnemonic = "DB";
                std::stringstream ss;
                ss << std::hex << static_cast<int>(opcode) << "h";
                instr.operands = ss.str();
                return true;
            }
        } catch (const std::out_of_range& e) {
            // We've reached the end of the binary data or had an indexing issue
            std::cerr << "Out of range error during disassembly: " << e.what() << std::endl;
            if (instr.bytes.empty()) {
                // Add the current position as a data byte if we haven't added anything yet
                if (position < binaryData.size()) {
                    instr.bytes.push_back(binaryData[position++]);
                    instr.mnemonic = "DB";
                    std::stringstream ss;
                    ss << std::hex << static_cast<int>(instr.bytes[0]) << "h";
                    instr.operands = ss.str();
                } else {
                    return false;
                }
            }
            return true;
        } catch (const std::exception& e) {
            // Handle any other exceptions
            std::cerr << "Error during disassembly: " << e.what() << std::endl;
            if (instr.bytes.empty() && position < binaryData.size()) {
                instr.bytes.push_back(binaryData[position++]);
                instr.mnemonic = "DB";
                std::stringstream ss;
                ss << std::hex << static_cast<int>(instr.bytes[0]) << "h";
                instr.operands = ss.str();
            }
            return true;
        }
    }
    
    bool Disassembler::disassemble() {
        instructions.clear();
        position = 0;
        
        if (binaryData.empty()) {
            std::cerr << "Cannot disassemble empty binary data" << std::endl;
            return false;
        }
        
        try {
            size_t maxInstructionCount = 10000; // Reasonable limit to prevent infinite loops
            
            // For debugging
            std::cout << "Binary data size: " << binaryData.size() << " bytes\n";
            
            while (position < binaryData.size() && instructions.size() < maxInstructionCount) {
                size_t savedPosition = position;
                uint32_t currentAddress = baseAddress + position;
                Instruction instr;
                instr.address = currentAddress;
                
                // Check for specific byte patterns
                // Handle register immediates first (they're very common and easy to identify)
                uint8_t opcode = binaryData[position];
                
                // Handle 16-bit register immediates (B8-BF)
                if (opcode >= 0xB8 && opcode <= 0xBF && position + 2 < binaryData.size()) {
                    // This is a MOV reg16, imm16 instruction
                    instr.mnemonic = "MOV";
                    instr.bytes.push_back(binaryData[position++]);
                    
                    // Read 16-bit immediate
                    if (position + 1 >= binaryData.size()) {
                        // Not enough bytes for a full immediate
                        position = savedPosition;
                        break;
                    }
                    
                    uint8_t low = binaryData[position++];
                    uint8_t high = binaryData[position++];
                    instr.bytes.push_back(low);
                    instr.bytes.push_back(high);
                    
                    uint16_t imm = (high << 8) | low;
                    
                    // Get register name
                    uint8_t reg = opcode & 0x07;
                    std::string regName = getRegisterName(reg, true);
                    
                    // Format operands
                    std::stringstream ss;
                    ss << regName << ", " << std::hex << imm << "h";
                    instr.operands = ss.str();
                    
                    instructions.push_back(instr);
                    continue;
                }
                
                // Handle 8-bit register immediates (B0-B7)
                if (opcode >= 0xB0 && opcode <= 0xB7 && position + 1 < binaryData.size()) {
                    // This is a MOV reg8, imm8 instruction
                    instr.mnemonic = "MOV";
                    instr.bytes.push_back(binaryData[position++]);
                    
                    // Read 8-bit immediate
                    if (position >= binaryData.size()) {
                        // Not enough bytes for a full immediate
                        position = savedPosition;
                        break;
                    }
                    
                    uint8_t imm = binaryData[position++];
                    instr.bytes.push_back(imm);
                    
                    // Get register name
                    uint8_t reg = opcode & 0x07;
                    std::string regName = getRegisterName(reg, false);
                    
                    // Format operands
                    std::stringstream ss;
                    ss << regName << ", " << std::hex << static_cast<int>(imm) << "h";
                    instr.operands = ss.str();
                    
                    instructions.push_back(instr);
                    continue;
                }
                
                // Handle INT instruction pattern
                if (opcode == 0xCD && position + 1 < binaryData.size()) {
                    instr.mnemonic = "INT";
                    instr.bytes.push_back(binaryData[position++]);
                    
                    uint8_t intNum = binaryData[position++];
                    instr.bytes.push_back(intNum);
                    
                    std::stringstream ss;
                    ss << std::hex << static_cast<int>(intNum) << "h";
                    instr.operands = ss.str();
                    
                    instructions.push_back(instr);
                    continue;
                }
                
                // Handle shift/rotate instructions (D0-D3)
                if ((opcode == 0xD0 || opcode == 0xD1 || opcode == 0xD2 || opcode == 0xD3) && 
                    position + 1 < binaryData.size()) {
                    
                    instr.bytes.push_back(binaryData[position++]);
                    uint8_t modrm = binaryData[position++];
                    instr.bytes.push_back(modrm);
                    
                    // Extract fields from ModR/M
                    uint8_t mod = (modrm >> 6) & 0x03;
                    uint8_t reg = (modrm >> 3) & 0x07;  // Determines the operation
                    uint8_t rm = modrm & 0x07;
                    
                    // Determine the operation based on reg field
                    std::string operation;
                    switch (reg) {
                        case 0: operation = "ROL"; break;
                        case 1: operation = "ROR"; break;
                        case 2: operation = "RCL"; break;
                        case 3: operation = "RCR"; break;
                        case 4: operation = "SHL"; break;
                        case 5: operation = "SHR"; break;
                        case 7: operation = "SAR"; break;
                        default: operation = "DB"; break; // Unknown operation
                    }
                    
                    // Update the mnemonic to the actual operation
                    instr.mnemonic = operation;
                    
                    // Determine operand size and shift count
                    bool is16Bit = (opcode == 0xD1 || opcode == 0xD3);
                    bool useCL = (opcode == 0xD2 || opcode == 0xD3);
                    
                    // Get the destination operand
                    std::string destOperand;
                    if (mod == 0b11) {
                        // Register operand
                        destOperand = getRegisterName(rm, is16Bit);
                    } else {
                        // Memory operand with potential displacement
                        // This is a simplified approach - for a real implementation, 
                        // you'd need to properly decode the ModR/M addressing mode
                        destOperand = "MEMORY";
                        
                        // Add displacement bytes to instruction bytes
                        if (mod == 0b01) {
                            // 8-bit displacement
                            if (position < binaryData.size()) {
                                instr.bytes.push_back(binaryData[position++]);
                            }
                        } else if (mod == 0b10) {
                            // 16-bit displacement
                            if (position + 1 < binaryData.size()) {
                                instr.bytes.push_back(binaryData[position++]);
                                instr.bytes.push_back(binaryData[position++]);
                            }
                        }
                    }
                    
                    // Format the operands
                    std::stringstream ss;
                    ss << destOperand << ", " << (useCL ? "CL" : "1");
                    instr.operands = ss.str();
                    
                    instructions.push_back(instr);
                    continue;
                }
                
                // Handle HLT instruction
                if (opcode == 0xF4) {
                    instr.mnemonic = "HLT";
                    instr.bytes.push_back(binaryData[position++]);
                    instructions.push_back(instr);
                    continue;
                }
                
                // Fall back to regular decoding if no special case matched
                position = savedPosition;
                if (decodeInstruction(instr)) {
                    instructions.push_back(instr);
                } else {
                    // If we can't decode, treat as a data byte and advance
                    instr.mnemonic = "DB";
                    instr.bytes.push_back(binaryData[position++]);
                    std::stringstream ss;
                    ss << std::hex << static_cast<int>(instr.bytes[0]) << "h";
                    instr.operands = ss.str();
                    instructions.push_back(instr);
                }
            }
            
            if (instructions.size() >= maxInstructionCount) {
                std::cerr << "Warning: Reached maximum instruction count limit" << std::endl;
            }
        } catch (const std::exception& e) {
            std::cerr << "Disassembly error: " << e.what() << std::endl;
            // Continue with what we've got so far
        }
        
        return !instructions.empty();
    }
    
    const std::vector<Instruction>& Disassembler::getInstructions() const {
        return instructions;
    }
    
    std::string Disassembler::toString() const {
        std::stringstream ss;
        
        for (const auto& instr : instructions) {
            ss << instr.toString() << "\n";
        }
        
        return ss.str();
    }
    
    bool Disassembler::saveToFile(const std::string& filename) const {
        std::ofstream file(filename);
        if (!file.is_open()) {
            return false;
        }
        
        file << "; 8086 Disassembly\n";
        file << "; Base address: " << std::hex << baseAddress << "\n\n";
        
        for (const auto& instr : instructions) {
            file << instr.toString() << "\n";
        }
        
        return true;
    }
    
    bool Disassembler::disassembleFile(const std::string& inputFile, const std::string& outputFile) {
        // Clear any previous data to avoid memory issues
        binaryData.clear();
        instructions.clear();
        position = 0;

        try {
            if (!loadBinaryFile(inputFile)) {
                std::cerr << "Failed to load binary file: " << inputFile << std::endl;
                return false;
            }
            
            if (binaryData.empty()) {
                std::cerr << "Binary file is empty: " << inputFile << std::endl;
                return false;
            }
            
            std::cout << "Loaded " << binaryData.size() << " bytes from " << inputFile << std::endl;
            
            if (!disassemble()) {
                std::cerr << "Disassembly failed for file: " << inputFile << std::endl;
                return false;
            }
            
            std::cout << "Disassembled " << instructions.size() << " instructions" << std::endl;
            
            if (!saveToFile(outputFile)) {
                std::cerr << "Failed to save output file: " << outputFile << std::endl;
                return false;
            }
            
            // Clean up to free memory
            binaryData.clear();
            instructions.clear();
            
            return true;
        } catch (const std::exception& e) {
            std::cerr << "Critical error during disassembly: " << e.what() << std::endl;
            
            // Clean up resources even in case of error
            binaryData.clear();
            instructions.clear();
            position = 0;
            
            return false;
        }
    }

} // namespace Disassembler 