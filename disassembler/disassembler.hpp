#ifndef DISASSEMBLER_HPP
#define DISASSEMBLER_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <sstream>
#include <fstream>
#include <cstdint>

namespace Disassembler {

    // Structure to represent a disassembled instruction
    struct Instruction {
        uint32_t address;
        std::vector<uint8_t> bytes;
        std::string mnemonic;
        std::string operands;
        
        std::string toString() const;
    };
    
    // Class to handle instruction decoding and disassembly
    class Disassembler {
    private:
        // Maps for opcode lookups
        std::unordered_map<uint8_t, std::string> singleByteOpcodes;
        std::unordered_map<uint8_t, std::string> twoByteOpcodes;
        
        // Register name lookups
        std::unordered_map<uint8_t, std::string> registers8;
        std::unordered_map<uint8_t, std::string> registers16;
        std::unordered_map<uint8_t, std::string> segmentRegisters;
        
        // Binary data being disassembled
        std::vector<uint8_t> binaryData;
        
        // Current position in the binary data
        size_t position = 0;
        
        // Base address for disassembly
        uint32_t baseAddress = 0;
        
        // Disassembled instructions
        std::vector<Instruction> instructions;
        
        // Helper methods
        void initializeOpcodeTables();
        void initializeRegisterTables();
        
        // Decode a single instruction at current position
        bool decodeInstruction(Instruction& instr);
        
        // Handle specific instruction types
        bool handleMOV(Instruction& instr, uint8_t opcode);
        bool handleArithmetic(Instruction& instr, uint8_t opcode);
        bool handleJump(Instruction& instr, uint8_t opcode);
        bool handleStack(Instruction& instr, uint8_t opcode);
        bool handleString(Instruction& instr, uint8_t opcode);
        bool handleMisc(Instruction& instr, uint8_t opcode);
        bool handleROL(Instruction& instr, uint8_t opcode);
        bool handleGroup1(Instruction& instr, uint8_t opcode);
        
        // ModRM byte processing
        std::string decodeModRM(uint8_t modrm, bool is16Bit = true);
        std::string getRegisterName(uint8_t reg, bool is16Bit = true);
        std::string getEffectiveAddress(uint8_t mod, uint8_t rm);
        
        // Reading data
        uint8_t readByte();
        uint16_t readWord();
        int8_t readSignedByte();
        int16_t readSignedWord();
        
    public:
        Disassembler();
        
        // Set the base address for disassembly
        void setBaseAddress(uint32_t addr);
        
        // Load binary data from file
        bool loadBinaryFile(const std::string& filename);
        
        // Load binary data from memory
        void loadBinaryData(const std::vector<uint8_t>& data);
        
        // Disassemble loaded binary data
        bool disassemble();
        
        // Get disassembled instructions
        const std::vector<Instruction>& getInstructions() const;
        
        // Save disassembly to a text file
        bool saveToFile(const std::string& filename) const;
        
        // Get disassembly as a string
        std::string toString() const;
        
        // Disassemble a file and save the results
        bool disassembleFile(const std::string& inputFile, const std::string& outputFile);
    };

} // namespace Disassembler

#endif // DISASSEMBLER_HPP 