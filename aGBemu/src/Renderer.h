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

    // Fullscreen quad for scaling GameBoy framebuffer
    GLuint quadVAO = 0;
    GLuint quadVBO = 0;
    GLuint shaderProgram = 0;

    void InitFullscreenQuad();
    void InitShaders();
    GLuint CompileShader(const char* source, GLenum type);
};
