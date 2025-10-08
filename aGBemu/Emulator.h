#pragma once

#include <string>
#include <fstream>
#include "Types.h"

#ifndef _EMU_H
#define _EMU_H

class Emulator
{
public:
	Emulator(void);
	~Emulator(void);
	bool LoadRom(const std::string&);
	void Update();
private:
	BYTE m_cartridgeMemory[0x200000];
	void UpdateTimers(int);
	void UpdateGraphics(int);
	void DoInterrupts();
	int ExecuteNextOpcode();
	void RenderScreen();

};

#endif