//
// Created by Hakan Avgın on 21.12.2024.
//

#ifndef CPU_HPP
#define CPU_HPP

#include "memory.hpp"
#include "registers.hpp"
#include "flags.hpp"
#include "instructions.hpp"
#include "../io/io.hpp"

namespace CPU {

    class CPU {
    private:
        Memory memory;
        Registers registers;
        Flags flags;
        IO::IOController ioController;
        Instructions instructions;
        
        // Cycle counting
        uint64_t total_cycles;
        uint64_t instruction_count;

    public:
        CPU() : memory(), registers(), flags(), ioController(), instructions(memory, registers, flags, ioController),
                total_cycles(0), instruction_count(0) {
            // Default initialization
        }

        // Load binary into memory at specific address
        void loadBinary(const std::vector<uint8_t>& binary, uint32_t address) {
            // Convert to physical address, by default use CS:IP for 8086 boot loading
            uint32_t physAddr = address;
            
            for (size_t i = 0; i < binary.size(); i++) {
                memory.writeByte(physAddr + i, binary[i]);
            }
        }

        // Load binary into memory at the standard boot address (0x7C00)
        void loadBootBinary(const std::vector<uint8_t>& binary) {
            registers.CS = 0x0000;
            registers.IP = 0x7C00;
            loadBinary(binary, 0x7C00);
        }

        // Execute a single instruction
        void executeInstruction() {
            uint32_t cycles = instructions.executeNext();
            total_cycles += cycles;
            instruction_count++;
        }

        // Run the CPU until HLT or error
        void run() {
            while (!instructions.isHalted()) {
                executeInstruction();
            }
            
            // Display cycle information when execution ends
            std::cout << "Execution completed:" << std::endl;
            std::cout << "Total instructions executed: " << instruction_count << std::endl;
            std::cout << "Total cycles: " << total_cycles << std::endl;
            if (instruction_count > 0) {
                std::cout << "Average cycles per instruction: " 
                          << static_cast<double>(total_cycles) / instruction_count << std::endl;
            }
        }

        // Get cycle and instruction count
        uint64_t getTotalCycles() const { return total_cycles; }
        uint64_t getInstructionCount() const { return instruction_count; }

        // Debug methods
        void dumpRegisters() const {
            std::cout << "AX: " << std::hex << registers.AX.value << " BX: " << registers.BX.value
                      << " CX: " << registers.CX.value << " DX: " << registers.DX.value << std::endl;
            std::cout << "SI: " << registers.SI << " DI: " << registers.DI 
                      << " BP: " << registers.BP << " SP: " << registers.SP << std::endl;
            std::cout << "CS: " << registers.CS << " DS: " << registers.DS 
                      << " ES: " << registers.ES << " SS: " << registers.SS << std::endl;
            std::cout << "IP: " << registers.IP << std::endl;
        }

        void dumpMemory(uint32_t start, uint32_t size) const {
            memory.dumpMemory(start, start + size);
        }

        // Reset the CPU
        void reset() {
            // Reset all registers and flags
            registers = Registers();
            flags = Flags();
            
            // Reset memory
            for (size_t i = 0; i < Memory::MEMORY_SIZE; i++) {
                memory.writeByte(i, 0);
            }
            
            // Reset cycle counting
            total_cycles = 0;
            instruction_count = 0;
            
            // We can't reassign instructions due to reference members,
            // so we'll ensure the CPU is not halted
            instructions.resetHaltState();
        }
    };

} // namespace CPU

#endif //CPU_HPP
