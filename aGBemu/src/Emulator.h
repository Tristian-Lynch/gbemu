#pragma once

#include <string>
#include <fstream>
#include "Types.h"
#include "CPU.h"
#include "MMU.h"
#include "PPU.h"

class Emulator
{
public:
    Emulator();
    ~Emulator();

    bool LoadRom(const std::string& romName);
    void Update();

private:
    // --- Core components ---
    PPU ppu;
    MMU mmu{ &ppu };
    CPU cpu{ &mmu };

    // Cartridge memory (ROM)
    BYTE m_cartridgeMemory[0x200000]; // 2 MB max

    // Frame update helpers
    void UpdateTimers(int cycles);
    void UpdateGraphics(int cycles);
    void DoInterrupts();
    int ExecuteNextOpcode();
    void RenderScreen();
};
