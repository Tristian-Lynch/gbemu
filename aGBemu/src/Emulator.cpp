#include "Emulator.h"
#include <cstring>
#include <iostream>
#include <fstream>

Emulator::Emulator()
	: mmu(&ppu), cpu(&mmu) // Initialize MMU with PPU, CPU with MMU
{

}

Emulator::~Emulator()
{

}

bool Emulator::LoadRom(const std::string& romName)
{
	// Clear cartridge memory
	memset(m_cartridgeMemory, 0, sizeof(m_cartridgeMemory));

	// Open ROM file
	std::ifstream file(romName, std::ios::binary);

	// if the file does not read correctly, return false
	if (!file) {
		std::cerr << "Failed to open ROM: " << romName << std::endl;
		return false;
	}

	// Read ROM into memory
	file.read(reinterpret_cast<char*>(m_cartridgeMemory), sizeof(m_cartridgeMemory));

	// Detect Oversized ROM
	if (file.gcount() == sizeof(m_cartridgeMemory)) {
		std::cerr << "Warning: ROM may be too large for memory!" << std::endl;
		// Possible Future Implementation.
	}

	// Return true if any bytes are read
	return file.gcount() > 0;
}

void Emulator::Update()
{
	// Number of CPU cycles per frame, at 60 frames per second.
	const int MAX_CYCLES = 69905;

	// counter for number of cycles this frame.
	int currentCycles = 0;

	// Execute instructions up to the number of operations per frame. 
	while (currentCycles < MAX_CYCLES)
	{
		int cycles = ExecuteNextOpcode();

		// If max number of cycles will be broke, c
		if (currentCycles + cycles > MAX_CYCLES)
		{
			cycles = MAX_CYCLES - currentCycles;
		}

		currentCycles += cycles;

		UpdateTimers(cycles);
		UpdateGraphics(cycles);
		DoInterrupts();
	}

	RenderScreen();
}

void Emulator::UpdateTimers(int cycles)
{
	// Not Yet Implemented
}

void Emulator::UpdateGraphics(int cycles)
{
	// Not Yet Implemented
}

void Emulator::DoInterrupts()
{
	// Not Yet Implemented
}

int Emulator::ExecuteNextOpcode()
{
	// Step CPU once
	cpu.Step();

	// For now, return 1 cycle per instruction (simplified)
	return 1;
}

void Emulator::RenderScreen()
{
	// Get PPU framebuffer for rendering
	uint8_t* framebuffer = ppu.GetFramebuffer();

	// TODO: integrate with SDL/OpenGL renderer
	// Example for testing first pixel:
	// std::cout << "Pixel 0 RGB: "
	//           << int(framebuffer[0]) << ", "
	//           << int(framebuffer[1]) << ", "
	//           << int(framebuffer[2]) << std::endl;
}
