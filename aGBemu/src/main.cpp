#include <SDL3/SDL.h>
#include "Renderer.h"
#include "PPU.h"
#include "MMU.h"
#include "CPU.h"
#include <backends/imgui_impl_sdl3.h>
#include <backends/imgui_impl_opengl3.h>
#include <iostream>

// Helper functions
SDL_Window* InitSDL();
Renderer* InitRenderer(SDL_Window* window);
void Cleanup(SDL_Window* window, Renderer* renderer);
void MainLoop(SDL_Window* window, Renderer* renderer, CPU& cpu, PPU& ppu, MMU& mmu);

int main(int argc, char** argv)
{
    SDL_Window* window = InitSDL();
    if (!window) return -1;

    Renderer* renderer = InitRenderer(window);
    if (!renderer)
    {
        SDL_DestroyWindow(window);
        return -1;
    }

    // --- Emulator core initialization ---
    PPU ppu;
    MMU mmu(&ppu);
    
    // Optional ROM path from CLI; otherwise load via UI or drag-and-drop
    if (argc >= 2) {
        const char* romPath = argv[1];
        if (!mmu.LoadROMFromFile(romPath))
        {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to load ROM: %s", romPath);
        }
    }
    CPU cpu(&mmu);

    cpu.Reset();
    ppu.Reset();

    MainLoop(window, renderer, cpu, ppu, mmu);

    Cleanup(window, renderer);
    return 0;
}

SDL_Window* InitSDL()
{
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
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

    if (!window)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL_CreateWindow failed: %s", SDL_GetError());
        SDL_Quit();
        return nullptr;
    }

    return window;
}

Renderer* InitRenderer(SDL_Window* window)
{
    Renderer* renderer = new Renderer();
    if (!renderer->Init(window))
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Renderer initialization failed");
        delete renderer;
        return nullptr;
    }
    return renderer;
}

void Cleanup(SDL_Window* window, Renderer* renderer)
{
    if (renderer)
    {
        renderer->Shutdown();
        delete renderer;
    }
    if (window) SDL_DestroyWindow(window);
    SDL_Quit();
}

// --- Main emulator loop ---
void MainLoop(SDL_Window* window, Renderer* renderer, CPU& cpu, PPU& ppu, MMU& mmu)
{
    bool running = true;
    SDL_Event event;
    bool paused = !mmu.IsROMLoaded();
    uint32_t lastRomGen = mmu.GetROMLoadGeneration();

    const int cyclesPerFrame = 69905; // DMG CPU: ~4.19MHz / 60Hz
    uint64_t lastTime = SDL_GetTicks();

    while (running)
    {
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL3_ProcessEvent(&event);
            if (event.type == SDL_EVENT_QUIT)
                running = false;
            // Handle drag-and-drop of ROM files
            if (event.type == SDL_EVENT_DROP_FILE && event.drop.data)
            {
                const char* dropped = event.drop.data;
                if (!mmu.LoadROMFromFile(dropped))
                {
                    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to load ROM: %s", dropped);
                }
                else
                {
                    SDL_Log("Loaded ROM: %s", dropped);
                }
            }
        }

        // Detect newly loaded ROM and reset core, unpause
        uint32_t gen = mmu.GetROMLoadGeneration();
        if (gen != lastRomGen)
        {
            lastRomGen = gen;
            cpu.Reset();
            ppu.Reset();
            paused = false;
        }

        int cyclesThisFrame = 0;
        if (!paused)
        {
            while (cyclesThisFrame < cyclesPerFrame)
            {
                cyclesThisFrame += cpu.Step();
            }
        }

        // Render
        if (!paused)
        {
            ppu.RenderFrame();
        }
        renderer->BeginFrame();
        renderer->RenderGameboyFrame(ppu.GetFramebuffer());
        renderer->RenderUI(&ppu, &cpu, &mmu, &paused);
        renderer->EndFrame();

        // Frame limiting to ~60Hz
        uint64_t now = SDL_GetTicks();
        uint64_t delta = now - lastTime;
        if (delta < 16) SDL_Delay(16 - delta); // ~16ms per frame
        lastTime = SDL_GetTicks();
    }
}
