#pragma once
#include <glad.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_opengl.h>

// Forward declaration for optional PPU framebuffer rendering
class PPU;

class Renderer {
public:
    Renderer();
    ~Renderer();

    bool Init(SDL_Window* window);
    void InitFramebufferTexture();

    void BeginFrame();
    void RenderUI(PPU* ppu = nullptr);
    void EndFrame();
    void RenderGameboyFrame(uint8_t* ppuFramebuffer);

    void Shutdown();

private:
    SDL_GLContext glContext;
    SDL_Window* window;

    GLuint gbTexture;
};
