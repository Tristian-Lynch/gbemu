#include <SDL3/SDL.h>
#include "Renderer.h"
#include "PPU.h"
#include <backends/imgui_impl_sdl3.h>
#include <backends/imgui_impl_opengl3.h>

// Helper functions
SDL_Window* InitSDL();
Renderer* InitRenderer(SDL_Window* window);
void Cleanup(SDL_Window* window, Renderer* renderer);
void MainLoop(SDL_Window* window, Renderer* renderer, PPU& ppu);

int main()
{
    SDL_Window* window = InitSDL();
    if (!window) return -1;

    Renderer* renderer = InitRenderer(window);
    if (!renderer) {
        SDL_DestroyWindow(window);
        return -1;
    }

    PPU ppu; // PPU instance

    MainLoop(window, renderer, ppu);
    Cleanup(window, renderer);

    return 0;
}

SDL_Window* InitSDL()
{
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL_Init failed: %s", SDL_GetError());
        return nullptr;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    SDL_Window* window = SDL_CreateWindow(
        "GameBoy Emulator",
        800, 600,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
    );

    if (!window) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL_CreateWindow failed: %s", SDL_GetError());
        SDL_Quit();
        return nullptr;
    }

    SDL_SetWindowPosition(window, 100, 100);

    return window;
}

Renderer* InitRenderer(SDL_Window* window)
{
    Renderer* renderer = new Renderer();
    if (!renderer->Init(window)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Renderer initialization failed");
        delete renderer;
        return nullptr;
    }
    return renderer;
}

void Cleanup(SDL_Window* window, Renderer* renderer)
{
    if (renderer) {
        renderer->Shutdown();
        delete renderer;
    }
    if (window) SDL_DestroyWindow(window);
    SDL_Quit();
}

void MainLoop(SDL_Window* window, Renderer* renderer, PPU& ppu)
{
    bool running = true;
    SDL_Event event;

    while (running) {
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL3_ProcessEvent(&event);
            if (event.type == SDL_EVENT_QUIT)
                running = false;
        }

        renderer->BeginFrame();

        // Render GameBoy framebuffer
        ppu.RenderFrame();
        renderer->RenderGameboyFrame(ppu.GetFramebuffer());

        // Render debug UI
        renderer->RenderUI(&ppu);

        renderer->EndFrame();
    }
}
