#ifndef MEMORY_HPP
#define MEMORY_HPP

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <vector>

namespace CPU {
    class Memory {
    private:
        std::vector<uint8_t> memory;
    public:
        static constexpr size_t MEMORY_SIZE = 1 << 20; // 1 MB

        Memory() : memory(MEMORY_SIZE){} // Constructor

        uint8_t readByte(uint32_t address) const;
        uint16_t readWord(uint32_t address) const;

        void writeByte(uint32_t addrses, uint8_t value);
        void writeWord(uint32_t address, uint16_t value);

        uint32_t calculatePhysicalAddress(uint16_t segment, uint16_t offset) const;

        void dumpMemory(uint32_t startAddreses, uint32_t endAddress) const;
    };
}

#endif // MEMORY_HPP