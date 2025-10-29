#include "Renderer.h"
#include "CPU.h"
#include <imgui.h>
#include <backends/imgui_impl_sdl3.h>
#include <backends/imgui_impl_opengl3.h>
#include <SDL3/SDL_log.h>

// Simple vertex & fragment shaders for fullscreen quad
static const char* vertexShaderSrc = R"(
#version 330 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aTexCoord;
out vec2 TexCoord;
void main() {
    gl_Position = vec4(aPos, 0.0, 1.0);
    TexCoord = aTexCoord;
})";

static const char* fragmentShaderSrc = R"(
#version 330 core
out vec4 FragColor;
in vec2 TexCoord;
uniform sampler2D screenTexture;
void main() {
    FragColor = texture(screenTexture, TexCoord);
})";

// Constructor / Destructor
Renderer::Renderer() : glContext(nullptr), window(nullptr), gbTexture(0), quadVAO(0), quadVBO(0), shaderProgram(0), imguiInitialized(false) {}
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

    // Enable VSync
    SDL_GL_SetSwapInterval(1);

    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to initialize GLAD");
        return false;
    }

    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "OpenGL version: %s", glGetString(GL_VERSION));

    // ImGui setup
    if (!imguiInitialized) {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui::StyleColorsDark();
        ImGui_ImplSDL3_InitForOpenGL(window, glContext);
        ImGui_ImplOpenGL3_Init("#version 330");
        imguiInitialized = true;
    }

    InitFramebufferTexture();
    InitFullscreenQuad();
    if (!InitShaders()) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Shader initialization failed");
        return false;
    }

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

void Renderer::InitFullscreenQuad()
{
    float quadVertices[] = {
        // positions   // texCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };

    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

GLuint Renderer::CompileShader(const char* source, GLenum type)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s shader compile error: %s",
            type == GL_VERTEX_SHADER ? "Vertex" : "Fragment", infoLog);
    }
    return shader;
}

bool Renderer::InitShaders()
{
    GLuint vertex = CompileShader(vertexShaderSrc, GL_VERTEX_SHADER);
    GLuint fragment = CompileShader(fragmentShaderSrc, GL_FRAGMENT_SHADER);

    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertex);
    glAttachShader(shaderProgram, fragment);
    glLinkProgram(shaderProgram);

    int success;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Shader linking failed: %s", infoLog);
        glDeleteShader(vertex);
        glDeleteShader(fragment);
        return false;
    }

    glDeleteShader(vertex);
    glDeleteShader(fragment);

    // Set the texture sampler to unit 0
    glUseProgram(shaderProgram);
    GLint loc = glGetUniformLocation(shaderProgram, "screenTexture");
    if (loc >= 0) glUniform1i(loc, 0);
    glUseProgram(0);

    return true;
}

void Renderer::BeginFrame()
{
    int w, h;
    SDL_GetWindowSize(window, &w, &h);
    glViewport(0, 0, w, h);

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    glClearColor(0.1f, 0.12f, 0.15f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

void Renderer::RenderUI(PPU* ppu, CPU* cpu)
{
    ImGui::Begin("GameBoy Emulator");

    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);

    // Show only registers A and B from CPU
    if (cpu) {
        ImGui::Text("Registers (Test Program):");
        ImGui::Text("A: 0x%02X", cpu->GetA());
        ImGui::Text("B: 0x%02X", cpu->GetB());
    }

    // Optional: PPU placeholder
    if (ppu) {
        ImGui::Text("PPU framebuffer placeholder");
    }

    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Renderer::RenderGameboyFrame(uint8_t* ppuFramebuffer)
{
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gbTexture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 160, 144, GL_RGB, GL_UNSIGNED_BYTE, ppuFramebuffer);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

    glUseProgram(shaderProgram);
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glBindVertexArray(0);
    glUseProgram(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Renderer::EndFrame()
{
    SDL_GL_SwapWindow(window);
}

void Renderer::Shutdown()
{
    if (shaderProgram) glDeleteProgram(shaderProgram);
    if (quadVBO) glDeleteBuffers(1, &quadVBO);
    if (quadVAO) glDeleteVertexArrays(1, &quadVAO);
    if (gbTexture) glDeleteTextures(1, &gbTexture);

    if (imguiInitialized) {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplSDL3_Shutdown();
        ImGui::DestroyContext();
        imguiInitialized = false;
    }

    if (glContext) {
        SDL_GL_DestroyContext(glContext);
        glContext = nullptr;
    }
}
