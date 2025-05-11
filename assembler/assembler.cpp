#include "assembler.hpp"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <regex>
#include <iomanip>

namespace Assembler {

    // Helper function to trim whitespace from a string
    static std::string trim(const std::string& str) {
        const auto strBegin = str.find_first_not_of(" \t\r\n");
        if (strBegin == std::string::npos)
            return "";
        
        const auto strEnd = str.find_last_not_of(" \t\r\n");
        const auto strRange = strEnd - strBegin + 1;
        
        return str.substr(strBegin, strRange);
    }
    
    // Helper function to convert string to uppercase
    static std::string toUpper(const std::string& str) {
        std::string result = str;
        std::transform(result.begin(), result.end(), result.begin(), 
                      [](unsigned char c) { return std::toupper(c); });
        return result;
    }
    
    // Helper function to convert hex string to integer
    static int hexToInt(const std::string& hex) {
        std::stringstream ss;
        ss << std::hex << hex;
        int result;
        ss >> result;
        return result;
    }

    Assembler::Assembler() {
        initializeRegisterTable();
        initializeInstructionTable();
    }
    
    void Assembler::initializeRegisterTable() {
        // 8-bit registers
        registerCodes["AL"] = 0;
        registerCodes["CL"] = 1;
        registerCodes["DL"] = 2;
        registerCodes["BL"] = 3;
        registerCodes["AH"] = 4;
        registerCodes["CH"] = 5;
        registerCodes["DH"] = 6;
        registerCodes["BH"] = 7;
        
        // 16-bit registers
        registerCodes["AX"] = 0;
        registerCodes["CX"] = 1;
        registerCodes["DX"] = 2;
        registerCodes["BX"] = 3;
        registerCodes["SP"] = 4;
        registerCodes["BP"] = 5;
        registerCodes["SI"] = 6;
        registerCodes["DI"] = 7;
        
        // Segment registers
        registerCodes["ES"] = 0;
        registerCodes["CS"] = 1;
        registerCodes["SS"] = 2;
        registerCodes["DS"] = 3;
    }
    
    void Assembler::initializeInstructionTable() {
        // MOV instructions
        instructionTable["MOV_R_R"] = {0x8B}; // MOV r16, r/m16
        instructionTable["MOV_R_M"] = {0x8B}; // MOV r16, m16
        instructionTable["MOV_M_R"] = {0x89}; // MOV m16, r16
        instructionTable["MOV_R_I"] = {0xB8}; // MOV r16, imm16 (Base + reg)
        instructionTable["MOV_M_I"] = {0xC7}; // MOV m16, imm16
        
        // Arithmetic instructions
        instructionTable["ADD_R_R"] = {0x01}; // ADD r/m16, r16
        instructionTable["ADD_R_I"] = {0x81, 0xC0}; // ADD r16, imm16
        instructionTable["ADD_R8_R8"] = {0x00}; // ADD r/m8, r8
        instructionTable["ADD_R8_I"] = {0x80, 0xC0}; // ADD r8, imm8
        instructionTable["ADD_AL_I"] = {0x04}; // ADD AL, imm8
        instructionTable["ADD_AX_I"] = {0x05}; // ADD AX, imm16
        
        // ADC instructions (Add with Carry)
        instructionTable["ADC_R_R"] = {0x11}; // ADC r/m16, r16
        instructionTable["ADC_R8_R8"] = {0x10}; // ADC r/m8, r8
        instructionTable["ADC_R_M"] = {0x13}; // ADC r16, r/m16
        instructionTable["ADC_R8_M"] = {0x12}; // ADC r8, r/m8
        instructionTable["ADC_R_I"] = {0x81, 0xD0}; // ADC r16, imm16
        instructionTable["ADC_R8_I"] = {0x80, 0xD0}; // ADC r8, imm8
        instructionTable["ADC_AL_I"] = {0x14}; // ADC AL, imm8
        instructionTable["ADC_AX_I"] = {0x15}; // ADC AX, imm16
        
        // SUB
        instructionTable["SUB_R_R"] = {0x29}; // SUB r/m16, r16
        instructionTable["SUB_R_M"] = {0x2B}; // SUB r16, r/m16
        instructionTable["SUB_R8_R8"] = {0x28}; // SUB r/m8, r8
        instructionTable["SUB_R8_M"] = {0x2A}; // SUB r8, r/m8
        instructionTable["SUB_R_I"] = {0x81, 0xE8}; // SUB r16, imm16
        instructionTable["SUB_R8_I"] = {0x80, 0xE8}; // SUB r8, imm8
        instructionTable["SUB_AL_I"] = {0x2C}; // SUB AL, imm8
        instructionTable["SUB_AX_I"] = {0x2D}; // SUB AX, imm16
        
        // SBB instructions (Subtract with Borrow)
        instructionTable["SBB_R_R"] = {0x19}; // SBB r/m16, r16
        instructionTable["SBB_R8_R8"] = {0x18}; // SBB r/m8, r8
        instructionTable["SBB_R_M"] = {0x1B}; // SBB r16, r/m16
        instructionTable["SBB_R8_M"] = {0x1A}; // SBB r8, r/m8
        instructionTable["SBB_R_I"] = {0x81, 0xD8}; // SBB r16, imm16
        instructionTable["SBB_R8_I"] = {0x80, 0xD8}; // SBB r8, imm8
        instructionTable["SBB_AL_I"] = {0x1C}; // SBB AL, imm8
        instructionTable["SBB_AX_I"] = {0x1D}; // SBB AX, imm16
        
        instructionTable["INC_R"] = {0x40}; // INC r16 (Base + reg)
        instructionTable["DEC_R"] = {0x48}; // DEC r16 (Base + reg)
        
        // Compare instructions
        instructionTable["CMP_R_R"] = {0x39}; // CMP r/m16, r16
        instructionTable["CMP_R_I"] = {0x81, 0xF8}; // CMP r16, imm16
        instructionTable["CMP_M_R"] = {0x39}; // CMP m16, r16
        instructionTable["CMP_R_M"] = {0x3B}; // CMP r16, m16
        instructionTable["CMP_R8_R8"] = {0x38}; // CMP r/m8, r8
        instructionTable["CMP_R8_M"] = {0x3A}; // CMP r8, r/m8
        instructionTable["CMP_AL_I"] = {0x3C}; // CMP AL, imm8
        instructionTable["CMP_AX_I"] = {0x3D}; // CMP AX, imm16
        
        // Jump instructions
        instructionTable["JMP"] = {0xEB}; // JMP rel8
        instructionTable["JMP_FAR"] = {0xE9}; // JMP rel16
        instructionTable["JE"] = {0x74}; // JE rel8
        instructionTable["JZ"] = {0x74}; // JZ rel8 (same as JE)
        instructionTable["JNE"] = {0x75}; // JNE rel8
        instructionTable["JG"] = {0x7F}; // JG rel8
        instructionTable["JGE"] = {0x7D}; // JGE rel8
        instructionTable["JL"] = {0x7C}; // JL rel8
        instructionTable["JLE"] = {0x7E}; // JLE rel8
        
        // Flag operations
        instructionTable["CLC"] = {0xF8}; // Clear Carry Flag
        instructionTable["STC"] = {0xF9}; // Set Carry Flag
        instructionTable["CMC"] = {0xF5}; // Complement Carry Flag
        instructionTable["CLD"] = {0xFC}; // Clear Direction Flag
        instructionTable["STD"] = {0xFD}; // Set Direction Flag
        instructionTable["CLI"] = {0xFA}; // Clear Interrupt Flag
        instructionTable["STI"] = {0xFB}; // Set Interrupt Flag
        
        // String operations
        instructionTable["MOVSB"] = {0xA4}; // MOVSB
        instructionTable["MOVSW"] = {0xA5}; // MOVSW
        instructionTable["CMPSB"] = {0xA6}; // CMPSB
        instructionTable["CMPSW"] = {0xA7}; // CMPSW
        instructionTable["STOSB"] = {0xAA}; // STOSB
        instructionTable["STOSW"] = {0xAB}; // STOSW
        instructionTable["LODSB"] = {0xAC}; // LODSB
        instructionTable["LODSW"] = {0xAD}; // LODSW
        instructionTable["SCASB"] = {0xAE}; // SCASB
        instructionTable["SCASW"] = {0xAF}; // SCASW
        instructionTable["REPNE"] = {0xF2}; // REPNE/REPNZ prefix
        instructionTable["REPNZ"] = {0xF2}; // REPNE/REPNZ prefix
        instructionTable["REP"] = {0xF3}; // REP/REPE/REPZ prefix
        instructionTable["REPE"] = {0xF3}; // REP/REPE/REPZ prefix
        instructionTable["REPZ"] = {0xF3}; // REP/REPE/REPZ prefix
        
        // Stack operations
        instructionTable["PUSH_R"] = {0x50}; // PUSH r16 (Base + reg)
        instructionTable["POP_R"] = {0x58}; // POP r16 (Base + reg)
        
        // I/O operations
        instructionTable["IN_AL"] = {0xE4}; // IN AL, imm8
        instructionTable["IN_AX"] = {0xE5}; // IN AX, imm8
        instructionTable["IN_AL_DX"] = {0xEC}; // IN AL, DX
        instructionTable["IN_AX_DX"] = {0xED}; // IN AX, DX
        instructionTable["OUT_I_AL"] = {0xE6}; // OUT imm8, AL
        instructionTable["OUT_I_AX"] = {0xE7}; // OUT imm8, AX
        instructionTable["OUT_DX_AL"] = {0xEE}; // OUT DX, AL
        instructionTable["OUT_DX_AX"] = {0xEF}; // OUT DX, AX
        
        // Logical operations
        instructionTable["AND_R_R"] = {0x21}; // AND r/m16, r16
        instructionTable["AND_R8_R8"] = {0x20}; // AND r/m8, r8
        instructionTable["AND_R_M"] = {0x23}; // AND r16, r/m16
        instructionTable["AND_R8_M"] = {0x22}; // AND r8, r/m8
        instructionTable["AND_M_R"] = {0x21}; // AND r/m16, r16
        instructionTable["AND_M_R8"] = {0x20}; // AND r/m8, r8
        instructionTable["AND_R_I"] = {0x81, 0xE0}; // AND r16, imm16
        instructionTable["AND_R8_I"] = {0x80, 0xE0}; // AND r8, imm8
        instructionTable["AND_AL_I"] = {0x24}; // AND AL, imm8
        instructionTable["AND_AX_I"] = {0x25}; // AND AX, imm16

        instructionTable["OR_R_R"] = {0x09}; // OR r/m16, r16
        instructionTable["OR_R8_R8"] = {0x08}; // OR r/m8, r8
        instructionTable["OR_R_M"] = {0x0B}; // OR r16, r/m16
        instructionTable["OR_R8_M"] = {0x0A}; // OR r8, r/m8
        instructionTable["OR_M_R"] = {0x09}; // OR r/m16, r16
        instructionTable["OR_M_R8"] = {0x08}; // OR r/m8, r8
        instructionTable["OR_R_I"] = {0x81, 0xC8}; // OR r16, imm16
        instructionTable["OR_R8_I"] = {0x80, 0xC8}; // OR r8, imm8
        instructionTable["OR_AL_I"] = {0x0C}; // OR AL, imm8
        instructionTable["OR_AX_I"] = {0x0D}; // OR AX, imm16

        instructionTable["XOR_R_R"] = {0x31}; // XOR r/m16, r16
        instructionTable["XOR_R8_R8"] = {0x30}; // XOR r/m8, r8
        instructionTable["XOR_R_M"] = {0x33}; // XOR r16, r/m16
        instructionTable["XOR_R8_M"] = {0x32}; // XOR r8, r/m8
        instructionTable["XOR_M_R"] = {0x31}; // XOR r/m16, r16
        instructionTable["XOR_M_R8"] = {0x30}; // XOR r/m8, r8
        instructionTable["XOR_R_I"] = {0x81, 0xF0}; // XOR r16, imm16
        instructionTable["XOR_R8_I"] = {0x80, 0xF0}; // XOR r8, imm8
        instructionTable["XOR_AL_I"] = {0x34}; // XOR AL, imm8
        instructionTable["XOR_AX_I"] = {0x35}; // XOR AX, imm16
        
        // Shift and Rotate (using Group 2 opcodes)
        instructionTable["ROL_R8_1"] = {0xD0, 0xC0}; // ROL r8, 1
        instructionTable["ROL_R_1"] = {0xD1, 0xC0}; // ROL r16, 1
        instructionTable["ROL_R8_CL"] = {0xD2, 0xC0}; // ROL r8, CL
        instructionTable["ROL_R_CL"] = {0xD3, 0xC0}; // ROL r16, CL
        
        instructionTable["ROR_R8_1"] = {0xD0, 0xC8}; // ROR r8, 1
        instructionTable["ROR_R_1"] = {0xD1, 0xC8}; // ROR r16, 1
        instructionTable["ROR_R8_CL"] = {0xD2, 0xC8}; // ROR r8, CL
        instructionTable["ROR_R_CL"] = {0xD3, 0xC8}; // ROR r16, CL
        
        instructionTable["RCL_R8_1"] = {0xD0, 0xD0}; // RCL r8, 1
        instructionTable["RCL_R_1"] = {0xD1, 0xD0}; // RCL r16, 1
        instructionTable["RCL_R8_CL"] = {0xD2, 0xD0}; // RCL r8, CL
        instructionTable["RCL_R_CL"] = {0xD3, 0xD0}; // RCL r16, CL
        
        instructionTable["RCR_R8_1"] = {0xD0, 0xD8}; // RCR r8, 1
        instructionTable["RCR_R_1"] = {0xD1, 0xD8}; // RCR r16, 1
        instructionTable["RCR_R8_CL"] = {0xD2, 0xD8}; // RCR r8, CL
        instructionTable["RCR_R_CL"] = {0xD3, 0xD8}; // RCR r16, CL
        
        instructionTable["SHL_R8_1"] = {0xD0, 0xE0}; // SHL r8, 1
        instructionTable["SHL_R_1"] = {0xD1, 0xE0}; // SHL r16, 1
        instructionTable["SHL_R8_CL"] = {0xD2, 0xE0}; // SHL r8, CL
        instructionTable["SHL_R_CL"] = {0xD3, 0xE0}; // SHL r16, CL
        
        instructionTable["SHR_R8_1"] = {0xD0, 0xE8}; // SHR r8, 1
        instructionTable["SHR_R_1"] = {0xD1, 0xE8}; // SHR r16, 1
        instructionTable["SHR_R8_CL"] = {0xD2, 0xE8}; // SHR r8, CL
        instructionTable["SHR_R_CL"] = {0xD3, 0xE8}; // SHR r16, CL
        
        instructionTable["SAR_R8_1"] = {0xD0, 0xF8}; // SAR r8, 1
        instructionTable["SAR_R_1"] = {0xD1, 0xF8}; // SAR r16, 1
        instructionTable["SAR_R8_CL"] = {0xD2, 0xF8}; 
        instructionTable["SAR_R_CL"] = {0xD3, 0xF8}; 
        
        // Control flow
        instructionTable["CALL"] = {0xE8};
        instructionTable["RET"] = {0xC3};
        instructionTable["IRET"] = {0xCF};
        
        // Misc operations
        instructionTable["INT"] = {0xCD};
        instructionTable["HLT"] = {0xF4};
        
        // Data definition
        instructionTable["DB"] = {0x00};
    }
    
    bool Assembler::parseOperand(const std::string& operandStr, Operand& operand) {
        std::string op = trim(operandStr);
        if (op.empty()) {
            return false;
        }
        
        // Check if it's a register
        std::string opUpper = toUpper(op);
        if (registerCodes.find(opUpper) != registerCodes.end()) {
            operand.type = OperandType::REGISTER;
            operand.value = opUpper;
            operand.size = (opUpper.length() == 2 && opUpper[1] != 'X' && 
                           opUpper != "SI" && opUpper != "DI" && 
                           opUpper != "BP" && opUpper != "SP") ? 8 : 16;
            return true;
        }
        
        // Check for string literal in single quotes
        if (op[0] == '\'' && op.back() == '\'') {
            operand.type = OperandType::STRING;
            operand.value = op.substr(1, op.length() - 2);
            return true;
        }
        
        // If the string starts with a quote but doesn't end with one, 
        // there might be parsing issues. Provide a better error.
        if (op[0] == '\'' && op.back() != '\'') {
            errors.push_back("Unterminated string literal: " + op);
            return false;
        }
        
        // Check for character literal in single quotes - one character between quotes
        if (op.length() == 3 && op[0] == '\'' && op[2] == '\'') {
            operand.type = OperandType::IMMEDIATE;
            operand.value = op;
            operand.displacement = static_cast<int>(op[1]);
            operand.hasDisplacement = true;
            operand.size = 8;  // Character literal is always 8-bit
            return true;
        }
        
        // Handle single character (likely from templates/macros)
        if (op.length() == 1 && std::isalpha(op[0])) {
            operand.type = OperandType::IMMEDIATE;
            operand.value = op;
            operand.displacement = static_cast<int>(op[0]);
            operand.hasDisplacement = true;
            operand.size = 8;  // Character is 8-bit
            return true;
        }
        
        // Check if it's an immediate value
        if (op[0] == '#' || op[0] == '$' || isdigit(op[0]) || 
            (op[0] == '0' && op.length() > 1 && (op[1] == 'x' || op[1] == 'X')) ||
            (op[0] == '-' && op.length() > 1 && isdigit(op[1]))) {
            
            operand.type = OperandType::IMMEDIATE;
            // Remove # or $ prefix
            if (op[0] == '#' || op[0] == '$') {
                op = op.substr(1);
            }
            
            // Handle hex values (0x or h suffix)
            if (op.substr(0, 2) == "0x" || op.substr(0, 2) == "0X") {
                operand.value = op.substr(2);
                operand.displacement = hexToInt(operand.value);
            } else if (op.length() > 1 && (op.back() == 'h' || op.back() == 'H')) {
                operand.value = op.substr(0, op.length() - 1);
                operand.displacement = hexToInt(operand.value);
            } else {
                operand.value = op;
                operand.displacement = std::stoi(op);
            }
            
            operand.hasDisplacement = true;
            operand.size = (operand.displacement > 255 || operand.displacement < -128) ? 16 : 8;
            return true;
        }
        
        // Check if it's a memory reference: [reg + reg + disp]
        if (op[0] == '[' && op.back() == ']') {
            operand.type = OperandType::MEMORY;
            std::string addrExpr = op.substr(1, op.length() - 2); // Remove []
            
            // Parse based on + signs
            std::stringstream ss(addrExpr);
            std::string token;
            while (std::getline(ss, token, '+')) {
                token = trim(token);
                std::string tokenUpper = toUpper(token);
                
                if (registerCodes.find(tokenUpper) != registerCodes.end()) {
                    if (operand.baseReg.empty()) {
                        operand.baseReg = tokenUpper;
                    } else if (operand.indexReg.empty()) {
                        operand.indexReg = tokenUpper;
                    } else {
                        errors.push_back("Too many registers in memory addressing mode: " + op);
                        return false;
                    }
                } else if (isdigit(token[0]) || token[0] == '-') {
                    operand.displacement = std::stoi(token);
                    operand.hasDisplacement = true;
                } else if (token.length() > 2 && (token.substr(0, 2) == "0x" || token.substr(0, 2) == "0X")) {
                    operand.displacement = hexToInt(token.substr(2));
                    operand.hasDisplacement = true;
                } else if (token.length() > 1 && (token.back() == 'h' || token.back() == 'H')) {
                    operand.displacement = hexToInt(token.substr(0, token.length() - 1));
                    operand.hasDisplacement = true;
                } else {
                    errors.push_back("Invalid token in memory addressing: " + token);
                    return false;
                }
            }
            
            return true;
        }
        
        // If none of the above, it's a label reference
        operand.type = OperandType::LABEL;
        operand.value = op;
        return true;
    }
    
    bool Assembler::parseInstruction(const std::string& line, Instruction& instr) {
        try {
            std::string cleanLine = line;
            
            // Remove comments
            size_t commentPos = cleanLine.find(';');
            if (commentPos != std::string::npos) {
                cleanLine = cleanLine.substr(0, commentPos);
            }
            
            cleanLine = trim(cleanLine);
            if (cleanLine.empty()) {
                return false;
            }
            
            // Check for label pattern MSG_XXX DB 'string' without requiring colon
            std::regex dbPattern(R"(^(\w+)\s+DB\s+['"].*)");
            std::smatch matches;
            if (std::regex_search(cleanLine, matches, dbPattern) && matches.size() > 1) {
                std::string labelName = matches[1].str();
                if (isValidLabel(labelName)) {
                    // Store the label
                    labels[labelName] = Label(labelName, currentAddress);
                    
                    // Create a DB instruction
                    instr.mnemonic = "DB";
                    instr.address = currentAddress;
                    
                    // Extract the rest of the line (operands)
                    size_t dbPos = cleanLine.find("DB");
                    std::string operandsStr = trim(cleanLine.substr(dbPos + 2));
                    
                    // Process operands for DB directive
                    size_t pos = 0;
                    while (pos < operandsStr.length()) {
                        // Skip leading whitespace
                        while (pos < operandsStr.length() && std::isspace(operandsStr[pos])) {
                            pos++;
                        }
                        
                        if (pos >= operandsStr.length()) break;
                        
                        // Handle string literals in quotes
                        if (operandsStr[pos] == '\'' || operandsStr[pos] == '"') {
                            char quoteChar = operandsStr[pos];
                            size_t endQuotePos = operandsStr.find(quoteChar, pos + 1);
                            if (endQuotePos == std::string::npos) {
                                errors.push_back("Unterminated string literal in DB directive");
                                return false;
                            }
                            
                            // Extract the string including quotes
                            std::string strToken = operandsStr.substr(pos, endQuotePos - pos + 1);
                            
                            // Parse the string literal
                            Operand op;
                            op.type = OperandType::STRING;
                            op.value = strToken.substr(1, strToken.length() - 2); // Remove quotes
                            instr.operands.push_back(op);
                            
                            // Move position past this string token and any following comma
                            pos = endQuotePos + 1;
                            while (pos < operandsStr.length() && (std::isspace(operandsStr[pos]) || operandsStr[pos] == ',')) {
                                pos++;
                            }
                        } else {
                            // Non-string operand (could be a number or label)
                            size_t commaPos = operandsStr.find(',', pos);
                            std::string token;
                            if (commaPos != std::string::npos) {
                                token = trim(operandsStr.substr(pos, commaPos - pos));
                                pos = commaPos + 1;
                            } else {
                                token = trim(operandsStr.substr(pos));
                                pos = operandsStr.length();
                            }
                            
                            // Check if token is a numeric value or a label
                            if (isdigit(token[0]) || token[0] == '-' || 
                                (token.length() > 2 && (token.substr(0, 2) == "0x" || token.substr(0, 2) == "0X")) ||
                                (token.length() > 1 && (token.back() == 'h' || token.back() == 'H'))) {
                                // Numeric value
                                Operand op;
                                op.type = OperandType::IMMEDIATE;
                                int value = 0;
                                
                                if (token.substr(0, 2) == "0x" || token.substr(0, 2) == "0X") {
                                    value = hexToInt(token.substr(2));
                                } else if (token.length() > 1 && (token.back() == 'h' || token.back() == 'H')) {
                                    value = hexToInt(token.substr(0, token.length() - 1));
                                } else {
                                    value = std::stoi(token);
                                }
                                
                                op.value = token;
                                op.displacement = value;
                                op.hasDisplacement = true;
                                op.size = 8; // DB is 8-bit
                                instr.operands.push_back(op);
                            } else if (token == "0") {
                                // Special case for zero
                                Operand op;
                                op.type = OperandType::IMMEDIATE;
                                op.value = "0";
                                op.displacement = 0;
                                op.hasDisplacement = true;
                                op.size = 8;
                                instr.operands.push_back(op);
                            } else {
                                // Treat as a label reference
                                Operand op;
                                op.type = OperandType::LABEL;
                                op.value = token;
                                instr.operands.push_back(op);
                            }
                        }
                    }
                    
                    return true;
                }
            }
            
            // Check for labels (ending with :)
            size_t colonPos = cleanLine.find(':');
            if (colonPos != std::string::npos) {
                std::string labelName = trim(cleanLine.substr(0, colonPos));
                if (isValidLabel(labelName)) {
                    // Store label
                    labels[labelName] = Label(labelName, currentAddress);
                    
                    // Process rest of line after label, if any
                    if (colonPos + 1 < cleanLine.length()) {
                        return parseInstruction(cleanLine.substr(colonPos + 1), instr);
                    }
                    return false;
                } else {
                    errors.push_back("Invalid label name: " + labelName);
                    return false;
                }
            }
            
            // Check for "label instruction" pattern (common in assembler)
            // Example: "MSG_INIT DB 'Initial value: ', 0"
            std::istringstream iss(cleanLine);
            std::string firstToken, secondToken;
            iss >> firstToken >> secondToken;
            
            // Check if this might be a label followed by directive
            if (!firstToken.empty() && !secondToken.empty() && isValidLabel(firstToken)) {
                // Check if the second token is a directive (like DB)
                std::string secondTokenUpper = toUpper(secondToken);
                if (secondTokenUpper == "DB" || secondTokenUpper == "DW" || secondTokenUpper == "DD") {
                    // This is a label definition followed by directive
                    labels[firstToken] = Label(firstToken, currentAddress);
                    
                    // Remove the label from the line and parse the rest as an instruction
                    size_t pos = cleanLine.find(firstToken) + firstToken.length();
                    // Skip whitespace after the label
                    while (pos < cleanLine.length() && std::isspace(cleanLine[pos])) {
                        pos++;
                    }
                    
                    return parseInstruction(cleanLine.substr(pos), instr);
                }
            }
            
            // Normal instruction parsing
            std::stringstream ss(cleanLine);
            std::string mnemonic;
            ss >> mnemonic;
            
            instr.mnemonic = toUpper(mnemonic);
            instr.address = currentAddress;
            
            // Get the rest of the line for operands
            std::string operandsStr;
            std::getline(ss, operandsStr);
            operandsStr = trim(operandsStr);
            
            // Special handling for DB directive
            if (instr.mnemonic == "DB") {
                size_t pos = 0;
                std::string token;
                
                // Split by commas, handling hex values correctly
                while (pos < operandsStr.length()) {
                    // Skip leading whitespace
                    while (pos < operandsStr.length() && std::isspace(operandsStr[pos])) {
                        pos++;
                    }
                    
                    if (pos >= operandsStr.length()) break;
                    
                    // Handle string literals specially
                    if (operandsStr[pos] == '\'' || operandsStr[pos] == '"') {
                        char quoteChar = operandsStr[pos];
                        // Find the matching closing quote
                        size_t endQuotePos = operandsStr.find(quoteChar, pos + 1);
                        if (endQuotePos == std::string::npos) {
                            errors.push_back("Unterminated string literal in DB directive");
                            return false;
                        }
                        
                        // Extract the string including quotes
                        token = operandsStr.substr(pos, endQuotePos - pos + 1);
                        
                        // Get next position (after the comma)
                        size_t nextCommaPos = operandsStr.find(',', endQuotePos + 1);
                        if (nextCommaPos == std::string::npos) {
                            pos = operandsStr.length();
                        } else {
                            pos = nextCommaPos + 1;
                        }
                        
                        // Parse as a string operand
                        Operand op;
                        op.type = OperandType::STRING;
                        op.value = token.substr(1, token.length() - 2); // Remove quotes
                        instr.operands.push_back(op);
                        
                        continue;
                    }
                    
                    // For non-string literals, handle as before
                    size_t endPos = operandsStr.find(',', pos);
                    if (endPos == std::string::npos) {
                        endPos = operandsStr.length();
                    }
                    
                    // Extract the token
                    token = trim(operandsStr.substr(pos, endPos - pos));
                    
                    // Check if token is a numeric value or a label
                    if (isdigit(token[0]) || token[0] == '-' || 
                        (token.length() > 2 && (token.substr(0, 2) == "0x" || token.substr(0, 2) == "0X")) ||
                        (token.length() > 1 && (token.back() == 'h' || token.back() == 'H'))) {
                        // Numeric value
                        Operand op;
                        op.type = OperandType::IMMEDIATE;
                        
                        // Handle hex values (0x prefix or h suffix)
                        if (token.substr(0, 2) == "0x" || token.substr(0, 2) == "0X") {
                            op.value = token.substr(2);
                            op.displacement = hexToInt(op.value);
                        } else if (token.length() > 1 && (token.back() == 'h' || token.back() == 'H')) {
                            op.value = token.substr(0, token.length() - 1);
                            op.displacement = hexToInt(op.value);
                        } else {
                            op.value = token;
                            try {
                                op.displacement = std::stoi(token);
                            } catch (const std::exception& e) {
                                errors.push_back("Invalid numeric value in DB directive: " + token);
                                return false;
                            }
                        }
                        
                        op.hasDisplacement = true;
                        op.size = 8;  // DB is always 8-bit
                        instr.operands.push_back(op);
                    } else if (token == "0") {
                        // Special case for zero
                        Operand op;
                        op.type = OperandType::IMMEDIATE;
                        op.value = "0";
                        op.displacement = 0;
                        op.hasDisplacement = true;
                        op.size = 8;
                        instr.operands.push_back(op);
                    } else {
                        // Treat as a label reference
                        Operand op;
                        op.type = OperandType::LABEL;
                        op.value = token;
                        instr.operands.push_back(op);
                    }
                    
                    // Move to the next token
                    pos = endPos + 1;
                }
                
                return !instr.operands.empty();
            }
            
            // Parse operands if present (for non-DB instructions)
            if (!operandsStr.empty()) {
                size_t pos = 0;
                std::string token;
                
                // Split by commas, but handle commas inside brackets []
                int bracketLevel = 0;
                std::string currentOperand;
                
                for (char c : operandsStr) {
                    if (c == '[') bracketLevel++;
                    if (c == ']') bracketLevel--;
                    
                    if (c == ',' && bracketLevel == 0) {
                        // End of operand
                        Operand op;
                        
                        // Special character literal handling (e.g., 'S', 'R', etc. in the examples/simple_shifts.asm file)
                        if (trim(currentOperand).length() == 3 && 
                            trim(currentOperand)[0] == '\'' && 
                            trim(currentOperand)[2] == '\'') {
                            // This is a character literal like 'S'
                            op.type = OperandType::IMMEDIATE;
                            op.value = trim(currentOperand);
                            op.displacement = static_cast<int>(trim(currentOperand)[1]); // Get ASCII value
                            op.hasDisplacement = true;
                            op.size = 8; // 8-bit char
                            instr.operands.push_back(op);
                        } else if (trim(currentOperand).length() == 1 && 
                                 std::isalpha(trim(currentOperand)[0])) {
                            // Single character outside of quotes (e.g., S, R, L, etc.)
                            op.type = OperandType::IMMEDIATE;
                            op.value = trim(currentOperand);
                            op.displacement = static_cast<int>(trim(currentOperand)[0]); // Get ASCII value
                            op.hasDisplacement = true;
                            op.size = 8; // 8-bit char
                            instr.operands.push_back(op);
                        } else if (parseOperand(currentOperand, op)) {
                            instr.operands.push_back(op);
                        }
                        currentOperand.clear();
                    } else {
                        currentOperand += c;
                    }
                }
                
                // Handle last operand
                if (!currentOperand.empty()) {
                    Operand op;
                    
                    // Special character literal handling
                    if (trim(currentOperand).length() == 3 && 
                        trim(currentOperand)[0] == '\'' && 
                        trim(currentOperand)[2] == '\'') {
                        // This is a character literal like 'S'
                        op.type = OperandType::IMMEDIATE;
                        op.value = trim(currentOperand);
                        op.displacement = static_cast<int>(trim(currentOperand)[1]); // Get ASCII value
                        op.hasDisplacement = true;
                        op.size = 8; // 8-bit char
                        instr.operands.push_back(op);
                    } else if (trim(currentOperand).length() == 1 && 
                             std::isalpha(trim(currentOperand)[0])) {
                        // Single character outside of quotes (e.g., S, R, L, etc.)
                        op.type = OperandType::IMMEDIATE;
                        op.value = trim(currentOperand);
                        op.displacement = static_cast<int>(trim(currentOperand)[0]); // Get ASCII value
                        op.hasDisplacement = true;
                        op.size = 8; // 8-bit char
                        instr.operands.push_back(op);
                    } else if (parseOperand(currentOperand, op)) {
                        instr.operands.push_back(op);
                    }
                }
            }
            
            return true;
        } catch (const std::exception& e) {
            // Add some error handling to catch potential issues
            errors.push_back("Error parsing instruction: " + std::string(e.what()));
            return false;
        }
    }
    
    bool Assembler::isValidLabel(const std::string& label) {
        if (label.empty()) return false;
        
        // First character must be a letter or underscore
        if (!isalpha(label[0]) && label[0] != '_') return false;
        
        // Rest can be alphanumeric or underscore
        for (size_t i = 1; i < label.length(); i++) {
            if (!isalnum(label[i]) && label[i] != '_') return false;
        }
        
        return true;
    }
    
    bool Assembler::firstPass(const std::vector<std::string>& lines) {
        currentAddress = 0;
        labels.clear();
        parsedInstructions.clear();
        errors.clear();
        
        for (const auto& line : lines) {
            // Special handling for lines that might contain label definitions with DB
            if (line.find("DB") != std::string::npos || line.find("db") != std::string::npos) {
                std::string cleanLine = line;
                // Remove comments
                size_t commentPos = cleanLine.find(';');
                if (commentPos != std::string::npos) {
                    cleanLine = cleanLine.substr(0, commentPos);
                }
                cleanLine = trim(cleanLine);
                
                // Check for label + DB pattern
                std::istringstream iss(cleanLine);
                std::string labelPart, directive;
                iss >> labelPart >> directive;
                
                directive = toUpper(directive);
                if (isValidLabel(labelPart) && directive == "DB") {
                    // Found a label with DB directive
                    labels[labelPart] = Label(labelPart, currentAddress);
                    
                    // Create an instruction for the DB directive
                    Instruction instr;
                    instr.mnemonic = "DB";
                    instr.address = currentAddress;
                    
                    // Extract the rest of the line (operands)
                    size_t pos = cleanLine.find(directive) + directive.length();
                    std::string operandsStr = trim(cleanLine.substr(pos));
                    
                    // Parse DB operands (including string literals)
                    size_t strPos = 0;
                    while (strPos < operandsStr.length()) {
                        // Skip whitespace
                        while (strPos < operandsStr.length() && std::isspace(operandsStr[strPos])) {
                            strPos++;
                        }
                        if (strPos >= operandsStr.length()) break;
                        
                        if (operandsStr[strPos] == '\'') {
                            // String literal - find the closing quote
                            size_t endQuote = operandsStr.find('\'', strPos + 1);
                            if (endQuote == std::string::npos) {
                                errors.push_back("Unterminated string literal in DB directive");
                                break;
                            }
                            
                            // Extract the string
                            std::string stringValue = operandsStr.substr(strPos + 1, endQuote - strPos - 1);
                            
                            // Add each character as a DB byte
                            for (char c : stringValue) {
                                Operand op;
                                op.type = OperandType::IMMEDIATE;
                                op.value = std::to_string(static_cast<int>(c));
                                op.displacement = static_cast<int>(c);
                                op.hasDisplacement = true;
                                op.size = 8;
                                instr.operands.push_back(op);
                                currentAddress++; // Each character takes 1 byte
                            }
                            
                            // Move past the string and the closing quote
                            strPos = endQuote + 1;
                            
                            // Check for comma
                            size_t commaPos = operandsStr.find(',', strPos);
                            if (commaPos != std::string::npos) {
                                strPos = commaPos + 1;
                            } else {
                                break;
                            }
                        } else {
                            // Number or other operand
                            size_t commaPos = operandsStr.find(',', strPos);
                            std::string token;
                            if (commaPos != std::string::npos) {
                                token = trim(operandsStr.substr(strPos, commaPos - strPos));
                                strPos = commaPos + 1;
                            } else {
                                token = trim(operandsStr.substr(strPos));
                                strPos = operandsStr.length();
                            }
                            
                            // Special handling for numeric constants
                            if (token == "0") {
                                Operand op;
                                op.type = OperandType::IMMEDIATE;
                                op.value = "0";
                                op.displacement = 0;
                                op.hasDisplacement = true;
                                op.size = 8;
                                instr.operands.push_back(op);
                                currentAddress++; // Zero byte
                            } else {
                                try {
                                    int value = 0;
                                    if (token.substr(0, 2) == "0x" || token.substr(0, 2) == "0X") {
                                        value = hexToInt(token.substr(2));
                                    } else if (token.length() > 1 && (token.back() == 'h' || token.back() == 'H')) {
                                        value = hexToInt(token.substr(0, token.length() - 1));
                                    } else {
                                        value = std::stoi(token);
                                    }
                                    
                                    Operand op;
                                    op.type = OperandType::IMMEDIATE;
                                    op.value = token;
                                    op.displacement = value;
                                    op.hasDisplacement = true;
                                    op.size = 8;
                                    instr.operands.push_back(op);
                                    currentAddress++; // Byte value
                                } catch (const std::exception& e) {
                                    errors.push_back("Invalid numeric value in DB directive: " + token);
                                }
                            }
                        }
                    }
                    
                    if (!instr.operands.empty()) {
                        parsedInstructions.push_back(instr);
                    }
                    
                    continue;
                }
            }
            
            // Normal instruction handling
            Instruction instr;
            if (parseInstruction(line, instr)) {
                std::cout << "Parsed instruction: " << instr.mnemonic << std::endl;
                if (!instr.operands.empty()) {
                    std::cout << "  Operands: " << instr.operands.size() << std::endl;
                    for (const auto& op : instr.operands) {
                        std::cout << "    Type: " << static_cast<int>(op.type) << ", Value: " << op.value << std::endl;
                    }
                }
                
                // Temporarily estimate instruction size for label addresses
                // This will be refined in the second pass
                int size = 1; // At least 1 byte for opcode
                
                if (!instr.operands.empty()) {
                    for (const auto& op : instr.operands) {
                        if (op.type == OperandType::IMMEDIATE) {
                            size += (op.size == 16) ? 2 : 1;
                        } else if (op.type == OperandType::MEMORY) {
                            size += 1; // ModRM byte
                            if (op.hasDisplacement) {
                                size += (abs(op.displacement) > 255) ? 2 : 1;
                            }
                        }
                    }
                }
                
                parsedInstructions.push_back(instr);
                currentAddress += size;
            }
        }
        
        return true;
    }
    
    uint8_t Assembler::getRegisterCode(const std::string& reg) {
        auto it = registerCodes.find(toUpper(reg));
        if (it != registerCodes.end()) {
            return it->second;
        }
        
        errors.push_back("Unknown register: " + reg);
        return 0;
    }
    
    std::vector<uint8_t> Assembler::encodeModRM(const Operand& dest, const Operand& src) {
        std::vector<uint8_t> result;
        
        // Make local copies of operands so we can modify them
        Operand destCopy = dest;
        Operand srcCopy = src;
        
        // ModRM byte: mod (2 bits) | reg (3 bits) | rm (3 bits)
        uint8_t modRM = 0;
        
        if (dest.type == OperandType::REGISTER && src.type == OperandType::REGISTER) {
            // Register-to-register: mod = 11
            modRM = 0xC0 | (getRegisterCode(src.value) << 3) | getRegisterCode(dest.value);
            result.push_back(modRM);
        } else if (dest.type == OperandType::REGISTER && src.type == OperandType::MEMORY) {
            // Memory-to-register
            
            // Determine addressing mode and mod bits
            uint8_t mod = 0;
            uint8_t rm = 0;
            
            if (srcCopy.baseReg.empty() && srcCopy.indexReg.empty()) {
                // Direct addressing: mod = 00, rm = 110
                mod = 0x00;
                rm = 0x06;
            } else if (!srcCopy.baseReg.empty() && srcCopy.indexReg.empty()) {
                // Register indirect: mod depends on displacement
                rm = getRegisterCode(srcCopy.baseReg);
                
                if (!srcCopy.hasDisplacement) {
                    mod = 0x00;
                } else if (srcCopy.displacement >= -128 && srcCopy.displacement <= 127) {
                    mod = 0x40; // 8-bit displacement
                } else {
                    mod = 0x80; // 16-bit displacement
                }
                
                // Special case: [BP] with no displacement needs 8-bit zero displacement
                if (mod == 0x00 && srcCopy.baseReg == "BP") {
                    mod = 0x40;
                    srcCopy.hasDisplacement = true;
                    srcCopy.displacement = 0;
                }
            } else if (!srcCopy.baseReg.empty() && !srcCopy.indexReg.empty()) {
                // Register + register
                if (srcCopy.baseReg == "BX" && srcCopy.indexReg == "SI") rm = 0x00;
                else if (srcCopy.baseReg == "BX" && srcCopy.indexReg == "DI") rm = 0x01;
                else if (srcCopy.baseReg == "BP" && srcCopy.indexReg == "SI") rm = 0x02;
                else if (srcCopy.baseReg == "BP" && srcCopy.indexReg == "DI") rm = 0x03;
                else if (srcCopy.baseReg == "SI") rm = 0x04;
                else if (srcCopy.baseReg == "DI") rm = 0x05;
                else if (srcCopy.baseReg == "BP") rm = 0x06;
                else if (srcCopy.baseReg == "BX") rm = 0x07;
                
                if (!srcCopy.hasDisplacement) {
                    mod = 0x00;
                } else if (srcCopy.displacement >= -128 && srcCopy.displacement <= 127) {
                    mod = 0x40; // 8-bit displacement
                } else {
                    mod = 0x80; // 16-bit displacement
                }
            }
            
            modRM = (mod & 0xC0) | ((getRegisterCode(dest.value) << 3) & 0x38) | (rm & 0x07);
            result.push_back(modRM);
            
            // Add displacement if needed
            if (mod == 0x00 && rm == 0x06) {
                // Direct addressing needs 16-bit displacement
                result.push_back(srcCopy.displacement & 0xFF);
                result.push_back((srcCopy.displacement >> 8) & 0xFF);
            } else if (mod == 0x40) {
                // 8-bit displacement
                result.push_back(srcCopy.displacement & 0xFF);
            } else if (mod == 0x80) {
                // 16-bit displacement
                result.push_back(srcCopy.displacement & 0xFF);
                result.push_back((srcCopy.displacement >> 8) & 0xFF);
            }
        } else if (dest.type == OperandType::MEMORY && src.type == OperandType::REGISTER) {
            // Register-to-memory (similar to above but operands are swapped)
            uint8_t mod = 0;
            uint8_t rm = 0;
            
            if (destCopy.baseReg.empty() && destCopy.indexReg.empty()) {
                // Direct addressing
                mod = 0x00;
                rm = 0x06;
            } else if (!destCopy.baseReg.empty() && destCopy.indexReg.empty()) {
                // Register indirect
                rm = getRegisterCode(destCopy.baseReg);
                
                if (!destCopy.hasDisplacement) {
                    mod = 0x00;
                } else if (destCopy.displacement >= -128 && destCopy.displacement <= 127) {
                    mod = 0x40;
                } else {
                    mod = 0x80;
                }
                
                // Special case for BP
                if (mod == 0x00 && destCopy.baseReg == "BP") {
                    mod = 0x40;
                    destCopy.hasDisplacement = true;
                    destCopy.displacement = 0;
                }
            } else if (!destCopy.baseReg.empty() && !destCopy.indexReg.empty()) {
                // Register + register
                if (destCopy.baseReg == "BX" && destCopy.indexReg == "SI") rm = 0x00;
                else if (destCopy.baseReg == "BX" && destCopy.indexReg == "DI") rm = 0x01;
                else if (destCopy.baseReg == "BP" && destCopy.indexReg == "SI") rm = 0x02;
                else if (destCopy.baseReg == "BP" && destCopy.indexReg == "DI") rm = 0x03;
                else if (destCopy.baseReg == "SI") rm = 0x04;
                else if (destCopy.baseReg == "DI") rm = 0x05;
                else if (destCopy.baseReg == "BP") rm = 0x06;
                else if (destCopy.baseReg == "BX") rm = 0x07;
                
                if (!destCopy.hasDisplacement) {
                    mod = 0x00;
                } else if (destCopy.displacement >= -128 && destCopy.displacement <= 127) {
                    mod = 0x40;
                } else {
                    mod = 0x80;
                }
            }
            
            modRM = (mod & 0xC0) | ((getRegisterCode(src.value) << 3) & 0x38) | (rm & 0x07);
            result.push_back(modRM);
            
            // Add displacement if needed
            if (mod == 0x00 && rm == 0x06) {
                result.push_back(destCopy.displacement & 0xFF);
                result.push_back((destCopy.displacement >> 8) & 0xFF);
            } else if (mod == 0x40) {
                result.push_back(destCopy.displacement & 0xFF);
            } else if (mod == 0x80) {
                result.push_back(destCopy.displacement & 0xFF);
                result.push_back((destCopy.displacement >> 8) & 0xFF);
            }
        }
        
        return result;
    }
    
    std::vector<uint8_t> Assembler::encodeInstruction(const Instruction& instr) {
        // Add debug output
        std::cout << "Encoding instruction: " << instr.mnemonic << std::endl;
        
        std::vector<uint8_t> result;
        std::string instrType;
        
        // Check if this is a jump instruction
        if (instr.mnemonic == "JMP" || instr.mnemonic == "JE" || instr.mnemonic == "JZ" || 
            instr.mnemonic == "JNE" || instr.mnemonic == "JG" || 
            instr.mnemonic == "JGE" || instr.mnemonic == "JL" || 
            instr.mnemonic == "JLE") {
            std::cout << "  -> Calling encodeJumpInstruction for " << instr.mnemonic << std::endl;
            return encodeJumpInstruction(instr);
        }
        
        // Handle shift/rotate operations directly
        if (instr.mnemonic == "SHL" || instr.mnemonic == "SHR" || 
            instr.mnemonic == "SAL" || instr.mnemonic == "SAR" ||
            instr.mnemonic == "ROL" || instr.mnemonic == "ROR" ||
            instr.mnemonic == "RCL" || instr.mnemonic == "RCR") {
            
            if (instr.operands.size() == 2) {
                const auto& dest = instr.operands[0];
                const auto& count = instr.operands[1];
                
                bool is8Bit = (dest.size == 8);
                bool byCL = (count.type == OperandType::REGISTER && count.value == "CL");
                
                // Choose the opcode based on operand size and count source
                uint8_t opcode;
                if (is8Bit) {
                    opcode = byCL ? 0xD2 : 0xD0; // 8-bit operand, by CL or by 1
                } else {
                    opcode = byCL ? 0xD3 : 0xD1; // 16-bit operand, by CL or by 1
                }
                result.push_back(opcode);
                
                // Encode the ModRM byte
                // mod = 11 (register direct), reg = opcode extension, rm = register code
                uint8_t modRM = 0xC0 | getRegisterCode(dest.value);
                
                // Set the reg field based on the operation
                if (instr.mnemonic == "ROL") {
                    modRM |= 0x00; // reg = 000
                } else if (instr.mnemonic == "ROR") {
                    modRM |= 0x08; // reg = 001
                } else if (instr.mnemonic == "RCL") {
                    modRM |= 0x10; // reg = 010
                } else if (instr.mnemonic == "RCR") {
                    modRM |= 0x18; // reg = 011
                } else if (instr.mnemonic == "SHL" || instr.mnemonic == "SAL") {
                    modRM |= 0x20; // reg = 100
                } else if (instr.mnemonic == "SHR") {
                    modRM |= 0x28; // reg = 101
                } else if (instr.mnemonic == "SAR") {
                    modRM |= 0x38; // reg = 111
                }
                
                // Add ModRM byte
                result.push_back(modRM);
                
                // If the count is an immediate other than 1, we need to handle it specially
                if (count.type == OperandType::IMMEDIATE && count.displacement > 1) {
                    std::cout << "Warning: Shift/rotate by immediate values > 1 not fully supported. Using shift by 1 instead.\n";
                }
                
                return result;
            }
        }
        
        // Special case for OR AL with character 
        if (instr.mnemonic == "OR" && 
            instr.operands.size() == 2 &&
            instr.operands[0].type == OperandType::REGISTER &&
            instr.operands[0].value == "AL") {
            
            // Check if second operand is a char or immediate
            if (instr.operands[1].type == OperandType::IMMEDIATE) {
                // This is OR AL, immediate
                result.push_back(0x0C); // OR AL, imm8 opcode
                result.push_back(instr.operands[1].displacement & 0xFF);
                return result;
            }
        }
        
        // Determine instruction type based on mnemonic and operands
        if (instr.mnemonic == "MOV") {
            if (instr.operands.size() == 2) {
                const auto& dest = instr.operands[0];
                const auto& src = instr.operands[1];
                
                // Debug output
                std::cout << "MOV: dest = " << dest.value << " (type: " << (int)dest.type << ", size: " << dest.size << "), "
                         << "src = " << src.value << " (type: " << (int)src.type << ", displacement: " << src.displacement << ")" << std::endl;
                
                // Handle 8-bit register high/low MOV operations with immediates
                if (dest.type == OperandType::REGISTER && 
                    (dest.value == "AH" || dest.value == "AL" || 
                     dest.value == "BH" || dest.value == "BL" ||
                     dest.value == "CH" || dest.value == "CL" ||
                     dest.value == "DH" || dest.value == "DL") &&
                    src.type == OperandType::IMMEDIATE) {
                    
                    // Use B0-B7 opcodes for 8-bit register immediate moves
                    uint8_t opcode = 0xB0 + getRegisterCode(dest.value);
                    result.push_back(opcode);
                    
                    // Add immediate value (8-bit)
                    result.push_back(src.displacement & 0xFF);
                    
                    return result;
                }
                
                if (dest.type == OperandType::REGISTER && src.type == OperandType::REGISTER) {
                    instrType = "MOV_R_R";
                } else if (dest.type == OperandType::REGISTER && src.type == OperandType::MEMORY) {
                    instrType = "MOV_R_M";
                } else if (dest.type == OperandType::MEMORY && src.type == OperandType::REGISTER) {
                    instrType = "MOV_M_R";
                } else if (dest.type == OperandType::REGISTER && src.type == OperandType::IMMEDIATE) {
                    instrType = "MOV_R_I";
                } else if (dest.type == OperandType::MEMORY && src.type == OperandType::IMMEDIATE) {
                    instrType = "MOV_M_I";
                } else if (dest.type == OperandType::REGISTER && src.type == OperandType::LABEL) {
                    // For MOV register, label - handle like an immediate
                    instrType = "MOV_R_I";
                }
            }
        } else if (instr.mnemonic == "ADD") {
            if (instr.operands.size() == 2) {
                const auto& dest = instr.operands[0];
                const auto& src = instr.operands[1];
                
                // Check if operating on 8-bit registers
                bool is8Bit = false;
                if (dest.type == OperandType::REGISTER && dest.size == 8) {
                    is8Bit = true;
                }
                
                if (dest.type == OperandType::REGISTER && src.type == OperandType::REGISTER) {
                    instrType = is8Bit ? "ADD_R8_R8" : "ADD_R_R";
                } else if (dest.type == OperandType::REGISTER && src.type == OperandType::IMMEDIATE) {
                    // Special cases for AL and AX
                    if (dest.value == "AL" && is8Bit) {
                        instrType = "ADD_AL_I";
                    } else if (dest.value == "AX" && !is8Bit) {
                        instrType = "ADD_AX_I";
                    } else {
                        instrType = is8Bit ? "ADD_R8_I" : "ADD_R_I";
                    }
                }
            }
        } else if (instr.mnemonic == "AND") {
            if (instr.operands.size() == 2) {
                const auto& dest = instr.operands[0];
                const auto& src = instr.operands[1];
                
                bool is8Bit = false;
                if (dest.type == OperandType::REGISTER && dest.size == 8) {
                    is8Bit = true;
                }
                
                if (dest.type == OperandType::REGISTER && src.type == OperandType::REGISTER) {
                    instrType = is8Bit ? "AND_R8_R8" : "AND_R_R";
                } else if (dest.type == OperandType::REGISTER && src.type == OperandType::MEMORY) {
                    instrType = is8Bit ? "AND_R8_M" : "AND_R_M";
                } else if (dest.type == OperandType::MEMORY && src.type == OperandType::REGISTER) {
                    instrType = is8Bit ? "AND_M_R8" : "AND_M_R";
                } else if (dest.type == OperandType::REGISTER && src.type == OperandType::IMMEDIATE) {
                    // Special cases for AL and AX
                    if (dest.value == "AL" && is8Bit) {
                        instrType = "AND_AL_I";
                    } else if (dest.value == "AX" && !is8Bit) {
                        instrType = "AND_AX_I";
                    } else {
                        instrType = is8Bit ? "AND_R8_I" : "AND_R_I";
                    }
                }
            }
        } else if (instr.mnemonic == "OR") {
            if (instr.operands.size() == 2) {
                const auto& dest = instr.operands[0];
                const auto& src = instr.operands[1];
                
                bool is8Bit = false;
                if (dest.type == OperandType::REGISTER && dest.size == 8) {
                    is8Bit = true;
                }
                
                if (dest.type == OperandType::REGISTER && src.type == OperandType::REGISTER) {
                    instrType = is8Bit ? "OR_R8_R8" : "OR_R_R";
                } else if (dest.type == OperandType::REGISTER && src.type == OperandType::MEMORY) {
                    instrType = is8Bit ? "OR_R8_M" : "OR_R_M";
                } else if (dest.type == OperandType::MEMORY && src.type == OperandType::REGISTER) {
                    instrType = is8Bit ? "OR_M_R8" : "OR_M_R";
                } else if (dest.type == OperandType::REGISTER && src.type == OperandType::IMMEDIATE) {
                    // Special cases for AL and AX
                    if (dest.value == "AL" && is8Bit) {
                        instrType = "OR_AL_I";
                    } else if (dest.value == "AX" && !is8Bit) {
                        instrType = "OR_AX_I";
                    } else {
                        instrType = is8Bit ? "OR_R8_I" : "OR_R_I";
                    }
                }
            }
        } else if (instr.mnemonic == "XOR") {
            if (instr.operands.size() == 2) {
                const auto& dest = instr.operands[0];
                const auto& src = instr.operands[1];
                
                bool is8Bit = false;
                if (dest.type == OperandType::REGISTER && dest.size == 8) {
                    is8Bit = true;
                }
                
                if (dest.type == OperandType::REGISTER && src.type == OperandType::REGISTER) {
                    instrType = is8Bit ? "XOR_R8_R8" : "XOR_R_R";
                } else if (dest.type == OperandType::REGISTER && src.type == OperandType::MEMORY) {
                    instrType = is8Bit ? "XOR_R8_M" : "XOR_R_M";
                } else if (dest.type == OperandType::MEMORY && src.type == OperandType::REGISTER) {
                    instrType = is8Bit ? "XOR_M_R8" : "XOR_M_R";
                } else if (dest.type == OperandType::REGISTER && src.type == OperandType::IMMEDIATE) {
                    // Special cases for AL and AX
                    if (dest.value == "AL" && is8Bit) {
                        instrType = "XOR_AL_I";
                    } else if (dest.value == "AX" && !is8Bit) {
                        instrType = "XOR_AX_I";
                    } else {
                        instrType = is8Bit ? "XOR_R8_I" : "XOR_R_I";
                    }
                }
            }
        } else if (instr.mnemonic == "CMP") {
            if (instr.operands.size() == 2) {
                const auto& dest = instr.operands[0];
                const auto& src = instr.operands[1];
                
                bool is8Bit = false;
                if (dest.type == OperandType::REGISTER && dest.size == 8) {
                    is8Bit = true;
                }
                
                if (dest.type == OperandType::REGISTER && src.type == OperandType::REGISTER) {
                    instrType = is8Bit ? "CMP_R8_R8" : "CMP_R_R";
                } else if (dest.type == OperandType::REGISTER && src.type == OperandType::MEMORY) {
                    instrType = is8Bit ? "CMP_R8_M" : "CMP_R_M";
                } else if (dest.type == OperandType::MEMORY && src.type == OperandType::REGISTER) {
                    instrType = "CMP_M_R";
                } else if (dest.type == OperandType::REGISTER && src.type == OperandType::IMMEDIATE) {
                    // Special cases for AL and AX
                    if (dest.value == "AL" && is8Bit) {
                        instrType = "CMP_AL_I";
                    } else if (dest.value == "AX" && !is8Bit) {
                        instrType = "CMP_AX_I";
                    } else {
                        instrType = "CMP_R_I";
                    }
                } else if (dest.type == OperandType::REGISTER && src.type == OperandType::LABEL) {
                    // For CMP register, label - handle like an immediate
                    instrType = "CMP_R_I";
                }
            }
        } else if (instr.mnemonic == "INC" && instr.operands.size() == 1) {
            const auto& dest = instr.operands[0];
            if (dest.type == OperandType::REGISTER) {
                instrType = "INC_R";
            }
        } else if (instr.mnemonic == "DEC" && instr.operands.size() == 1) {
            const auto& dest = instr.operands[0];
            if (dest.type == OperandType::REGISTER) {
                instrType = "DEC_R";
            }
        } else if (instr.mnemonic == "PUSH" && instr.operands.size() == 1) {
            const auto& dest = instr.operands[0];
            if (dest.type == OperandType::REGISTER) {
                instrType = "PUSH_R";
            }
        } else if (instr.mnemonic == "POP" && instr.operands.size() == 1) {
            const auto& dest = instr.operands[0];
            if (dest.type == OperandType::REGISTER) {
                instrType = "POP_R";
            }
        } else if (instr.mnemonic == "CALL" && instr.operands.size() == 1) {
            instrType = "CALL";
        } else if (instr.mnemonic == "RET" && instr.operands.empty()) {
            instrType = "RET";
        } else if (instr.mnemonic == "IRET" && instr.operands.empty()) {
            instrType = "IRET";
        } else if (instr.mnemonic == "INT" && instr.operands.size() == 1) {
            instrType = "INT";
        } else if (instr.mnemonic == "HLT" && instr.operands.empty()) {
            instrType = "HLT";
        } else if (instr.mnemonic == "CLC" && instr.operands.empty()) {
            instrType = "CLC";
        } else if (instr.mnemonic == "STC" && instr.operands.empty()) {
            instrType = "STC";
        } else if (instr.mnemonic == "CMC" && instr.operands.empty()) {
            instrType = "CMC";
        } else if (instr.mnemonic == "CLD" && instr.operands.empty()) {
            instrType = "CLD";
        } else if (instr.mnemonic == "STD" && instr.operands.empty()) {
            instrType = "STD";
        } else if (instr.mnemonic == "CLI" && instr.operands.empty()) {
            instrType = "CLI";
        } else if (instr.mnemonic == "STI" && instr.operands.empty()) {
            instrType = "STI";
        // Handle string operations
        } else if (instr.mnemonic == "MOVSB" && instr.operands.empty()) {
            instrType = "MOVSB";
        } else if (instr.mnemonic == "MOVSW" && instr.operands.empty()) {
            instrType = "MOVSW";
        } else if (instr.mnemonic == "DB") {
            // Handle DB directive - directly add bytes to binary output
            for (const auto& operand : instr.operands) {
                if (operand.type == OperandType::IMMEDIATE) {
                    // Ensure value is between 0-255
                    uint8_t byteValue = operand.displacement & 0xFF;
                    result.push_back(byteValue);
                } else if (operand.type == OperandType::STRING) {
                    // Add each character as a byte
                    for (char c : operand.value) {
                        result.push_back(static_cast<uint8_t>(c));
                    }
                } else if (operand.type == OperandType::LABEL) {
                    // Handle label references in DB directives
                    auto it = labels.find(operand.value);
                    if (it != labels.end()) {
                        uint16_t address = it->second.address;
                        // Add label address as a byte value
                        result.push_back(address & 0xFF);
                        result.push_back((address >> 8) & 0xFF);
                    } else {
                        errors.push_back("Undefined label in DB directive: " + operand.value);
                    }
                } else {
                    errors.push_back("DB directive only accepts immediate values, string literals, or labels");
                    return {};
                }
            }
            return result;
        }
        
        if (instrType.empty()) {
            errors.push_back("Unknown instruction type: " + instr.mnemonic);
            return result;
        }
        
        // Get opcode based on instruction type
        auto it = instructionTable.find(instrType);
        if (it != instructionTable.end()) {
            const auto& opcodes = it->second;
            for (uint8_t opcode : opcodes) {
                result.push_back(opcode);
            }
        } else {
            errors.push_back("Unknown instruction type: " + instrType);
            return result;
        }
        
        // Handle instruction specific encoding
        if (instrType == "MOV_R_R" || instrType == "MOV_R_M" || instrType == "MOV_M_R") {
            auto modRM = encodeModRM(instr.operands[0], instr.operands[1]);
            result.insert(result.end(), modRM.begin(), modRM.end());
        } else if (instrType == "MOV_R_I") {
            // For MOV r16, imm16 we use the register number as part of the opcode
            result[0] += getRegisterCode(instr.operands[0].value);
            
            // Add immediate value
            int value = instr.operands[1].displacement;
            result.push_back(value & 0xFF);
            result.push_back((value >> 8) & 0xFF);
        } else if (instrType == "INC_R" || instrType == "DEC_R" || instrType == "PUSH_R" || instrType == "POP_R") {
            // For INC r16, DEC r16, PUSH r16, POP r16 we use register as part of opcode
            result[0] += getRegisterCode(instr.operands[0].value);
        } else if (instrType == "INT") {
            // For INT imm8
            result.push_back(instr.operands[0].displacement & 0xFF);
        } else if (instrType.find("ADD") == 0) {
            // Handle ADD instructions
            if (instrType == "ADD_R_R" || instrType == "ADD_R8_R8") {
                auto modRM = encodeModRM(instr.operands[0], instr.operands[1]);
                result.insert(result.end(), modRM.begin(), modRM.end());
            } else if (instrType == "ADD_R_I" || instrType == "ADD_R8_I") {
                if (instrType == "ADD_R_I") {
                    // For ADD r16, imm16
                    uint8_t modRM = 0xC0 | (0 << 3) | getRegisterCode(instr.operands[0].value);
                    result.push_back(modRM);
                    result.push_back(instr.operands[1].displacement & 0xFF);
                    result.push_back((instr.operands[1].displacement >> 8) & 0xFF);
                } else {
                    // For ADD r8, imm8
                    uint8_t modRM = 0xC0 | (0 << 3) | getRegisterCode(instr.operands[0].value);
                    result.push_back(modRM);
                    result.push_back(instr.operands[1].displacement & 0xFF);
                }
            } else if (instrType == "ADD_AL_I") {
                // For ADD AL, imm8
                result.push_back(instr.operands[1].displacement & 0xFF);
            } else if (instrType == "ADD_AX_I") {
                // For ADD AX, imm16
                result.push_back(instr.operands[1].displacement & 0xFF);
                result.push_back((instr.operands[1].displacement >> 8) & 0xFF);
            }
        } else if (instrType.find("CMP") == 0) {
            // Handle CMP instructions similar to ADD
            if (instrType == "CMP_R_R" || instrType == "CMP_R8_R8" || 
                instrType == "CMP_R_M" || instrType == "CMP_R8_M" ||
                instrType == "CMP_M_R") {
                auto modRM = encodeModRM(instr.operands[0], instr.operands[1]);
                result.insert(result.end(), modRM.begin(), modRM.end());
            } else if (instrType == "CMP_R_I") {
                // For CMP r16, imm16
                uint8_t modRM = 0xC0 | (7 << 3) | getRegisterCode(instr.operands[0].value);
                result.push_back(modRM);
                result.push_back(instr.operands[1].displacement & 0xFF);
                result.push_back((instr.operands[1].displacement >> 8) & 0xFF);
            } else if (instrType == "CMP_AL_I") {
                // For CMP AL, imm8
                result.push_back(instr.operands[1].displacement & 0xFF);
            } else if (instrType == "CMP_AX_I") {
                // For CMP AX, imm16
                result.push_back(instr.operands[1].displacement & 0xFF);
                result.push_back((instr.operands[1].displacement >> 8) & 0xFF);
            }
        } else if (instrType.find("AND") == 0) {
            // Handle AND instructions
            if (instrType == "AND_R_R" || instrType == "AND_R8_R8" || 
                instrType == "AND_R_M" || instrType == "AND_R8_M" ||
                instrType == "AND_M_R" || instrType == "AND_M_R8") {
                auto modRM = encodeModRM(instr.operands[0], instr.operands[1]);
                result.insert(result.end(), modRM.begin(), modRM.end());
            } else if (instrType == "AND_R_I" || instrType == "AND_R8_I") {
                if (instrType == "AND_R_I") {
                    // For AND r16, imm16
                    uint8_t modRM = 0xC0 | (4 << 3) | getRegisterCode(instr.operands[0].value);
                    result.push_back(modRM);
                    result.push_back(instr.operands[1].displacement & 0xFF);
                    result.push_back((instr.operands[1].displacement >> 8) & 0xFF);
                } else {
                    // For AND r8, imm8
                    uint8_t modRM = 0xC0 | (4 << 3) | getRegisterCode(instr.operands[0].value);
                    result.push_back(modRM);
                    result.push_back(instr.operands[1].displacement & 0xFF);
                }
            } else if (instrType == "AND_AL_I") {
                // For AND AL, imm8
                result.push_back(instr.operands[1].displacement & 0xFF);
            } else if (instrType == "AND_AX_I") {
                // For AND AX, imm16
                result.push_back(instr.operands[1].displacement & 0xFF);
                result.push_back((instr.operands[1].displacement >> 8) & 0xFF);
            }
        } else if (instrType.find("OR") == 0) {
            // Handle OR instructions
            if (instrType == "OR_R_R" || instrType == "OR_R8_R8" || 
                instrType == "OR_R_M" || instrType == "OR_R8_M" ||
                instrType == "OR_M_R" || instrType == "OR_M_R8") {
                auto modRM = encodeModRM(instr.operands[0], instr.operands[1]);
                result.insert(result.end(), modRM.begin(), modRM.end());
            } else if (instrType == "OR_R_I" || instrType == "OR_R8_I") {
                if (instrType == "OR_R_I") {
                    // For OR r16, imm16
                    uint8_t modRM = 0xC0 | (1 << 3) | getRegisterCode(instr.operands[0].value);
                    result.push_back(modRM);
                    result.push_back(instr.operands[1].displacement & 0xFF);
                    result.push_back((instr.operands[1].displacement >> 8) & 0xFF);
                } else {
                    // For OR r8, imm8
                    uint8_t modRM = 0xC0 | (1 << 3) | getRegisterCode(instr.operands[0].value);
                    result.push_back(modRM);
                    result.push_back(instr.operands[1].displacement & 0xFF);
                }
            } else if (instrType == "OR_AL_I") {
                // For OR AL, imm8
                result.push_back(instr.operands[1].displacement & 0xFF);
            } else if (instrType == "OR_AX_I") {
                // For OR AX, imm16
                result.push_back(instr.operands[1].displacement & 0xFF);
                result.push_back((instr.operands[1].displacement >> 8) & 0xFF);
            }
        } else if (instrType.find("XOR") == 0) {
            // Handle XOR instructions
            if (instrType == "XOR_R_R" || instrType == "XOR_R8_R8" || 
                instrType == "XOR_R_M" || instrType == "XOR_R8_M" ||
                instrType == "XOR_M_R" || instrType == "XOR_M_R8") {
                auto modRM = encodeModRM(instr.operands[0], instr.operands[1]);
                result.insert(result.end(), modRM.begin(), modRM.end());
            } else if (instrType == "XOR_R_I" || instrType == "XOR_R8_I") {
                if (instrType == "XOR_R_I") {
                    // For XOR r16, imm16
                    uint8_t modRM = 0xC0 | (6 << 3) | getRegisterCode(instr.operands[0].value);
                    result.push_back(modRM);
                    result.push_back(instr.operands[1].displacement & 0xFF);
                    result.push_back((instr.operands[1].displacement >> 8) & 0xFF);
                } else {
                    // For XOR r8, imm8
                    uint8_t modRM = 0xC0 | (6 << 3) | getRegisterCode(instr.operands[0].value);
                    result.push_back(modRM);
                    result.push_back(instr.operands[1].displacement & 0xFF);
                }
            } else if (instrType == "XOR_AL_I") {
                // For XOR AL, imm8
                result.push_back(instr.operands[1].displacement & 0xFF);
            } else if (instrType == "XOR_AX_I") {
                // For XOR AX, imm16
                result.push_back(instr.operands[1].displacement & 0xFF);
                result.push_back((instr.operands[1].displacement >> 8) & 0xFF);
            }
        } else if (instrType == "CALL") {
            // For CALL with label operand
            if (instr.operands[0].type == OperandType::LABEL) {
                auto it = labels.find(instr.operands[0].value);
                if (it != labels.end()) {
                    // Calculate relative offset (target - current - instruction size)
                    int offset = it->second.address - (instr.address + 3);
                    result.push_back(offset & 0xFF);
                    result.push_back((offset >> 8) & 0xFF);
                } else {
                    errors.push_back("Undefined label: " + instr.operands[0].value);
                }
            }
        }
        
        return result;
    }
    
    void Assembler::secondPass() {
        binaryOutput.clear();
        errors.clear();
        
        // Create binary output from parsed instructions
        for (auto& instr : parsedInstructions) {
            std::cout << "Processing instruction: " << instr.mnemonic << std::endl;
            
            std::vector<uint8_t> encoded = encodeInstruction(instr);
            binaryOutput.insert(binaryOutput.end(), encoded.begin(), encoded.end());
        }
    }
    
    std::vector<uint8_t> Assembler::assemble(const std::string& source) {
        // Split source into lines
        std::vector<std::string> lines;
        std::stringstream ss(source);
        std::string line;
        
        while (std::getline(ss, line)) {
            lines.push_back(line);
        }
        
        // Perform assembly
        firstPass(lines);
        secondPass();
        
        return binaryOutput;
    }
    
    bool Assembler::saveToFile(const std::string& filename) {
        std::ofstream file(filename, std::ios::binary);
        if (!file.is_open()) {
            errors.push_back("Failed to open output file: " + filename);
            return false;
        }
        
        file.write(reinterpret_cast<const char*>(binaryOutput.data()), binaryOutput.size());
        return true;
    }
    
    bool Assembler::assembleFile(const std::string& inputFile, const std::string& outputFile) {
        // Clear any previous data
        binaryOutput.clear();
        parsedInstructions.clear();
        labels.clear();
        errors.clear();

        // Read source file
        std::ifstream file(inputFile);
        if (!file.is_open()) {
            errors.push_back("Failed to open input file: " + inputFile);
            return false;
        }
        
        // Process each line, with special handling for problematic DB directives
        std::vector<std::string> processedLines;
        std::string line;
        while (std::getline(file, line)) {
            // Check for DB directives with labels (problem cases)
            std::regex dbPattern(R"(^(\w+)\s+DB\s+['"].*$)");
            std::smatch matches;
            if (std::regex_search(line, matches, dbPattern) && matches.size() > 1) {
                // Found a potential DB directive with a label
                std::string labelName = matches[1].str();
                
                // Add a label definition line first
                processedLines.push_back(labelName + ":");
                
                // Then add the DB directive separately
                std::string rest = line.substr(labelName.length());
                processedLines.push_back("DB" + rest);
            } else {
                // Normal line, just add it
                processedLines.push_back(line);
            }
        }
        file.close(); // Close file immediately after reading
        
        // Now pass the processed lines to the assembler
        firstPass(processedLines);
        secondPass();
        
        // Save result
        if (!errors.empty()) {
            std::cout << "Assembly failed with errors:" << std::endl;
            for (const auto& error : errors) {
                std::cout << "  - " << error << std::endl;
            }
            return false;
        }
        
        bool result = saveToFile(outputFile);
        
        // Clean up to free memory
        binaryOutput.clear();
        parsedInstructions.clear();
        
        return result;
    }
    
    std::vector<std::string> Assembler::getErrors() const {
        return errors;
    }

    std::vector<uint8_t> Assembler::encodeJumpInstruction(const Instruction& instr) {
        std::vector<uint8_t> result;
        
        // Get the opcode from the instruction table
        if (instructionTable.find(instr.mnemonic) == instructionTable.end()) {
            errors.push_back("Unknown jump instruction: " + instr.mnemonic);
            return {};
        }
        
        auto opcodes = instructionTable[instr.mnemonic];
        
        if (instr.operands.size() != 1) {
            errors.push_back("Jump instructions require exactly one operand");
            return {};
        }
        
        // Add the opcode
        result.push_back(opcodes[0]);
        
        const auto& target = instr.operands[0];
        if (target.type == OperandType::LABEL) {
            // Calculate offset for the jump
            if (labels.find(target.value) == labels.end()) {
                errors.push_back("Undefined label: " + target.value);
                return {};
            }
            
            int32_t targetAddress = labels[target.value].address;
            int32_t currentAddress = instr.address + 2; // Address after this instruction (1 byte opcode + 1 byte offset)
            int32_t offset = targetAddress - currentAddress;
            
            // Check if offset is in range for short jump
            if (offset < -128 || offset > 127) {
                if (instr.mnemonic == "JMP") {
                    // For JMP, we can use the FAR version
                    result.clear();
                    result.push_back(instructionTable["JMP_FAR"][0]);
                    // 16-bit offset
                    result.push_back(offset & 0xFF);
                    result.push_back((offset >> 8) & 0xFF);
                } else {
                    errors.push_back("Jump offset too large for conditional jump: " + std::to_string(offset));
                    return {};
                }
            } else {
                // 8-bit offset
                result.push_back(static_cast<uint8_t>(offset));
            }
        } else {
            errors.push_back("Jump target must be a label");
            return {};
        }
        
        return result;
    }

} // namespace Assembler 