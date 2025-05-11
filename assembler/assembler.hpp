#ifndef ASSEMBLER_HPP
#define ASSEMBLER_HPP

#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <fstream>
#include <memory>
#include <cstdint>
#include <stdexcept>

namespace Assembler {

    enum class OperandType {
        NONE,
        REGISTER,
        IMMEDIATE,
        MEMORY,
        LABEL,
        STRING
    };

    struct Operand {
        OperandType type = OperandType::NONE;
        std::string value;
        int size = 0;  // Size in bits (8, 16, etc.)
        
        // For memory operands
        std::string baseReg;
        std::string indexReg;
        int displacement = 0;
        bool hasDisplacement = false;
    };

    struct Instruction {
        std::string mnemonic;
        std::vector<Operand> operands;
        uint32_t address = 0;
        std::vector<uint8_t> machineCode;
    };

    class Label {
    public:
        std::string name;
        uint32_t address;
        bool defined = false;
        
        Label() : name(""), address(0), defined(false) {}
        Label(const std::string& name) : name(name), address(0) {}
        Label(const std::string& name, uint32_t addr) : name(name), address(addr), defined(true) {}
    };

    class Assembler {
    private:
        // Instruction tables for lookups
        std::unordered_map<std::string, std::vector<uint8_t>> instructionTable;
        
        // Register encodings
        std::unordered_map<std::string, uint8_t> registerCodes;
        
        // Symbol table for labels
        std::map<std::string, Label> labels;
        
        // Storage for generated code and parsed instructions
        std::vector<uint8_t> binaryOutput;
        std::vector<Instruction> parsedInstructions;
        
        // Current address during assembly
        uint32_t currentAddress = 0;
        
        // Helper methods
        void initializeInstructionTable();
        void initializeRegisterTable();
        bool parseInstruction(const std::string& line, Instruction& instr);
        bool parseOperand(const std::string& operandStr, Operand& operand);
        std::vector<uint8_t> encodeInstruction(const Instruction& instr);
        std::vector<uint8_t> encodeModRM(const Operand& dest, const Operand& src);
        std::vector<uint8_t> encodeJumpInstruction(const Instruction& instr);
        uint8_t getRegisterCode(const std::string& reg);
        bool isValidLabel(const std::string& label);
        
        // First pass: parse all instructions and build label table
        bool firstPass(const std::vector<std::string>& lines);
        
        // Second pass: resolve labels and generate binary code
        void secondPass();
        
    public:
        Assembler();
        
        // Assemble source code into binary
        std::vector<uint8_t> assemble(const std::string& source);
        
        // Save binary output to file
        bool saveToFile(const std::string& filename);
        
        // Load assembly from file and assemble it
        bool assembleFile(const std::string& inputFile, const std::string& outputFile);
        
        // Get error messages
        std::vector<std::string> getErrors() const;
        
    private:
        std::vector<std::string> errors;
    };

} // namespace Assembler

#endif // ASSEMBLER_HPP 