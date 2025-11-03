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

    // Load ROM image into fixed 32KB ROM (no MBC yet)
    bool LoadROMFromFile(const char* filepath);
    bool IsROMLoaded() const { return romLoaded; }
    uint32_t GetROMLoadGeneration() const { return romLoadGeneration; }
    
    // Load a tiny in-memory test program at 0x0100 (dev only)
    void LoadTestProgram();

private:
    PPU* ppu;

    // Memory arrays
    uint8_t rom[0x8000];   // 32 KB ROM
    uint8_t wram[0x2000];  // 8 KB Work RAM
    uint8_t hram[0x7F];    // High RAM
    uint8_t io[0x80];      // IO registers

    // ROM load state
    bool romLoaded = false;
    uint32_t romLoadGeneration = 0; // increments each successful load
};
