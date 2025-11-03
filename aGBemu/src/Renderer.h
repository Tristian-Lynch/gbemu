#pragma once
#include <glad.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_opengl.h>
#include <cstdint>

// Forward declarations
class PPU;
class CPU;
class MMU;

class Renderer
{
public:
    Renderer();
    ~Renderer();

    // Initialize SDL3 + OpenGL context + ImGui
    bool Init(SDL_Window* window);

    // Initialize the framebuffer texture
    void InitFramebufferTexture();

    // Frame lifecycle
    void BeginFrame();
    void RenderUI(PPU* ppu = nullptr, CPU* cpu = nullptr, MMU* mmu = nullptr, bool* paused = nullptr); // add ROM UI + pause toggle
    void RenderGameboyFrame(uint8_t* ppuFramebuffer);
    void EndFrame();

    // Shutdown everything cleanly
    void Shutdown();

private:
    SDL_GLContext glContext;
    SDL_Window* window;

    GLuint gbTexture;

    // Fullscreen quad for scaling GameBoy framebuffer
    GLuint quadVAO = 0;
    GLuint quadVBO = 0;
    GLuint shaderProgram = 0;

    bool imguiInitialized;

    // OpenGL helpers
    void InitFullscreenQuad();
    bool InitShaders();
    GLuint CompileShader(const char* source, GLenum type);
};
