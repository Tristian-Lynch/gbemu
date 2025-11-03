#pragma once
#include <cstdint>

class MMU;

class CPU
{
public:
    CPU(MMU* mmu);

    void Reset();
    int Step();            // Execute a single instruction, return cycles
    void RunCycles(int n); // Optional: execute n cycles

    // --- NEW: expose only registers A and B for debug UI ---
    uint8_t GetA() const { return A; }
    uint8_t GetB() const { return B; }

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
    void LD_r_r(uint8_t& dest, uint8_t src);
    void LD_r_HL(uint8_t& reg);
    void LD_HL_r(uint8_t reg);
    void LD_HL_n();
    void LD_A_BC();
    void LD_A_DE();
    void LD_BC_A();
    void LD_DE_A();
    void LD_A_nn();
    void LD_nn_A();
    void LD_A_C();
    void LD_C_A();
    void LD_A_HLdec();
    void LD_A_HLinc();
    void LD_HLdec_A();
    void LD_HLinc_A();
    void LD_SP_nn();
    void LD_HL_SPn();
    
    // 8-bit INC/DEC
    void INC_r(uint8_t& reg);
    void DEC_r(uint8_t& reg);
    void INC_HLmem();  // INC (HL) - increment value at address HL
    void DEC_HLmem();  // DEC (HL) - decrement value at address HL
    
    // 16-bit INC/DEC
    void INC_BC();
    void DEC_BC();
    void INC_DE();
    void DEC_DE();
    void INC_HL();
    void INC_SP();
    void DEC_HL();
    void DEC_SP();
    
    // Arithmetic
    void ADD_A_r(uint8_t val);
    void ADD_A_n();
    void ADD_A_HL();
    void ADD_HL_BC();
    void ADD_HL_DE();
    void ADD_HL_HL();
    void ADD_HL_SP();
    void ADC_A_r(uint8_t val);
    void ADC_A_n();
    void ADC_A_HL();
    void SUB_A_r(uint8_t val);
    void SUB_A_n();
    void SUB_A_HL();
    void SBC_A_r(uint8_t val);
    void SBC_A_n();
    void SBC_A_HL();
    void CP_A_r(uint8_t val);
    void CP_A_n();
    void CP_A_HL();
    
    // Logical
    void AND_A_r(uint8_t val);
    void AND_A_n();
    void AND_A_HL();
    void OR_A_r(uint8_t val);
    void OR_A_n();
    void OR_A_HL();
    void XOR_A_r(uint8_t val);
    void XOR_A_n();
    void XOR_A_HL();
    
    // Rotates and shifts
    void RLC_A();
    void RRC_A();
    void RL_A();
    void RR_A();
    void SLA_r(uint8_t& reg);
    void SRA_r(uint8_t& reg);
    void SRL_r(uint8_t& reg);
    
    // Jumps
    void JP(uint16_t addr);
    void JP_NZ(uint16_t addr);
    void JP_Z(uint16_t addr);
    void JP_NC(uint16_t addr);
    void JP_C(uint16_t addr);
    void JR(int8_t offset);
    void JR_NZ(int8_t offset);
    void JR_Z(int8_t offset);
    void JR_NC(int8_t offset);
    void JR_C(int8_t offset);
    
    // Calls and returns
    void CALL(uint16_t addr);
    void CALL_NZ(uint16_t addr);
    void CALL_Z(uint16_t addr);
    void CALL_NC(uint16_t addr);
    void CALL_C(uint16_t addr);
    void RET();
    void RET_NZ();
    void RET_Z();
    void RET_NC();
    void RET_C();
    void RETI();
    
    // Stack operations
    void PUSH_AF();
    void PUSH_BC();
    void PUSH_DE();
    void PUSH_HL();
    void POP_AF();
    void POP_BC();
    void POP_DE();
    void POP_HL();
    
    // Miscellaneous
    void HALT();
    void STOP();
    void DI();
    void EI();
    void CPL();
    void SCF();
    void CCF();
    void DAA();

    // Fetch helpers
    uint8_t Fetch8();
    uint16_t Fetch16();
    
    // Flag helpers
    bool GetFlag(Flag flag);
    void SetFlag(Flag flag, bool value);
    
    // Helper for checking condition codes
    bool CheckCondition(uint8_t condition);
};
