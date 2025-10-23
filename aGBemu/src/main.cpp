#include <SDL3/SDL.h>
#include "Renderer.h"
#include <backends/imgui_impl_sdl3.h>
#include <backends/imgui_impl_opengl3.h>

// Helper functions
SDL_Window* InitSDL();
Renderer* InitRenderer(SDL_Window* window);
void Cleanup(SDL_Window* window, Renderer* renderer);
void MainLoop(SDL_Window* window, Renderer* renderer);

int main()
{
    SDL_Window* window = InitSDL();
    if (!window) return -1;

    Renderer* renderer = InitRenderer(window);
    if (!renderer) {
        SDL_DestroyWindow(window);
        return -1;
    }

    MainLoop(window, renderer);
    Cleanup(window, renderer);

    return 0;
}

SDL_Window* InitSDL()
{
    // SDL3 returns SDL_bool; SDL_FALSE = failure
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL_Init failed: %s", SDL_GetError());
        return nullptr;
    }

    // OpenGL 3.3 core
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    // SDL3 window: width, height, flags
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

void MainLoop(SDL_Window* window, Renderer* renderer)
{
    bool running = true;
    SDL_Event event;

    while (running) {
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL3_ProcessEvent(&event);  // SDL3 ImGui backend
            if (event.type == SDL_EVENT_QUIT)
                running = false;
        }

        renderer->BeginFrame();
        renderer->RenderUI();  // Optional: pass PPU pointer later
        renderer->EndFrame();
    }
}
