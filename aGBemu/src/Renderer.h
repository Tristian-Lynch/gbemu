#pragma once
#include <SDL3/SDL.h>

// Forward declaration for optional PPU framebuffer rendering
class PPU;

class Renderer {
public:
    Renderer();
    ~Renderer();

    // Initialize OpenGL + ImGui
    bool Init(SDL_Window* window);

    // Frame management
    void BeginFrame();
    void RenderUI(PPU* ppu = nullptr);
    void EndFrame();

    // Cleanup
    void Shutdown();

private:
    SDL_GLContext glContext;  // OpenGL context
};
