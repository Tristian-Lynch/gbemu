#pragma once

#include <string>
#include <fstream>

#ifndef _EMU_H
#define _EMU_H

typedef unsigned char BYTE;
typedef char SIGNED_BYTE;
typedef unsigned short WORD;
typedef signed short SIGNED_WORD;

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