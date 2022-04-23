#include "glfw_module.h"
#include "gl_include.h"

#include <cassert>
#include <format>
#include <iostream>

#define ASSERT assert

namespace utils {

void ShowErrorMessage(char const* msg)
{
    std::cout << "ERROR: " << msg << "\n";
}

GLShaderProgram CreateAndLinkShaders(const char* vertex_shader_source, const char* fragment_shader_source)
{
    GLuint vertex_shader = 0;
    GLuint fragment_shader = 0;
    GLuint shader_program = 0;
    GLint success = 0;
    char info_log[512]{};

    // build and compile our shader program
    // ------------------------------------
    // vertex shader
    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_shader_source, nullptr);
    glCompileShader(vertex_shader);
    // check for shader compile errors
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertex_shader, 512, nullptr, info_log);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << info_log << "\n";
    }

    if (success)
    {
        // fragment shader
        fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment_shader, 1, &fragment_shader_source, nullptr);
        glCompileShader(fragment_shader);
        // check for shader compile errors
        glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(fragment_shader, 512, nullptr, info_log);
            std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << info_log << std::endl;
        }
    }

    if (success)
    {
        // link shaders
        shader_program = glCreateProgram();
        glAttachShader(shader_program, vertex_shader);
        glAttachShader(shader_program, fragment_shader);
        glLinkProgram(shader_program);
        // check for linking errors
        glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(shader_program, 512, nullptr, info_log);
            std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << info_log << std::endl;
        }
    }

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    if (!success)
    {
        glDeleteProgram(shader_program);
        shader_program = 0;
    }

    return shader_program;
}

GlfwModule::GlfwModule()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
}

GlfwModule::~GlfwModule()
{
    glfwTerminate();
}

bool GlfwModule::InitializeContext()
{
    ASSERT(!window_);
    // glfw window creation
    // --------------------
    window_ = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, nullptr, nullptr);
    if (!window_)
    {
        ShowErrorMessage("Failed to create GLFW window");
        return false;
    }

    glfwMakeContextCurrent(window_);
    glfwSetFramebufferSizeCallback(window_, FramebufferSizeCallback);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        ShowErrorMessage("Failed to initialize GLAD");
        return false;
    }

    return true;
}

void GlfwModule::RunMessageLoop(std::function<void(void)> render)
{
    // render loop
    // -----------
    while (!glfwWindowShouldClose(window_))
    {
        ProcessInput();

        // render
        // ------
        glClearColor(bkg_color_[0], bkg_color_[1], bkg_color_[2], 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        if (render)
        {
            render();
        }

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window_);
        glfwPollEvents();
    }
}

void GlfwModule::SetBackgroundColor(float red, float green, float blue)
{
    bkg_color_ = {red, green, blue};
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void GlfwModule::FramebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------
void GlfwModule::ProcessInput()
{
    if (glfwGetKey(window_, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window_, true);
    }
}

} // namespace utils