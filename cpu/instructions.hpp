//
// Created by Hakan Avgın on 21.12.2024.
//

#ifndef INSTRUCTIONS_HPP
#define INSTRUCTIONS_HPP

#include "memory.hpp"
#include "registers.hpp"
#include "flags.hpp"
#include <cstdint>
#include <functional>
#include <unordered_map>
#include <string>

namespace CPU {
    class Instructions {
    private:
        Memory &memory;
        Registers &registers;
        Flags &flags;

        //Opcode table mapping
        using InstructionHandler = std::function<void()>; //Function pointer type
        std::unordered_map<uint8_t, InstructionHandler> opcodeTable; //Opcode -> Handler

        uint8_t fetchByte();
        uint16_t fetchWord();
        void decodeAndExecute(uint8_t opcode);
        uint16_t* getRegisterReference(uint8_t reg);
        void handleMOV();
        void handleADD();
        void handleSUB();
        void handleMUL();
        void handleDIV();
        void handleINC();
        void handleDEC();
        void handleCMP();
        void handleJMP();
        void handleJE();
        void handleJNE();
        void handleJG();
        void handleJGE();
        void handleJL();
        void handleJLE();
        void handleINT();
        void handleHLT();
        void handleAND();
        void handleOR();
        void handleXOR();
        void handleNOT();
        void handleSHL();
        void handleSHR();
    public:
        Instructions(Memory &mem, Registers &reg, Flags &flg);

        void executeNext();
    };

}


#endif //INSTRUCTIONS_HPP
