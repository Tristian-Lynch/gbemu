#pragma once
#include <cstdint>

class MMU;

class CPU {
public:
    CPU(MMU* mmu);

    void Reset();
    void Step();           // Execute a single instruction
    void RunCycles(int n); // Optional: execute n cycles

private:
    MMU* mmu;

    // 8-bit registers
    uint8_t A, F, B, C, D, E, H, L;

    // 16-bit registers
    uint16_t SP; // Stack pointer
    uint16_t PC; // Program counter

    // Flags in F register
    enum Flag {
        FLAG_Z = 7,  // Zero
        FLAG_N = 6,  // Subtract
        FLAG_H = 5,  // Half carry
        FLAG_C = 4   // Carry
    };

    // Helpers
    uint16_t GetAF();
    uint16_t GetBC();
    uint16_t GetDE();
    uint16_t GetHL();
    void SetAF(uint16_t val);
    void SetBC(uint16_t val);
    void SetDE(uint16_t val);
    void SetHL(uint16_t val);

    // Instruction implementations
    void NOP();
    void LD_r_n(uint8_t& reg);
    void INC_r(uint8_t& reg);
    void DEC_r(uint8_t& reg);
    void JP(uint16_t addr);

    // Fetch helpers
    uint8_t Fetch8();
    uint16_t Fetch16();
};
