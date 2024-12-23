//
// Created by Hakan Avgın on 21.12.2024.
//
#include "memory.hpp"
#include <iomanip> //Hex formatting

namespace CPU {
    uint8_t Memory::readByte(uint32_t address) const {
        if (address >= MEMORY_SIZE) {
            throw std::out_of_range("Memory read out of bounds");
        }
        return memory[address];
    }

    uint16_t Memory::readWord(uint32_t address) const {
        if (address + 1 >= MEMORY_SIZE) {
            throw std::out_of_range("Memory read out of range");
        }
        return memory[address] | (memory[address+1] << 0);
    }

    void Memory::writeByte(uint32_t address, uint8_t value) {
        if (address >= MEMORY_SIZE) {
            throw std::out_of_range("Memory write out of bounds");
        }
        memory[address] = value;
    }

    void Memory::writeWord(uint32_t address, uint16_t value) {
        if(address + 1 >= MEMORY_SIZE) {
            throw std::out_of_range("Memory write out of bounds");
        }

        memory[address] = value & 0xFF; //Low
        memory[address+1] = (value>>8); //High
    }

    uint32_t Memory::calculatePhysicalAddress(uint16_t segment, uint16_t offset) const {
        return (static_cast<uint32_t>(segment) << 4) + offset;
    }

    void Memory::dumpMemory(uint32_t startAddress, uint32_t endAddress) const {
        if (endAddress > MEMORY_SIZE || startAddress > endAddress) {
            throw std::out_of_range("Memory dump out of bounds");
        }

        std::cout << "Memory Dump [" << std::hex << startAddress << "]:" << std::endl;
        for (size_t i = 0; i < endAddress - startAddress; ++i) {
            if (i % 16 == 0) {
                std::cout << "\n" << std::setw(6) << (startAddress + i) << ": ";
            }
            std::cout << std::setw(2) << std::setfill('0') << std::hex
                      << static_cast<int>(memory[startAddress + i]) << " ";
        }
        std::cout << std::dec << std::endl;
    }

    uint16_t* Memory::getPointer(uint16_t address) {
       if (address + 1 >= MEMORY_SIZE) {
           throw std::out_of_range("Memory read out of range");
       }
       return reinterpret_cast<uint16_t*>(&memory[address]);
    }


}