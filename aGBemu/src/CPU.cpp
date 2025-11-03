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
    // NOP
    case 0x00: NOP(); return 4;
    
    // LD r, n (8-bit immediate loads)
    case 0x06: LD_r_n(B); return 8;
    case 0x0E: LD_r_n(C); return 8;
    case 0x16: LD_r_n(D); return 8;
    case 0x1E: LD_r_n(E); return 8;
    case 0x26: LD_r_n(H); return 8;
    case 0x2E: LD_r_n(L); return 8;
    case 0x3E: LD_r_n(A); return 8;
    
    // LD r, r (register to register)
    case 0x40: LD_r_r(B, B); return 4;
    case 0x41: LD_r_r(B, C); return 4;
    case 0x42: LD_r_r(B, D); return 4;
    case 0x43: LD_r_r(B, E); return 4;
    case 0x44: LD_r_r(B, H); return 4;
    case 0x45: LD_r_r(B, L); return 4;
    case 0x46: LD_r_HL(B); return 8;
    case 0x47: LD_r_r(B, A); return 4;
    case 0x48: LD_r_r(C, B); return 4;
    case 0x49: LD_r_r(C, C); return 4;
    case 0x4A: LD_r_r(C, D); return 4;
    case 0x4B: LD_r_r(C, E); return 4;
    case 0x4C: LD_r_r(C, H); return 4;
    case 0x4D: LD_r_r(C, L); return 4;
    case 0x4E: LD_r_HL(C); return 8;
    case 0x4F: LD_r_r(C, A); return 4;
    case 0x50: LD_r_r(D, B); return 4;
    case 0x51: LD_r_r(D, C); return 4;
    case 0x52: LD_r_r(D, D); return 4;
    case 0x53: LD_r_r(D, E); return 4;
    case 0x54: LD_r_r(D, H); return 4;
    case 0x55: LD_r_r(D, L); return 4;
    case 0x56: LD_r_HL(D); return 8;
    case 0x57: LD_r_r(D, A); return 4;
    case 0x58: LD_r_r(E, B); return 4;
    case 0x59: LD_r_r(E, C); return 4;
    case 0x5A: LD_r_r(E, D); return 4;
    case 0x5B: LD_r_r(E, E); return 4;
    case 0x5C: LD_r_r(E, H); return 4;
    case 0x5D: LD_r_r(E, L); return 4;
    case 0x5E: LD_r_HL(E); return 8;
    case 0x5F: LD_r_r(E, A); return 4;
    case 0x60: LD_r_r(H, B); return 4;
    case 0x61: LD_r_r(H, C); return 4;
    case 0x62: LD_r_r(H, D); return 4;
    case 0x63: LD_r_r(H, E); return 4;
    case 0x64: LD_r_r(H, H); return 4;
    case 0x65: LD_r_r(H, L); return 4;
    case 0x66: LD_r_HL(H); return 8;
    case 0x67: LD_r_r(H, A); return 4;
    case 0x68: LD_r_r(L, B); return 4;
    case 0x69: LD_r_r(L, C); return 4;
    case 0x6A: LD_r_r(L, D); return 4;
    case 0x6B: LD_r_r(L, E); return 4;
    case 0x6C: LD_r_r(L, H); return 4;
    case 0x6D: LD_r_r(L, L); return 4;
    case 0x6E: LD_r_HL(L); return 8;
    case 0x6F: LD_r_r(L, A); return 4;
    case 0x70: LD_HL_r(B); return 8;
    case 0x71: LD_HL_r(C); return 8;
    case 0x72: LD_HL_r(D); return 8;
    case 0x73: LD_HL_r(E); return 8;
    case 0x74: LD_HL_r(H); return 8;
    case 0x75: LD_HL_r(L); return 8;
    case 0x76: HALT(); return 4;
    case 0x77: LD_HL_r(A); return 8;
    case 0x78: LD_r_r(A, B); return 4;
    case 0x79: LD_r_r(A, C); return 4;
    case 0x7A: LD_r_r(A, D); return 4;
    case 0x7B: LD_r_r(A, E); return 4;
    case 0x7C: LD_r_r(A, H); return 4;
    case 0x7D: LD_r_r(A, L); return 4;
    case 0x7E: LD_r_HL(A); return 8;
    case 0x7F: LD_r_r(A, A); return 4;
    
    // LD (HL), n
    case 0x36: LD_HL_n(); return 12;
    
    // INC r (8-bit)
    case 0x04: INC_r(B); return 4;
    case 0x0C: INC_r(C); return 4;
    case 0x14: INC_r(D); return 4;
    case 0x1C: INC_r(E); return 4;
    case 0x24: INC_r(H); return 4;
    case 0x2C: INC_r(L); return 4;
    case 0x34: INC_HLmem(); return 12;
    case 0x3C: INC_r(A); return 4;
    
    // DEC r (8-bit)
    case 0x05: DEC_r(B); return 4;
    case 0x0D: DEC_r(C); return 4;
    case 0x15: DEC_r(D); return 4;
    case 0x1D: DEC_r(E); return 4;
    case 0x25: DEC_r(H); return 4;
    case 0x2D: DEC_r(L); return 4;
    case 0x35: DEC_HLmem(); return 12;
    case 0x3D: DEC_r(A); return 4;
    
    // 16-bit loads
    case 0x01: { SetBC(Fetch16()); return 12; }
    case 0x11: { SetDE(Fetch16()); return 12; }
    case 0x21: { SetHL(Fetch16()); return 12; }
    case 0x31: { SP = Fetch16(); return 12; }
    case 0x0A: LD_A_BC(); return 8;
    case 0x1A: LD_A_DE(); return 8;
    case 0x2A: LD_A_HLinc(); return 8;
    case 0x3A: LD_A_HLdec(); return 8;
    case 0x02: LD_BC_A(); return 8;
    case 0x12: LD_DE_A(); return 8;
    case 0x22: LD_HLinc_A(); return 8;
    case 0x32: LD_HLdec_A(); return 8;
    case 0xFA: LD_A_nn(); return 16;
    case 0xEA: LD_nn_A(); return 16;
    case 0xF2: LD_A_C(); return 8;
    case 0xE2: LD_C_A(); return 8;
    case 0xF9: { SP = GetHL(); return 8; }
    case 0xF8: LD_HL_SPn(); return 12;
    
    // 16-bit INC/DEC
    case 0x03: INC_BC(); return 8;
    case 0x13: INC_DE(); return 8;
    case 0x23: INC_HL(); return 8;
    case 0x33: INC_SP(); return 8;
    case 0x0B: DEC_BC(); return 8;
    case 0x1B: DEC_DE(); return 8;
    case 0x2B: DEC_HL(); return 8;
    case 0x3B: DEC_SP(); return 8;
    
    // ADD HL, rr (16-bit addition)
    case 0x09: ADD_HL_BC(); return 8;
    case 0x19: ADD_HL_DE(); return 8;
    case 0x29: ADD_HL_HL(); return 8;
    case 0x39: ADD_HL_SP(); return 8;
    
    // ADD A, r
    case 0x80: ADD_A_r(B); return 4;
    case 0x81: ADD_A_r(C); return 4;
    case 0x82: ADD_A_r(D); return 4;
    case 0x83: ADD_A_r(E); return 4;
    case 0x84: ADD_A_r(H); return 4;
    case 0x85: ADD_A_r(L); return 4;
    case 0x86: ADD_A_HL(); return 8;
    case 0x87: ADD_A_r(A); return 4;
    case 0xC6: ADD_A_n(); return 8;
    
    // ADC A, r
    case 0x88: ADC_A_r(B); return 4;
    case 0x89: ADC_A_r(C); return 4;
    case 0x8A: ADC_A_r(D); return 4;
    case 0x8B: ADC_A_r(E); return 4;
    case 0x8C: ADC_A_r(H); return 4;
    case 0x8D: ADC_A_r(L); return 4;
    case 0x8E: ADC_A_HL(); return 8;
    case 0x8F: ADC_A_r(A); return 4;
    case 0xCE: ADC_A_n(); return 8;
    
    // SUB A, r
    case 0x90: SUB_A_r(B); return 4;
    case 0x91: SUB_A_r(C); return 4;
    case 0x92: SUB_A_r(D); return 4;
    case 0x93: SUB_A_r(E); return 4;
    case 0x94: SUB_A_r(H); return 4;
    case 0x95: SUB_A_r(L); return 4;
    case 0x96: SUB_A_HL(); return 8;
    case 0x97: SUB_A_r(A); return 4;
    case 0xD6: SUB_A_n(); return 8;
    
    // SBC A, r
    case 0x98: SBC_A_r(B); return 4;
    case 0x99: SBC_A_r(C); return 4;
    case 0x9A: SBC_A_r(D); return 4;
    case 0x9B: SBC_A_r(E); return 4;
    case 0x9C: SBC_A_r(H); return 4;
    case 0x9D: SBC_A_r(L); return 4;
    case 0x9E: SBC_A_HL(); return 8;
    case 0x9F: SBC_A_r(A); return 4;
    case 0xDE: SBC_A_n(); return 8;
    
    // AND A, r
    case 0xA0: AND_A_r(B); return 4;
    case 0xA1: AND_A_r(C); return 4;
    case 0xA2: AND_A_r(D); return 4;
    case 0xA3: AND_A_r(E); return 4;
    case 0xA4: AND_A_r(H); return 4;
    case 0xA5: AND_A_r(L); return 4;
    case 0xA6: AND_A_HL(); return 8;
    case 0xA7: AND_A_r(A); return 4;
    case 0xE6: AND_A_n(); return 8;
    
    // OR A, r
    case 0xB0: OR_A_r(B); return 4;
    case 0xB1: OR_A_r(C); return 4;
    case 0xB2: OR_A_r(D); return 4;
    case 0xB3: OR_A_r(E); return 4;
    case 0xB4: OR_A_r(H); return 4;
    case 0xB5: OR_A_r(L); return 4;
    case 0xB6: OR_A_HL(); return 8;
    case 0xB7: OR_A_r(A); return 4;
    case 0xF6: OR_A_n(); return 8;
    
    // XOR A, r
    case 0xA8: XOR_A_r(B); return 4;
    case 0xA9: XOR_A_r(C); return 4;
    case 0xAA: XOR_A_r(D); return 4;
    case 0xAB: XOR_A_r(E); return 4;
    case 0xAC: XOR_A_r(H); return 4;
    case 0xAD: XOR_A_r(L); return 4;
    case 0xAE: XOR_A_HL(); return 8;
    case 0xAF: XOR_A_r(A); return 4;
    case 0xEE: XOR_A_n(); return 8;
    
    // CP A, r
    case 0xB8: CP_A_r(B); return 4;
    case 0xB9: CP_A_r(C); return 4;
    case 0xBA: CP_A_r(D); return 4;
    case 0xBB: CP_A_r(E); return 4;
    case 0xBC: CP_A_r(H); return 4;
    case 0xBD: CP_A_r(L); return 4;
    case 0xBE: CP_A_HL(); return 8;
    case 0xBF: CP_A_r(A); return 4;
    case 0xFE: CP_A_n(); return 8;
    
    // Rotates
    case 0x07: RLC_A(); return 4;
    case 0x0F: RRC_A(); return 4;
    case 0x17: RL_A(); return 4;
    case 0x1F: RR_A(); return 4;
    
    // Miscellaneous
    case 0x27: DAA(); return 4;
    case 0x2F: CPL(); return 4;
    case 0x37: SCF(); return 4;
    case 0x3F: CCF(); return 4;
    
    // Jumps
    case 0xC3: { uint16_t addr = Fetch16(); JP(addr); return 16; }
    case 0xC2: { uint16_t addr = Fetch16(); bool taken = !GetFlag(FLAG_Z); JP_NZ(addr); return taken ? 16 : 12; }
    case 0xCA: { uint16_t addr = Fetch16(); bool taken = GetFlag(FLAG_Z); JP_Z(addr); return taken ? 16 : 12; }
    case 0xD2: { uint16_t addr = Fetch16(); bool taken = !GetFlag(FLAG_C); JP_NC(addr); return taken ? 16 : 12; }
    case 0xDA: { uint16_t addr = Fetch16(); bool taken = GetFlag(FLAG_C); JP_C(addr); return taken ? 16 : 12; }
    case 0xE9: { PC = GetHL(); return 4; } // JP (HL)
    
    // Relative jumps
    case 0x18: { int8_t offset = static_cast<int8_t>(Fetch8()); JR(offset); return 12; }
    case 0x20: { int8_t offset = static_cast<int8_t>(Fetch8()); bool taken = !GetFlag(FLAG_Z); JR_NZ(offset); return taken ? 12 : 8; }
    case 0x28: { int8_t offset = static_cast<int8_t>(Fetch8()); bool taken = GetFlag(FLAG_Z); JR_Z(offset); return taken ? 12 : 8; }
    case 0x30: { int8_t offset = static_cast<int8_t>(Fetch8()); bool taken = !GetFlag(FLAG_C); JR_NC(offset); return taken ? 12 : 8; }
    case 0x38: { int8_t offset = static_cast<int8_t>(Fetch8()); bool taken = GetFlag(FLAG_C); JR_C(offset); return taken ? 12 : 8; }
    
    // Calls
    case 0xCD: { uint16_t addr = Fetch16(); CALL(addr); return 24; }
    case 0xC4: { uint16_t addr = Fetch16(); bool taken = !GetFlag(FLAG_Z); CALL_NZ(addr); return taken ? 24 : 12; }
    case 0xCC: { uint16_t addr = Fetch16(); bool taken = GetFlag(FLAG_Z); CALL_Z(addr); return taken ? 24 : 12; }
    case 0xD4: { uint16_t addr = Fetch16(); bool taken = !GetFlag(FLAG_C); CALL_NC(addr); return taken ? 24 : 12; }
    case 0xDC: { uint16_t addr = Fetch16(); bool taken = GetFlag(FLAG_C); CALL_C(addr); return taken ? 24 : 12; }
    
    // Returns
    case 0xC9: RET(); return 16;
    case 0xC0: { bool taken = !GetFlag(FLAG_Z); RET_NZ(); return taken ? 20 : 8; }
    case 0xC8: { bool taken = GetFlag(FLAG_Z); RET_Z(); return taken ? 20 : 8; }
    case 0xD0: { bool taken = !GetFlag(FLAG_C); RET_NC(); return taken ? 20 : 8; }
    case 0xD8: { bool taken = GetFlag(FLAG_C); RET_C(); return taken ? 20 : 8; }
    case 0xD9: RETI(); return 16;
    
    // Stack operations
    case 0xF5: PUSH_AF(); return 16;
    case 0xC5: PUSH_BC(); return 16;
    case 0xD5: PUSH_DE(); return 16;
    case 0xE5: PUSH_HL(); return 16;
    case 0xF1: POP_AF(); return 12;
    case 0xC1: POP_BC(); return 12;
    case 0xD1: POP_DE(); return 12;
    case 0xE1: POP_HL(); return 12;
    
    // Interrupts and other
    case 0xF3: DI(); return 4;
    case 0xFB: EI(); return 4;
    case 0x10: STOP(); return 4;
    
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

// --- Flag helpers ---
bool CPU::GetFlag(Flag flag) {
    return (F & (1 << flag)) != 0;
}

void CPU::SetFlag(Flag flag, bool value) {
    if (value)
        F |= (1 << flag);
    else
        F &= ~(1 << flag);
}

bool CPU::CheckCondition(uint8_t condition) {
    switch (condition) {
        case 0: return !GetFlag(FLAG_Z); // NZ
        case 1: return GetFlag(FLAG_Z);   // Z
        case 2: return !GetFlag(FLAG_C);  // NC
        case 3: return GetFlag(FLAG_C);   // C
        default: return false;
    }
}

// --- LD instructions ---
void CPU::LD_r_r(uint8_t& dest, uint8_t src) {
    dest = src;
}

void CPU::LD_r_HL(uint8_t& reg) {
    reg = mmu->Read8(GetHL());
}

void CPU::LD_HL_r(uint8_t reg) {
    mmu->Write8(GetHL(), reg);
}

void CPU::LD_HL_n() {
    uint8_t n = Fetch8();
    mmu->Write8(GetHL(), n);
}

void CPU::LD_A_BC() {
    A = mmu->Read8(GetBC());
}

void CPU::LD_A_DE() {
    A = mmu->Read8(GetDE());
}

void CPU::LD_BC_A() {
    mmu->Write8(GetBC(), A);
}

void CPU::LD_DE_A() {
    mmu->Write8(GetDE(), A);
}

void CPU::LD_A_nn() {
    uint16_t addr = Fetch16();
    A = mmu->Read8(addr);
}

void CPU::LD_nn_A() {
    uint16_t addr = Fetch16();
    mmu->Write8(addr, A);
}

void CPU::LD_A_C() {
    A = mmu->Read8(0xFF00 + C);
}

void CPU::LD_C_A() {
    mmu->Write8(0xFF00 + C, A);
}

void CPU::LD_A_HLinc() {
    A = mmu->Read8(GetHL());
    SetHL(GetHL() + 1);
}

void CPU::LD_A_HLdec() {
    A = mmu->Read8(GetHL());
    SetHL(GetHL() - 1);
}

void CPU::LD_HLinc_A() {
    mmu->Write8(GetHL(), A);
    SetHL(GetHL() + 1);
}

void CPU::LD_HLdec_A() {
    mmu->Write8(GetHL(), A);
    SetHL(GetHL() - 1);
}

void CPU::LD_SP_nn() {
    SP = Fetch16();
}

void CPU::LD_HL_SPn() {
    int8_t n = static_cast<int8_t>(Fetch8());
    uint16_t result = SP + n;
    
    SetFlag(FLAG_C, (SP & 0xFF) + n > 0xFF);
    SetFlag(FLAG_H, (SP & 0x0F) + (n & 0x0F) > 0x0F);
    SetFlag(FLAG_Z, false);
    SetFlag(FLAG_N, false);
    
    SetHL(result);
}

// --- 8-bit INC/DEC (memory) ---
void CPU::INC_HLmem() {
    uint16_t addr = GetHL();
    uint8_t val = mmu->Read8(addr);
    uint8_t old = val;
    val++;
    
    if (val == 0) SetFlag(FLAG_Z, true);
    else SetFlag(FLAG_Z, false);
    SetFlag(FLAG_N, false);
    SetFlag(FLAG_H, (old & 0x0F) + 1 > 0x0F);
    
    mmu->Write8(addr, val);
}

void CPU::DEC_HLmem() {
    uint16_t addr = GetHL();
    uint8_t val = mmu->Read8(addr);
    uint8_t old = val;
    val--;
    
    if (val == 0) SetFlag(FLAG_Z, true);
    else SetFlag(FLAG_Z, false);
    SetFlag(FLAG_N, true);
    SetFlag(FLAG_H, (old & 0x0F) == 0);
    
    mmu->Write8(addr, val);
}

// --- 16-bit INC/DEC ---
void CPU::INC_BC() {
    SetBC(GetBC() + 1);
}

void CPU::DEC_BC() {
    SetBC(GetBC() - 1);
}

void CPU::INC_DE() {
    SetDE(GetDE() + 1);
}

void CPU::DEC_DE() {
    SetDE(GetDE() - 1);
}

void CPU::INC_HL() {
    SetHL(GetHL() + 1);
}

void CPU::DEC_HL() {
    SetHL(GetHL() - 1);
}

void CPU::INC_SP() {
    SP++;
}

void CPU::DEC_SP() {
    SP--;
}

// --- Arithmetic operations ---
void CPU::ADD_A_r(uint8_t val) {
    uint16_t result = A + val;
    
    SetFlag(FLAG_Z, (result & 0xFF) == 0);
    SetFlag(FLAG_N, false);
    SetFlag(FLAG_H, (A & 0x0F) + (val & 0x0F) > 0x0F);
    SetFlag(FLAG_C, result > 0xFF);
    
    A = result & 0xFF;
}

void CPU::ADD_A_n() {
    ADD_A_r(Fetch8());
}

void CPU::ADD_A_HL() {
    ADD_A_r(mmu->Read8(GetHL()));
}

void CPU::ADD_HL_BC() {
    uint32_t result = GetHL() + GetBC();
    SetFlag(FLAG_N, false);
    SetFlag(FLAG_H, (GetHL() & 0x0FFF) + (GetBC() & 0x0FFF) > 0x0FFF);
    SetFlag(FLAG_C, result > 0xFFFF);
    SetHL(result & 0xFFFF);
}

void CPU::ADD_HL_DE() {
    uint32_t result = GetHL() + GetDE();
    SetFlag(FLAG_N, false);
    SetFlag(FLAG_H, (GetHL() & 0x0FFF) + (GetDE() & 0x0FFF) > 0x0FFF);
    SetFlag(FLAG_C, result > 0xFFFF);
    SetHL(result & 0xFFFF);
}

void CPU::ADD_HL_HL() {
    uint32_t result = GetHL() + GetHL();
    SetFlag(FLAG_N, false);
    SetFlag(FLAG_H, (GetHL() & 0x0FFF) + (GetHL() & 0x0FFF) > 0x0FFF);
    SetFlag(FLAG_C, result > 0xFFFF);
    SetHL(result & 0xFFFF);
}

void CPU::ADD_HL_SP() {
    uint32_t result = GetHL() + SP;
    SetFlag(FLAG_N, false);
    SetFlag(FLAG_H, (GetHL() & 0x0FFF) + (SP & 0x0FFF) > 0x0FFF);
    SetFlag(FLAG_C, result > 0xFFFF);
    SetHL(result & 0xFFFF);
}

void CPU::ADC_A_r(uint8_t val) {
    uint8_t carry = GetFlag(FLAG_C) ? 1 : 0;
    uint16_t result = A + val + carry;
    
    SetFlag(FLAG_Z, (result & 0xFF) == 0);
    SetFlag(FLAG_N, false);
    SetFlag(FLAG_H, (A & 0x0F) + (val & 0x0F) + carry > 0x0F);
    SetFlag(FLAG_C, result > 0xFF);
    
    A = result & 0xFF;
}

void CPU::ADC_A_n() {
    ADC_A_r(Fetch8());
}

void CPU::ADC_A_HL() {
    ADC_A_r(mmu->Read8(GetHL()));
}

void CPU::SUB_A_r(uint8_t val) {
    uint8_t result = A - val;
    
    SetFlag(FLAG_Z, result == 0);
    SetFlag(FLAG_N, true);
    SetFlag(FLAG_H, (A & 0x0F) < (val & 0x0F));
    SetFlag(FLAG_C, A < val);
    
    A = result;
}

void CPU::SUB_A_n() {
    SUB_A_r(Fetch8());
}

void CPU::SUB_A_HL() {
    SUB_A_r(mmu->Read8(GetHL()));
}

void CPU::SBC_A_r(uint8_t val) {
    uint8_t carry = GetFlag(FLAG_C) ? 1 : 0;
    uint16_t total = val + carry;
    uint8_t result = A - total;
    
    SetFlag(FLAG_Z, result == 0);
    SetFlag(FLAG_N, true);
    SetFlag(FLAG_H, (A & 0x0F) < (total & 0x0F));
    SetFlag(FLAG_C, A < total);
    
    A = result;
}

void CPU::SBC_A_n() {
    SBC_A_r(Fetch8());
}

void CPU::SBC_A_HL() {
    SBC_A_r(mmu->Read8(GetHL()));
}

void CPU::CP_A_r(uint8_t val) {
    SetFlag(FLAG_Z, A == val);
    SetFlag(FLAG_N, true);
    SetFlag(FLAG_H, (A & 0x0F) < (val & 0x0F));
    SetFlag(FLAG_C, A < val);
}

void CPU::CP_A_n() {
    CP_A_r(Fetch8());
}

void CPU::CP_A_HL() {
    CP_A_r(mmu->Read8(GetHL()));
}

// --- Logical operations ---
void CPU::AND_A_r(uint8_t val) {
    A &= val;
    
    SetFlag(FLAG_Z, A == 0);
    SetFlag(FLAG_N, false);
    SetFlag(FLAG_H, true);
    SetFlag(FLAG_C, false);
}

void CPU::AND_A_n() {
    AND_A_r(Fetch8());
}

void CPU::AND_A_HL() {
    AND_A_r(mmu->Read8(GetHL()));
}

void CPU::OR_A_r(uint8_t val) {
    A |= val;
    
    SetFlag(FLAG_Z, A == 0);
    SetFlag(FLAG_N, false);
    SetFlag(FLAG_H, false);
    SetFlag(FLAG_C, false);
}

void CPU::OR_A_n() {
    OR_A_r(Fetch8());
}

void CPU::OR_A_HL() {
    OR_A_r(mmu->Read8(GetHL()));
}

void CPU::XOR_A_r(uint8_t val) {
    A ^= val;
    
    SetFlag(FLAG_Z, A == 0);
    SetFlag(FLAG_N, false);
    SetFlag(FLAG_H, false);
    SetFlag(FLAG_C, false);
}

void CPU::XOR_A_n() {
    XOR_A_r(Fetch8());
}

void CPU::XOR_A_HL() {
    XOR_A_r(mmu->Read8(GetHL()));
}

// --- Rotates and shifts ---
void CPU::RLC_A() {
    bool carry = (A & 0x80) != 0;
    A = (A << 1) | (carry ? 1 : 0);
    
    SetFlag(FLAG_Z, A == 0);
    SetFlag(FLAG_N, false);
    SetFlag(FLAG_H, false);
    SetFlag(FLAG_C, carry);
}

void CPU::RRC_A() {
    bool carry = (A & 0x01) != 0;
    A = (A >> 1) | (carry ? 0x80 : 0);
    
    SetFlag(FLAG_Z, A == 0);
    SetFlag(FLAG_N, false);
    SetFlag(FLAG_H, false);
    SetFlag(FLAG_C, carry);
}

void CPU::RL_A() {
    bool carry = GetFlag(FLAG_C);
    bool newCarry = (A & 0x80) != 0;
    A = (A << 1) | (carry ? 1 : 0);
    
    SetFlag(FLAG_Z, A == 0);
    SetFlag(FLAG_N, false);
    SetFlag(FLAG_H, false);
    SetFlag(FLAG_C, newCarry);
}

void CPU::RR_A() {
    bool carry = GetFlag(FLAG_C);
    bool newCarry = (A & 0x01) != 0;
    A = (A >> 1) | (carry ? 0x80 : 0);
    
    SetFlag(FLAG_Z, A == 0);
    SetFlag(FLAG_N, false);
    SetFlag(FLAG_H, false);
    SetFlag(FLAG_C, newCarry);
}

void CPU::SLA_r(uint8_t& reg) {
    bool carry = (reg & 0x80) != 0;
    reg <<= 1;
    
    SetFlag(FLAG_Z, reg == 0);
    SetFlag(FLAG_N, false);
    SetFlag(FLAG_H, false);
    SetFlag(FLAG_C, carry);
}

void CPU::SRA_r(uint8_t& reg) {
    bool carry = (reg & 0x01) != 0;
    bool msb = (reg & 0x80) != 0;
    reg = (reg >> 1) | (msb ? 0x80 : 0);
    
    SetFlag(FLAG_Z, reg == 0);
    SetFlag(FLAG_N, false);
    SetFlag(FLAG_H, false);
    SetFlag(FLAG_C, carry);
}

void CPU::SRL_r(uint8_t& reg) {
    bool carry = (reg & 0x01) != 0;
    reg >>= 1;
    
    SetFlag(FLAG_Z, reg == 0);
    SetFlag(FLAG_N, false);
    SetFlag(FLAG_H, false);
    SetFlag(FLAG_C, carry);
}

// --- Jump instructions ---
void CPU::JP_NZ(uint16_t addr) {
    if (!GetFlag(FLAG_Z)) PC = addr;
}

void CPU::JP_Z(uint16_t addr) {
    if (GetFlag(FLAG_Z)) PC = addr;
}

void CPU::JP_NC(uint16_t addr) {
    if (!GetFlag(FLAG_C)) PC = addr;
}

void CPU::JP_C(uint16_t addr) {
    if (GetFlag(FLAG_C)) PC = addr;
}

void CPU::JR(int8_t offset) {
    PC += offset;
}

void CPU::JR_NZ(int8_t offset) {
    if (!GetFlag(FLAG_Z)) {
        PC += offset;
    }
}

void CPU::JR_Z(int8_t offset) {
    if (GetFlag(FLAG_Z)) {
        PC += offset;
    }
}

void CPU::JR_NC(int8_t offset) {
    if (!GetFlag(FLAG_C)) {
        PC += offset;
    }
}

void CPU::JR_C(int8_t offset) {
    if (GetFlag(FLAG_C)) {
        PC += offset;
    }
}

// --- Call and return instructions ---
void CPU::CALL(uint16_t addr) {
    SP -= 2;
    mmu->Write16(SP, PC);
    PC = addr;
}

void CPU::CALL_NZ(uint16_t addr) {
    if (!GetFlag(FLAG_Z)) {
        CALL(addr);
    }
}

void CPU::CALL_Z(uint16_t addr) {
    if (GetFlag(FLAG_Z)) {
        CALL(addr);
    }
}

void CPU::CALL_NC(uint16_t addr) {
    if (!GetFlag(FLAG_C)) {
        CALL(addr);
    }
}

void CPU::CALL_C(uint16_t addr) {
    if (GetFlag(FLAG_C)) {
        CALL(addr);
    }
}

void CPU::RET() {
    PC = mmu->Read16(SP);
    SP += 2;
}

void CPU::RET_NZ() {
    if (!GetFlag(FLAG_Z)) {
        RET();
    }
}

void CPU::RET_Z() {
    if (GetFlag(FLAG_Z)) {
        RET();
    }
}

void CPU::RET_NC() {
    if (!GetFlag(FLAG_C)) {
        RET();
    }
}

void CPU::RET_C() {
    if (GetFlag(FLAG_C)) {
        RET();
    }
}

void CPU::RETI() {
    RET();
    // TODO: Enable interrupts
}

// --- Stack operations ---
void CPU::PUSH_AF() {
    SP -= 2;
    mmu->Write16(SP, GetAF());
}

void CPU::PUSH_BC() {
    SP -= 2;
    mmu->Write16(SP, GetBC());
}

void CPU::PUSH_DE() {
    SP -= 2;
    mmu->Write16(SP, GetDE());
}

void CPU::PUSH_HL() {
    SP -= 2;
    mmu->Write16(SP, GetHL());
}

void CPU::POP_AF() {
    SetAF(mmu->Read16(SP));
    SP += 2;
}

void CPU::POP_BC() {
    SetBC(mmu->Read16(SP));
    SP += 2;
}

void CPU::POP_DE() {
    SetDE(mmu->Read16(SP));
    SP += 2;
}

void CPU::POP_HL() {
    SetHL(mmu->Read16(SP));
    SP += 2;
}

// --- Miscellaneous instructions ---
void CPU::HALT() {
    // TODO: Implement halt state
}

void CPU::STOP() {
    // TODO: Implement stop state
}

void CPU::DI() {
    // TODO: Disable interrupts
}

void CPU::EI() {
    // TODO: Enable interrupts
}

void CPU::CPL() {
    A = ~A;
    SetFlag(FLAG_N, true);
    SetFlag(FLAG_H, true);
}

void CPU::SCF() {
    SetFlag(FLAG_N, false);
    SetFlag(FLAG_H, false);
    SetFlag(FLAG_C, true);
}

void CPU::CCF() {
    SetFlag(FLAG_N, false);
    SetFlag(FLAG_H, false);
    SetFlag(FLAG_C, !GetFlag(FLAG_C));
}

void CPU::DAA() {
    // Decimal Adjust Accumulator
    uint8_t correction = 0;
    bool carry = GetFlag(FLAG_C);
    
    if (GetFlag(FLAG_H) || (!GetFlag(FLAG_N) && (A & 0x0F) > 9)) {
        correction |= 0x06;
    }
    
    if (carry || (!GetFlag(FLAG_N) && A > 0x99)) {
        correction |= 0x60;
        carry = true;
    }
    
    if (GetFlag(FLAG_N)) {
        A -= correction;
    } else {
        A += correction;
    }
    
    SetFlag(FLAG_Z, A == 0);
    SetFlag(FLAG_H, false);
    SetFlag(FLAG_C, carry);
}