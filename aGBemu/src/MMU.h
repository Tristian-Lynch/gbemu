#pragma once
#include <cstdint>

class PPU;

class MMU
{
public:
    MMU(PPU* ppu);

    // 8-bit access
    uint8_t Read8(uint16_t addr);
    void Write8(uint16_t addr, uint8_t value);

    // 16-bit access (convenience for CPU instructions)
    uint16_t Read16(uint16_t addr);
    void Write16(uint16_t addr, uint16_t value);

    // Load a tiny in-memory test program at 0x0100
    void LoadTestProgram();

private:
    PPU* ppu;

    // Memory arrays
    uint8_t rom[0x8000];   // 32 KB ROM
    uint8_t wram[0x2000];  // 8 KB Work RAM
    uint8_t hram[0x7F];    // High RAM
    uint8_t io[0x80];      // IO registers
};
