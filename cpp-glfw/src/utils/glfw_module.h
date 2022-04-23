#pragma once

#include "gl_include.h"
#include <array>
#include <functional>
#include <string>
#include <tuple>

namespace utils {

constexpr int WINDOW_WIDTH = 800;
constexpr int WINDOW_HEIGHT = 600;
constexpr char WINDOW_TITLE[] = "LearnOpenGL";

void ShowErrorMessage(char const* msg);

inline void ShowErrorMessage(std::string const& msg)
{
    ShowErrorMessage(msg.c_str());
}

using GLShaderProgram = GLuint;

GLShaderProgram CreateAndLinkShaders(const char* vertex_shader_source, const char* fragment_shader_source);

class GlfwModule
{
public:
    GlfwModule();
    ~GlfwModule();

    bool InitializeContext();
    void RunMessageLoop(std::function<void(void)> render);

    void SetBackgroundColor(float red, float green, float blue);

private:
    static void FramebufferSizeCallback(GLFWwindow* window, int width, int height);
    void ProcessInput();

private:
    GLFWwindow* window_ = nullptr;
    std::array<GLfloat, 3> bkg_color_{};
};

} // namespace utils