#include <iostream>
#include <fstream>
#include <vector>
#include <stdexcept>
#include <string>
#include "cpu/registers.hpp"
#include "cpu/flags.hpp"
#include "cpu/memory.hpp"
#include "cpu/instructions.hpp"
#include "io/io.hpp"
#include "assembler/assembler.hpp"
#include "disassembler/disassembler.hpp"
#include "cpu/cpu.hpp"

// Print usage information
void printUsage(const char* programName) {
    std::cerr << "Usage: " << programName << " [options]\n\n"
              << "Options:\n"
              << "  -a <file>    Assemble the specified file\n"
              << "  -i <file>    Input assembly file (default: examples/simple.asm)\n"
              << "  -o <file>    Output binary file (default: examples/output/simple.bin)\n"
              << "  -d           Disassemble the binary file\n"
              << "  -e           Execute the binary file (default)\n"
              << "  -h, --help   Show help message\n"
              << std::endl;
}

// Load file into vector
std::vector<uint8_t> loadBinaryFile(const std::string &filename) {
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + filename);
    }

    std::streamsize size = file.tellg();
    if (size <= 0 || size > 1024 * 1024) { // Sanity check: max 1MB file
        throw std::runtime_error("File size invalid or too large: " + std::to_string(size) + " bytes");
    }
    
    file.seekg(0, std::ios::beg);

    std::vector<uint8_t> buffer(size);
    if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
        throw std::runtime_error("Failed to read file: " + filename);
    }

    return buffer;
}

// Helper function to read entire file into string
std::string readFileIntoString(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + filename);
    }
    
    return std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
}

// Helper function to read binary file into vector
std::vector<uint8_t> readBinaryFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open binary file: " + filename);
    }
    
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    
    std::vector<uint8_t> buffer(size);
    if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
        throw std::runtime_error("Failed to read binary file: " + filename);
    }
    
    return buffer;
}

int main(int argc, char** argv) {
    try {
        std::string inputFile = "examples/simple.asm";
        std::string outputFile = "examples/output/simple.bin";
        bool disassembleMode = false;
        bool executeMode = true;
        bool assembleMode = false;
        
        // Parse command line arguments
        for (int i = 1; i < argc; i++) {
            std::string arg = argv[i];
            
            if (arg == "-a" && i + 1 < argc) {
                inputFile = argv[++i];
                assembleMode = true;
            } else if (arg == "-i" && i + 1 < argc) {
                inputFile = argv[++i];
                assembleMode = true;
            } else if (arg == "-o" && i + 1 < argc) {
                outputFile = argv[++i];
            } else if (arg == "-d") {
                disassembleMode = true;
            } else if (arg == "-e") {
                if (i + 1 < argc && (argv[i+1][0] != '-')) {
                    // Check if there's a value after -e
                    std::string value = argv[++i];
                    if (value == "false") {
                        executeMode = false;
                    }
                } else {
                    executeMode = true;
                }
            } else if (arg == "-h" || arg == "--help") {
                printUsage(argv[0]);
                return 0;
            } else {
                // If a filename is provided without flags, treat it as input file
                inputFile = arg;
                assembleMode = true;
            }
        }
        
        // If no arguments were provided, use the default
        if (argc == 1) {
            inputFile = "examples/simple.asm";
            assembleMode = true;
        }
        
        // Get filename for output if not specified
        if (inputFile != "examples/simple.asm" && outputFile == "examples/output/simple.bin") {
            // Generate output filename based on input filename
            size_t lastDot = inputFile.find_last_of('.');
            size_t lastSlash = inputFile.find_last_of('/');
            
            if (lastDot != std::string::npos) {
                std::string baseName;
                if (lastSlash != std::string::npos && lastSlash < lastDot) {
                    baseName = inputFile.substr(lastSlash + 1, lastDot - lastSlash - 1);
                } else {
                    baseName = inputFile.substr(0, lastDot);
                }
                
                outputFile = "examples/output/" + baseName + ".bin";
            }
        }
        
        // Assemble the file
        std::cout << "Assembling file: " << inputFile << std::endl;
        Assembler::Assembler assembler;
        
        bool assembled = assembler.assembleFile(inputFile, outputFile);
        if (!assembled) {
            std::cout << "Assembly failed with errors:" << std::endl;
            for (const auto& error : assembler.getErrors()) {
                std::cout << "  - " << error << std::endl;
            }
            return 1;
        }
        
        std::cout << "Assembly successful. Output written to: " << outputFile << std::endl;
        
        // Disassemble if requested
        if (disassembleMode) {
            std::cout << "\nDisassembling binary file: " << outputFile << std::endl;
            Disassembler::Disassembler disassembler;
            
            // Set base address to match 8086 boot sector
            disassembler.setBaseAddress(0x7C00);
            
            if (!disassembler.loadBinaryFile(outputFile)) {
                std::cerr << "Failed to load binary file for disassembly" << std::endl;
                return 1;
            }
            
            if (!disassembler.disassemble()) {
                std::cerr << "Disassembly failed" << std::endl;
                return 1;
            }
            
            std::cout << "\nDisassembly:\n" << disassembler.toString() << std::endl;
        }
        
        // Execute if requested
        if (executeMode) {
            std::cout << "\nExecuting binary file: " << outputFile << std::endl;
            
            // Load the binary file
            std::vector<uint8_t> binary = readBinaryFile(outputFile);
            
            // Create and initialize CPU
            CPU::CPU cpu;
            
            // Load binary into memory at the boot address (0x7C00)
            cpu.loadBootBinary(binary);
            
            // Execute CPU
            try {
                std::cout << "\nExecution output:\n";
                cpu.run();
                std::cout << "\nExecution completed successfully" << std::endl;
            } catch (const std::exception& e) {
                std::cerr << "\nExecution error: " << e.what() << std::endl;
                return 1;
            }
        }
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
