#include "Renderer.h"
#include <glad.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_opengl.h>
#include <imgui.h>
#include <backends/imgui_impl_sdl3.h>
#include <backends/imgui_impl_opengl3.h>

// Constructor / Destructor
Renderer::Renderer() : glContext(nullptr) {}
Renderer::~Renderer() { Shutdown(); }

// Initialize SDL3 OpenGL context + GLAD + ImGui
bool Renderer::Init(SDL_Window* window)
{
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

    return true;
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

void Renderer::EndFrame()
{
    SDL_GL_SwapWindow(SDL_GL_GetCurrentWindow());
}

void Renderer::Shutdown()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    if (glContext) {
        SDL_GL_DestroyContext(glContext);
        glContext = nullptr;
    }
}
