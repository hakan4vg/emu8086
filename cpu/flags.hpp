//
// Created by Hakan Avgın on 21.12.2024.
//

#ifndef FLAGS_HPP
#define FLAGS_HPP

#include <cstdint>
#include <iostream>

namespace CPU {
    class Flags {
    private:
        uint16_t flags;
    public:
        Flags(): flags(0){}

        void reset(){flags = 0;}

        void setFlag(uint16_t flagMask, bool value) {
            if(value)
                flags |= flagMask;
            else
                flags &= ~flagMask; //bitwise not, clear the bit
        }

        bool getFlag(uint16_t flagMask) const {
            return (flags & flagMask) != 0;
        }

        void dumpFlags() const {
            std::cout << "Flags: " << std::hex << flags << std::endl;
        }
    };

    enum FLAGS {
        CF = 1 << 0, //Carry
        PF = 1 << 2, //Parity - Bitwise
        AF = 1 << 4, //Auxillary Carry Indicates a carry/borrow between the lower nibble (4 bits) during BCD (Binary-Coded Decimal) operations.
        ZF = 1 << 6, //Zero
        SF = 1 << 7, //Sign - Only necessary when distinguishing between signed and unsigned numbers /TODO
        TF = 1 << 8, //Trap - Debugging
        IF = 1 << 9, //Interrupt Enable
        DF = 1 << 10, //Direction
        OF = 1 << 11, //Overflow
    };
}


#endif //FLAGS_HPP
