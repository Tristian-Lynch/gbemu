#pragma once

// If Windows.h is already defined.. like in Emulator.h.. only include SDL3. Testing Purposes.
#ifdef WIN32
#include <Windows.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_opengl.h>
#else
#include <SDL3/SDL.h>
#include <SDL3/SDL_opengl.h>
#endif