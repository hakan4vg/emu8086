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

        uint16_t* getMemoryReference(uint8_t mod, uint8_t rm);
        uint8_t fetchByte();
        uint16_t fetchWord();
        void decodeAndExecute(uint8_t opcode);
        uint16_t* getRegisterReference(uint8_t reg);
        void setArithmeticFlags(uint16_t result, uint16_t dest, uint16_t src); //Set flags for arithmetic operations
        void setArithmeticFlags(uint16_t dest, uint16_t src); //Set flags for logical operations
        void handleF6(); //Multiple instructions share the same opcode
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
        void handlePUSH();
        void handlePOP();
        void handleCALL();
        void handleRET();
        
    public:
        Instructions(Memory &mem, Registers &reg, Flags &flg);

        void executeNext();
    };

}


#endif //INSTRUCTIONS_HPP
