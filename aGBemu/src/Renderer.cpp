#include "Renderer.h"
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
Renderer::Renderer() : glContext(nullptr), window(nullptr), gbTexture(0), quadVAO(0), quadVBO(0), shaderProgram(0) {}
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
    InitFullscreenQuad();
    InitShaders();

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
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Shader compilation failed: %s", infoLog);
    }
    return shader;
}

void Renderer::InitShaders()
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
    }

    glDeleteShader(vertex);
    glDeleteShader(fragment);
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
    glBindTexture(GL_TEXTURE_2D, gbTexture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 160, 144, GL_RGB, GL_UNSIGNED_BYTE, ppuFramebuffer);

    glUseProgram(shaderProgram);
    glBindVertexArray(quadVAO);
    glBindTexture(GL_TEXTURE_2D, gbTexture);
    glDrawArrays(GL_TRIANGLES, 0, 6);
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

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    if (glContext)
    {
        SDL_GL_DestroyContext(glContext);
        glContext = nullptr;
    }
}
