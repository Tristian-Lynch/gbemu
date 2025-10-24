#include "Renderer.h"
#include <imgui.h>
#include <backends/imgui_impl_sdl3.h>
#include <backends/imgui_impl_opengl3.h>
#include <SDL3/SDL_log.h>

// Constructor / Destructor
Renderer::Renderer() : glContext(nullptr), window(nullptr), gbTexture(0) {}
Renderer::~Renderer() { Shutdown(); }

// Initialize SDL3 OpenGL context + GLAD + ImGui
bool Renderer::Init(SDL_Window* window)
{
    this->window = window;
    glContext = SDL_GL_CreateContext(window);

    if (!glContext) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
            "Failed to create OpenGL context: %s", SDL_GetError());
        return false;
    }

    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to initialize GLAD");
        return false;
    }

    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "OpenGL version: %s", glGetString(GL_VERSION));

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGui_ImplSDL3_InitForOpenGL(window, glContext);
    ImGui_ImplOpenGL3_Init("#version 330");
    InitFramebufferTexture();

    return true;
}

void Renderer::InitFramebufferTexture()
{
    glGenTextures(1, &gbTexture);
    glBindTexture(GL_TEXTURE_2D, gbTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 160, 144, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Renderer::BeginFrame()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    glClearColor(0.1f, 0.12f, 0.15f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

void Renderer::RenderUI(PPU* ppu)
{
    ImGui::Begin("GameBoy Emulator");
    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);

    if (ppu) {
        ImGui::Text("PPU framebuffer placeholder");
    }

    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Renderer::RenderGameboyFrame(uint8_t* ppuFramebuffer)
{
    // Bind texture and upload current PPU framebuffer
    glBindTexture(GL_TEXTURE_2D, gbTexture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 160, 144, GL_RGB, GL_UNSIGNED_BYTE, ppuFramebuffer);

    // TODO: draw textured quad fullscreen (or scaled)
    // For now, texture is ready for rendering
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Renderer::EndFrame()
{
    SDL_GL_SwapWindow(window);
}

void Renderer::Shutdown()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    if (glContext) 
    {
        SDL_GL_DestroyContext(glContext);
        glContext = nullptr;
    }
    if (gbTexture)
    {
        glDeleteTextures(1, &gbTexture);
        gbTexture = 0;
    }

}
