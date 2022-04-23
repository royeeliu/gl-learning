#pragma once

#include "gl_include.h"

namespace utils {

class Shader
{
public:
    Shader(const char* vertex_shader_source, const char* fragment_shader_source);
    ~Shader();

    Shader(Shader const&) = delete;
    Shader& operator=(Shader const&) = delete;

    void Use();
    void SetBool(const char* name, GLboolean value);
    void SetInt(const char* name, GLint value);
    void SetFloat(const char* name, GLfloat value);

private:
    void CheckCompileErrors(GLuint shader);
    void CheckLinkErrors(GLuint program);

private:
    GLuint program_ = 0;
};

} // namespace utils