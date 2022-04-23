#include "utils/glfw_module.h"
#include "utils/shader.h"

#include <iostream>

const char* const VERTEXT_SHADER_SOURCE = R"(
    #version 330 core
    layout (location = 0) in vec3 aPos;
    uniform float offset;
    void main()
    {
       gl_Position = vec4(aPos.x + offset, aPos.y, aPos.z, 1.0);
    }
)";

const char* const FRAGMENT_SHADER_SOURCE = R"(
    #version 330 core
    out vec4 FragColor;
    void main()
    {
        FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);
    }
)";

GLfloat vertices[] = {
    -0.5f, -0.5f, 0.0f, // left
    0.5f,  -0.5f, 0.0f, // right
    0.0f,  0.5f,  0.0f  // top
};

int main()
{
    auto module = utils::GlfwModule();
    if (!module.InitializeContext())
    {
        return -1;
    }

    utils::Shader shader{VERTEXT_SHADER_SOURCE, FRAGMENT_SHADER_SOURCE};

    GLuint vbo = 0;
    GLuint vao = 0;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);

    // note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound
    // vertex buffer object so afterwards we can safely unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // uncomment this call to draw in wireframe polygons.
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    GLfloat offset = -0.5f;
    GLfloat step = 0.005f;

    module.SetBackgroundColor(0.2f, 0.3f, 0.3f);
    module.RunMessageLoop([&shader, vao, &offset, &step] {
        offset += step;
        if (offset > 0.5f || offset < -0.5f)
        {
            step = -step;
        }

        shader.Use();
        shader.SetFloat("offset", offset);
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        // glBindVertexArray(0); // no need to unbind it every time
    });

    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);

    return 0;
}