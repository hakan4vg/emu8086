#ifndef INSTRUCTIONS_HPP
#define INSTRUCTIONS_HPP

#include <cstdint>
#include <functional>
#include <unordered_map>
#include <stdexcept>
#include <string>
#include "memory.hpp"
#include "registers.hpp"
#include "flags.hpp"

namespace CPU {

    class Instructions {
    public:
        Instructions(Memory &mem, Registers &reg, Flags &flg);

        // Fetch and execute one instruction at CS:IP
        void executeNext();

        bool isHalted() const { return halted; }

    private:
        // References to CPU components
        Memory      &memory;
        Registers   &registers;
        Flags       &flags;

        bool halted = false;

        // Opcode table: opcode -> handler function
        using InstructionHandler = std::function<void()>;
        std::unordered_map<uint8_t, InstructionHandler> opcodeTable;

        //----------------------------------------------------------------------
        // Internal helper methods
        //----------------------------------------------------------------------
        // Fetch next byte or word from memory and advance IP
        uint8_t  fetchByte();
        uint16_t fetchWord();

        // Decode the opcode from memory, look up and call the handler
        void decodeAndExecute(uint8_t opcode);

        // Return pointer to a 16-bit register based on reg index
        uint16_t* getRegisterReference(uint8_t reg);

        // Return pointer into memory for the given addressing mode
        // (mod r/m) from an x86 ModR/M byte
        uint16_t* getMemoryReference(uint8_t mod, uint8_t rm);

        // Flag-setting helpers
        // For arithmetic ops (ADD, SUB, etc.)
        void setArithmeticFlags(uint32_t result, uint16_t dest, uint16_t src);

        //----------------------------------------------------------------------
        // Instruction handlers
        //----------------------------------------------------------------------
        // Move
        void handleMOV();

        // Arithmetic
        void handleADD();
        void handleSUB();
        void handleCMP();
        void handleINC();
        void handleDEC();

        // Logical
        void handleAND();
        void handleOR();
        void handleXOR();
        void handleNOT();


        // Shift/Rotate
        void handleSHL();
        void handleSHR();

        // Jump / Branch
        void handleJMP();
        void handleJE();
        void handleJNE();
        void handleJG();
        void handleJGE();
        void handleJL();
        void handleJLE();

        // Call/Return/Stack
        void handlePUSH();
        void handlePOP();
        void handleCALL();
        void handleRET();

        // Interrupt / Halt
        void handleINT();
        void handleHLT();

        //----------------------------------------------------------------------
        // F6 / F7 handlers for 8-bit and 16-bit ops
        //----------------------------------------------------------------------
        void handleF6(); // 0xF6 => 8-bit (TEST, NOT, NEG, MUL, IMUL, DIV, IDIV)
        void handleF7(); // 0xF7 => 16-bit (TEST, NOT, NEG, MUL, IMUL, DIV, IDIV)

        // 8-bit sub-handlers
        void handleTest8(uint8_t modrm);
        void handleNot8(uint8_t modrm);
        void handleNeg8(uint8_t modrm);
        void handleMul8(uint8_t modrm);
        void handleIMul8(uint8_t modrm);
        void handleDiv8(uint8_t modrm);
        void handleIDiv8(uint8_t modrm);

        // 16-bit sub-handlers
        void handleTest16(uint8_t modrm);
        void handleNot16(uint8_t modrm);
        void handleNeg16(uint8_t modrm);
        void handleMul16(uint8_t modrm);
        void handleIMul16(uint8_t modrm);
        void handleDiv16(uint8_t modrm);
        void handleIDiv16(uint8_t modrm);
    };

} 

#endif // INSTRUCTIONS_HPP
