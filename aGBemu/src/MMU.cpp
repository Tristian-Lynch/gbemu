#include "MMU.h"
#include "PPU.h"
#include <cstring>
#include <iostream>
#include <fstream>

MMU::MMU(PPU* ppu) : ppu(ppu)
{
    std::memset(rom, 0, sizeof(rom));
    std::memset(wram, 0, sizeof(wram));
    std::memset(hram, 0, sizeof(hram));
    std::memset(io, 0, sizeof(io));
    romLoaded = false;
    romLoadGeneration = 0;
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
    {
        uint16_t i = addr - 0xFF00;
        switch (addr)
        {
        case 0xFF40: return ppu->GetLCDC();
        case 0xFF42: return io[i]; // SCY mirror
        case 0xFF43: return io[i]; // SCX mirror
        case 0xFF47: return io[i]; // BGP
        case 0xFF48: return io[i]; // OBP0
        case 0xFF49: return io[i]; // OBP1
        case 0xFF4A: return io[i]; // WY
        case 0xFF4B: return io[i]; // WX
        default:
            return io[i];
        }
    }

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
        uint16_t i = addr - 0xFF00;
        io[i] = value;
        switch (addr)
        {
        case 0xFF40: ppu->SetLCDC(value); break;
        case 0xFF42: ppu->SetSCY(value); break;
        case 0xFF43: ppu->SetSCX(value); break;
        case 0xFF47: ppu->SetBGP(value); break;
        case 0xFF48: ppu->SetOBP0(value); break;
        case 0xFF49: ppu->SetOBP1(value); break;
        case 0xFF4A: ppu->SetWY(value); break;
        case 0xFF4B: ppu->SetWX(value); break;
        default: break;
        }
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

// --- Load ROM from file (up to 32KB, no MBC) ---
bool MMU::LoadROMFromFile(const char* filepath)
{
    std::ifstream file(filepath, std::ios::binary);
    if (!file)
    {
        std::cerr << "Failed to open ROM: " << filepath << std::endl;
        return false;
    }

    // Read up to 32KB into rom[]
    std::memset(rom, 0, sizeof(rom));
    file.read(reinterpret_cast<char*>(rom), sizeof(rom));
    std::streamsize bytesRead = file.gcount();

    if (bytesRead <= 0)
    {
        std::cerr << "ROM read returned 0 bytes: " << filepath << std::endl;
        return false;
    }

    // If file exceeds 32KB, we currently ignore the remainder (no MBC implemented)
    if (!file.eof())
    {
        std::cerr << "Warning: ROM larger than 32KB; only first 32KB loaded (no MBC)." << std::endl;
    }

    romLoaded = true;
    romLoadGeneration++;
    return true;
}

// --- Load a tiny test program into ROM ---
void MMU::LoadTestProgram()
{
    uint8_t program[] = {
        0x3E, 0x3C,       // LD A, 0x3C       ; initialize A
        0x06, 0x42,       // LD B, 0x42       ; initialize B
        0x04,             // loop_start: INC B
        0x00,             // NOP
        0xC3, 0x04, 0x01  // JP 0x0104        ; jump back to INC B
    };

    for (int i = 0; i < sizeof(program); ++i)
        rom[0x0100 + i] = program[i];
}

