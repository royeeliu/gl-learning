#include "shader.h"

#include <cassert>
#include <iostream>

#define ASSERT assert

namespace utils {

Shader::Shader(const char* vertex_shader_source, const char* fragment_shader_source)
{
    ASSERT(vertex_shader_source);
    ASSERT(fragment_shader_source);

    GLuint vertex_shader = 0;
    GLuint fragment_shader = 0;

    // build and compile our shader program
    // ------------------------------------
    // vertex shader
    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_shader_source, nullptr);
    glCompileShader(vertex_shader);
    CheckCompileErrors(vertex_shader);

    // fragment shader
    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_source, nullptr);
    glCompileShader(fragment_shader);
    CheckCompileErrors(fragment_shader);

    // link shaders
    program_ = glCreateProgram();
    glAttachShader(program_, vertex_shader);
    glAttachShader(program_, fragment_shader);
    glLinkProgram(program_);
    CheckLinkErrors(program_);

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
}

Shader::~Shader()
{
    glDeleteProgram(program_);
    program_ = 0;
}

void Shader::Use()
{
    glUseProgram(program_);
}

void Shader::SetBool(const char* name, GLboolean value)
{
    glUniform1i(glGetUniformLocation(program_, name), static_cast<GLint>(value));
}

void Shader::SetInt(const char* name, GLint value)
{
    glUniform1i(glGetUniformLocation(program_, name), value);
}

void Shader::SetFloat(const char* name, GLfloat value)
{
    glUniform1f(glGetUniformLocation(program_, name), value);
}

GLuint Shader::GetUniformLocation(const char* name)
{
    return glGetUniformLocation(program_, name);
}

void Shader::CheckCompileErrors(GLuint shader)
{
    GLint success = 0;
    char info_log[512]{};
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(shader, 512, nullptr, info_log);
        std::cout << "ERROR::SHADER::COMPILATION_FAILED\n" << info_log << "\n";
    }
}

void Shader::CheckLinkErrors(GLuint program)
{
    GLint success = 0;
    char info_log[512]{};
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(program, 512, nullptr, info_log);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << info_log << std::endl;
    }
}

} // namespace utils