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
#include "../io/io.hpp"

namespace CPU {

    class Instructions {
    public:
        Instructions(Memory &mem, Registers &reg, Flags &flg, IO::IOController &ioController);

        // Fetch and execute one instruction at CS:IP - now returns cycle count
        uint32_t executeNext();

        // Check if CPU is halted
        bool isHalted() const { return halted; }
        
        // Reset the halt state (used when resetting the CPU)
        void resetHaltState() { halted = false; }

    private:
        // References to CPU components
        Memory      &memory;
        Registers   &registers;
        Flags       &flags;
        IO::IOController &io;

        bool halted = false;

        // Cycle counts for different instruction groups (based on 8086 documentation)
        struct CycleCounts {
            const uint32_t MOV_REG_REG = 2;      // MOV register to register
            const uint32_t MOV_MEM_REG = 9;      // MOV memory to register
            const uint32_t MOV_REG_MEM = 9;      // MOV register to memory
            const uint32_t MOV_IMM_REG = 4;      // MOV immediate to register
            const uint32_t MOV_IMM_MEM = 10;     // MOV immediate to memory
            
            const uint32_t PUSH_REG = 11;        // PUSH register
            const uint32_t POP_REG = 10;         // POP register
            
            const uint32_t ALU_REG_REG = 3;      // ADD/SUB/CMP etc. register to register
            const uint32_t ALU_MEM_REG = 9;      // ALU memory to register
            const uint32_t ALU_REG_MEM = 16;     // ALU register to memory
            const uint32_t ALU_IMM_REG = 4;      // ALU immediate to register
            const uint32_t ALU_IMM_MEM = 17;     // ALU immediate to memory
            
            const uint32_t INC_REG = 3;          // INC/DEC register
            const uint32_t INC_MEM = 15;         // INC/DEC memory
            
            const uint32_t JMP_NEAR = 15;        // JMP near
            const uint32_t JMP_SHORT = 16;       // JMP short
            const uint32_t JCOND_TAKEN = 16;     // Conditional jump, taken
            const uint32_t JCOND_NOT_TAKEN = 4;  // Conditional jump, not taken
            
            const uint32_t CALL_NEAR = 19;       // CALL near
            const uint32_t RET_NEAR = 20;        // RET near
            
            const uint32_t SHIFT_REG_1 = 2;      // Shift/rotate register by 1
            const uint32_t SHIFT_MEM_1 = 15;     // Shift/rotate memory by 1
            const uint32_t SHIFT_REG_CL = 8;     // Shift/rotate register by CL
            const uint32_t SHIFT_MEM_CL = 20;    // Shift/rotate memory by CL
            
            const uint32_t FLAG_OP = 2;          // Flag operations (CLC, STC, etc.)
            
            const uint32_t INT = 51;             // INT instruction
            const uint32_t HLT = 2;              // HLT instruction
        } cycles;

        // Opcode table: opcode -> handler function
        using InstructionHandler = std::function<uint32_t()>;
        std::unordered_map<uint8_t, InstructionHandler> opcodeTable;

        //----------------------------------------------------------------------
        // Internal helper methods
        //----------------------------------------------------------------------
        // Fetch next byte or word from memory and advance IP
        uint8_t  fetchByte();
        uint16_t fetchWord();

        // Decode the opcode from memory, look up and call the handler
        uint32_t decodeAndExecute(uint8_t opcode);

        // Return pointer to a 16-bit register based on reg index
        uint16_t* getRegisterReference(uint8_t reg);

        // Return pointer into memory for the given addressing mode
        // (mod r/m) from an x86 ModR/M byte
        uint16_t* getMemoryReference(uint8_t mod, uint8_t rm);

        // Flag-setting helpers
        // For arithmetic ops (ADD, SUB, etc.)
        void setArithmeticFlags(uint32_t result, uint16_t dest, uint16_t src);

        //----------------------------------------------------------------------
        // Instruction handlers - now return cycle counts
        //----------------------------------------------------------------------
        // Move
        uint32_t handleMOV();
        
        // MOV register, immediate (B0-BF)
        uint32_t handleMOVRegImm();
        
        // Arithmetic
        uint32_t handleADD();
        uint32_t handleADD8();
        uint32_t handleADDImm8();
        uint32_t handleADDImm16();
        uint32_t handleSUB();
        uint32_t handleADC();  // Add with Carry
        uint32_t handleADC8(); // Add with Carry (8-bit)
        uint32_t handleSBB();  // Subtract with Borrow
        uint32_t handleSBB8(); // Subtract with Borrow (8-bit)
        uint32_t handleCMP();
        uint32_t handleCMPImm();
        uint32_t handleGroup1();
        uint32_t handleINC();
        uint32_t handleDEC();

        // Logical
        uint32_t handleAND();
        uint32_t handleANDImm();
        uint32_t handleOR();
        uint32_t handleORImm();
        uint32_t handleXOR();
        uint32_t handleXORImm();
        uint32_t handleNOT();

        // String operations
        uint32_t handleMOVS();  // Move string
        uint32_t handleCMPS();  // Compare string
        uint32_t handleSTOS();  // Store string
        uint32_t handleLODS();  // Load string
        uint32_t handleSCAS();  // Scan string
        uint32_t handleREP();   // Repeat prefix

        // Shift/Rotate
        uint32_t handleSHL();
        uint32_t handleSHR();
        uint32_t handleROL();  // Rotate Left
        uint32_t handleROR();  // Rotate Right
        uint32_t handleRCL();  // Rotate through Carry Left
        uint32_t handleRCR();  // Rotate through Carry Right
        uint32_t handleSAL();  // Shift Arithmetic Left (same as SHL)
        uint32_t handleSAR();  // Shift Arithmetic Right

        // Flag Control
        uint32_t handleCLC();  // Clear Carry Flag
        uint32_t handleSTC();  // Set Carry Flag
        uint32_t handleCMC();  // Complement Carry Flag
        uint32_t handleCLD();  // Clear Direction Flag
        uint32_t handleSTD();  // Set Direction Flag
        uint32_t handleCLI();  // Clear Interrupt Flag
        uint32_t handleSTI();  // Set Interrupt Flag

        // Jump / Branch
        uint32_t handleJMP();
        uint32_t handleJE();
        uint32_t handleJNE();
        uint32_t handleJG();
        uint32_t handleJGE();
        uint32_t handleJL();
        uint32_t handleJLE();

        // Call/Return/Stack
        uint32_t handlePUSH();
        uint32_t handlePOP();
        uint32_t handleCALL();
        uint32_t handleRET();
        uint32_t handleIRET();  // Return from interrupt

        // Interrupt / Halt
        uint32_t handleINT();
        uint32_t handleHLT();

        // I/O operations
        uint32_t handleIN();    // Input from port
        uint32_t handleOUT();   // Output to port

        //----------------------------------------------------------------------
        // F6 / F7 handlers for 8-bit and 16-bit ops
        //----------------------------------------------------------------------
        uint32_t handleF6(); // 0xF6 => 8-bit (TEST, NOT, NEG, MUL, IMUL, DIV, IDIV)
        uint32_t handleF7(); // 0xF7 => 16-bit (TEST, NOT, NEG, MUL, IMUL, DIV, IDIV)

        // 8-bit sub-handlers
        uint32_t handleTest8(uint8_t modrm);
        uint32_t handleNot8(uint8_t modrm);
        uint32_t handleNeg8(uint8_t modrm);
        uint32_t handleMul8(uint8_t modrm);
        uint32_t handleIMul8(uint8_t modrm);
        uint32_t handleDiv8(uint8_t modrm);
        uint32_t handleIDiv8(uint8_t modrm);

        // 16-bit sub-handlers
        uint32_t handleTest16(uint8_t modrm);
        uint32_t handleNot16(uint8_t modrm);
        uint32_t handleNeg16(uint8_t modrm);
        uint32_t handleMul16(uint8_t modrm);
        uint32_t handleIMul16(uint8_t modrm);
        uint32_t handleDiv16(uint8_t modrm);
        uint32_t handleIDiv16(uint8_t modrm);

        // Helper methods
        uint32_t getEffectiveAddress(uint8_t mod, uint8_t rm);
        uint8_t* get8BitRegisterRef(uint8_t reg);
        void setArithmeticFlags8(uint16_t result, uint8_t dest, uint8_t src);

        // Rotate and Shift Helper Methods
        uint32_t handleROL8(uint8_t modrm, uint8_t count, uint8_t mod, uint8_t rm);
        uint32_t handleROL16(uint8_t modrm, uint8_t count, uint8_t mod, uint8_t rm);
        uint32_t handleROR8(uint8_t modrm, uint8_t count, uint8_t mod, uint8_t rm);
        uint32_t handleROR16(uint8_t modrm, uint8_t count, uint8_t mod, uint8_t rm);
        uint32_t handleRCL8(uint8_t modrm, uint8_t count, uint8_t mod, uint8_t rm);
        uint32_t handleRCL16(uint8_t modrm, uint8_t count, uint8_t mod, uint8_t rm);
        uint32_t handleRCR8(uint8_t modrm, uint8_t count, uint8_t mod, uint8_t rm);
        uint32_t handleRCR16(uint8_t modrm, uint8_t count, uint8_t mod, uint8_t rm);
        uint32_t handleSAL8(uint8_t modrm, uint8_t count, uint8_t mod, uint8_t rm);
        uint32_t handleSAL16(uint8_t modrm, uint8_t count, uint8_t mod, uint8_t rm);
        uint32_t handleSHR8(uint8_t modrm, uint8_t count, uint8_t mod, uint8_t rm);
        uint32_t handleSHR16(uint8_t modrm, uint8_t count, uint8_t mod, uint8_t rm);
        uint32_t handleSAR8(uint8_t modrm, uint8_t count, uint8_t mod, uint8_t rm);
        uint32_t handleSAR16(uint8_t modrm, uint8_t count, uint8_t mod, uint8_t rm);
    };

} 

#endif // INSTRUCTIONS_HPP
