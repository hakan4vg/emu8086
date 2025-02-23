#include <iostream>
#include <fstream>
#include <vector>
#include <stdexcept>
#include "cpu/registers.hpp"
#include "cpu/flags.hpp"
#include "cpu/memory.hpp"
#include "cpu/instructions.hpp"

// Load file into vector
std::vector<uint8_t> loadBinaryFile(const std::string &filename) {
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + filename);
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<uint8_t> buffer(size);
    if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
        throw std::runtime_error("Failed to read file: " + filename);
    }

    return buffer;
}

int main(int argc, char* argv[]) {
    try {
        // Basic argument handling: e.g., ./emu8086 run program.bin
        if (argc < 3) {
            std::cerr << "Usage: " << argv[0] << " run <filename>" << std::endl;
            return 1;
        }

        std::string command = argv[1];
        std::string filename = argv[2];

        if (command == "run") {
            CPU::Memory memory;
            CPU::Registers regs; 
            CPU::Flags flags;
            CPU::Instructions instructions(memory, regs, flags);


            std::vector<uint8_t> binData = loadBinaryFile(filename);

            uint32_t loadAddress = 0; 
            for (size_t i = 0; i < binData.size(); i++) {
                if (loadAddress + i >= CPU::Memory::MEMORY_SIZE) {
                    throw std::runtime_error("Program too large to fit in emulated memory.");
                }
                memory.writeByte(loadAddress + i, binData[i]);
            }

            regs.CS = 0x07C0;  // code segment
            regs.IP = 0x0000;  // instruction pointer where we loaded the code
            loadAddress = 0x07C00;


            bool running = true;
            while (running) {
                try {
                    // instructions.executeNext() will fetch the next opcode at CS:IP
                    // and execute it. If the opcode is unknown, it throws an exception.
                    instructions.executeNext();
                    if (instructions.isHalted()) {
                        std::cout <<"HLT encountered. Stopping emulation.\n";
                        running = false;
                    }
                }
                catch (const std::runtime_error &e) {
                    std::cerr << "Emulation error: " << e.what() << std::endl;
                    running = false;
                }
            }

            std::cout << "Emulation finished. Final register state:\n";
            std::cout << "AX=" << std::hex << regs.AX.value 
                      << " BX=" << regs.BX.value 
                      << " CX=" << regs.CX.value 
                      << " DX=" << regs.DX.value << std::endl;

            flags.dumpFlags();
        }
        else {
            std::cerr << "Unknown command: " << command << std::endl;
            return 1;
        }

    } catch (const std::exception &ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
        return 1;
    }

    return 0;
}
