#pragma once

#include <SDL3/SDL.h>
#include <string>

class PPU;

class Renderer {
public:
	Renderer();
	~Renderer();

	bool Init(const std::string& title, int width, int height);
	void BeginFrame();
	void Render();
	void EndFrame();
	void Shutdown();

private:
	SDL_Window* window = nullptr;
	SDL_GLContext glContext = nullptr;
	bool initialized = false;

	void InitImGui();
	void RenderImGuiUI(PPU& ppu);
};