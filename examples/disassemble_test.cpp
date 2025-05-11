#include "../disassembler/disassembler.hpp"
#include <iostream>
#include <fstream>
#include <vector>

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <input_binary>" << std::endl;
        return 1;
    }

    // Load the binary file
    std::ifstream input(argv[1], std::ios::binary);
    if (!input) {
        std::cerr << "Error: Could not open input file " << argv[1] << std::endl;
        return 1;
    }

    // Read the binary data
    std::vector<uint8_t> binaryData((std::istreambuf_iterator<char>(input)), 
                                     std::istreambuf_iterator<char>());
    input.close();

    // Create and initialize the disassembler
    Disassembler::Disassembler disassembler;
    disassembler.setBaseAddress(0x0100); // Standard starting address for COM files
    disassembler.loadBinaryData(binaryData);

    // Disassemble
    if (disassembler.disassemble()) {
        // Print the disassembly
        std::cout << disassembler.toString() << std::endl;

        // Save to file if an output file is specified
        if (argc > 2) {
            if (disassembler.saveToFile(argv[2])) {
                std::cout << "Disassembly saved to " << argv[2] << std::endl;
            } else {
                std::cerr << "Error: Could not save disassembly to " << argv[2] << std::endl;
                return 1;
            }
        }
    } else {
        std::cerr << "Error: Disassembly failed" << std::endl;
        return 1;
    }

    return 0;
} 