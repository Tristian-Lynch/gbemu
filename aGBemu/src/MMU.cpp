#include "MMU.h"
#include "PPU.h"
#include <cstring>
#include <iostream>

MMU::MMU(PPU* ppu) : ppu(ppu)
{
    std::memset(rom, 0, sizeof(rom));
    std::memset(wram, 0, sizeof(wram));
    std::memset(hram, 0, sizeof(hram));
    std::memset(io, 0, sizeof(io));
}

// --- 8-bit memory access ---
uint8_t MMU::Read8(uint16_t addr)
{
    if (addr <= 0x7FFF)
        return rom[addr]; // ROM

    if (addr >= 0x8000 && addr <= 0x9FFF)
        return ppu->ReadVRAM(addr - 0x8000);

    if (addr >= 0xFE00 && addr <= 0xFE9F)
        return ppu->ReadOAM(addr - 0xFE00);

    if (addr >= 0xC000 && addr <= 0xDFFF)
        return wram[addr - 0xC000];

    if (addr >= 0xFF80 && addr <= 0xFFFE)
        return hram[addr - 0xFF80];

    if (addr >= 0xFF00 && addr <= 0xFF7F)
        return io[addr - 0xFF00];

    return 0; // Unmapped memory returns 0
}

void MMU::Write8(uint16_t addr, uint8_t value)
{
    if (addr <= 0x7FFF)
    {
        // ROM is read-only for now; ignore writes
        return;
    }
    else if (addr >= 0x8000 && addr <= 0x9FFF)
    {
        ppu->WriteVRAM(addr - 0x8000, value);
    }
    else if (addr >= 0xFE00 && addr <= 0xFE9F)
    {
        ppu->WriteOAM(addr - 0xFE00, value);
    }
    else if (addr >= 0xC000 && addr <= 0xDFFF)
    {
        wram[addr - 0xC000] = value;
    }
    else if (addr >= 0xFF80 && addr <= 0xFFFE)
    {
        hram[addr - 0xFF80] = value;
    }
    else if (addr >= 0xFF00 && addr <= 0xFF7F)
    {
        io[addr - 0xFF00] = value;
    }
}

// --- 16-bit convenience access ---
uint16_t MMU::Read16(uint16_t addr)
{
    uint8_t lo = Read8(addr);
    uint8_t hi = Read8(addr + 1);
    return (hi << 8) | lo;
}

void MMU::Write16(uint16_t addr, uint16_t value)
{
    Write8(addr, value & 0xFF);
    Write8(addr + 1, value >> 8);
}

// --- Load a tiny test program into ROM ---
void MMU::LoadTestProgram()
{
    uint8_t program[] = {
        0x3E, 0x3C,       // LD A, 0x3C
        0x06, 0x42,       // LD B, 0x42
        0x04,             // INC B
        0x00,             // NOP
        0xC3, 0x00, 0x01  // JP 0x0100
    };

    for (int i = 0; i < sizeof(program); ++i)
        rom[0x0100 + i] = program[i];
}
