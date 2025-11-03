#include "CPU.h"
#include "MMU.h"
#include <SDL3/SDL.h> // for optional logging

CPU::CPU(MMU* mmu) : mmu(mmu) {
    Reset();
}

void CPU::Reset() {
    A = F = B = C = D = E = H = L = 0;
    SP = 0xFFFE;
    PC = 0x0100; // entry point after BIOS
}

// --- Step / Fetch ---
int CPU::Step() {
    uint8_t opcode = Fetch8();

    switch (opcode) {
    case 0x00: NOP(); return 4;        // NOP takes 4 cycles
    case 0x3E: LD_r_n(A); return 8;    // LD A, n
    case 0x06: LD_r_n(B); return 8;    // LD B, n
    case 0x0E: LD_r_n(C); return 8;    // LD C, n
    case 0x04: INC_r(B); return 4;     // INC B
    case 0x05: DEC_r(B); return 4;     // DEC B
    case 0xC3: {                        // JP nn
        uint16_t addr = Fetch16();
        JP(addr);
        return 16;
    }
    default: {
        SDL_Log("Unimplemented opcode: 0x%02X at PC=%04X", opcode, PC - 1);
        NOP();
        return 4;
    }
    }
}

// --- Instruction implementations ---
void CPU::NOP() {}

void CPU::LD_r_n(uint8_t& reg) {
    reg = Fetch8();
}

void CPU::INC_r(uint8_t& reg) {
    uint8_t old = reg;
    reg++;

    if (reg == 0) F |= (1 << FLAG_Z);
    else          F &= ~(1 << FLAG_Z);

    F &= ~(1 << FLAG_N);

    if ((old & 0x0F) + 1 > 0x0F) F |= (1 << FLAG_H);
    else                         F &= ~(1 << FLAG_H);
}

void CPU::DEC_r(uint8_t& reg) {
    uint8_t old = reg;
    reg--;

    if (reg == 0) F |= (1 << FLAG_Z);
    else          F &= ~(1 << FLAG_Z);

    F |= (1 << FLAG_N);

    if ((old & 0x0F) == 0) F |= (1 << FLAG_H);
    else                   F &= ~(1 << FLAG_H);
}

void CPU::JP(uint16_t addr) {
    PC = addr;
}

// --- Fetch helpers ---
uint8_t CPU::Fetch8() { return mmu->Read8(PC++); }

uint16_t CPU::Fetch16() {
    uint8_t low = Fetch8();
    uint8_t high = Fetch8();
    return (high << 8) | low;
}

// --- Register helpers ---
uint16_t CPU::GetAF() { return (A << 8) | F; }
uint16_t CPU::GetBC() { return (B << 8) | C; }
uint16_t CPU::GetDE() { return (D << 8) | E; }
uint16_t CPU::GetHL() { return (H << 8) | L; }

void CPU::SetAF(uint16_t val) { A = val >> 8; F = val & 0xF0; }
void CPU::SetBC(uint16_t val) { B = val >> 8; C = val & 0xFF; }
void CPU::SetDE(uint16_t val) { D = val >> 8; E = val & 0xFF; }
void CPU::SetHL(uint16_t val) { H = val >> 8; L = val & 0xFF; }

void CPU::RunCycles(int n) {
    int cycles = 0;
    while (cycles < n)
        cycles += Step();
}
