#include "stb_image/stb_image.h"
#include "utils/file_path.h"
#include "utils/glfw_module.h"
#include "utils/shader.h"

#include <filesystem>
#include <iostream>

const char* const VERTEXT_SHADER_SOURCE = R"(
    #version 330 core
    layout (location = 0) in vec3 aPos;
    layout (location = 1) in vec2 aTexCoord;
    out vec2 TexCoord;

    void main()
    {
       gl_Position = vec4(aPos, 1.0);
       TexCoord = aTexCoord;
    }
)";

const char* const FRAGMENT_SHADER_SOURCE = R"(
    #version 330 core
    out vec4 FragColor;
    in vec2 TexCoord;
    uniform sampler2D texture1;
    uniform sampler2D texture2;

    void main()
    {
        FragColor = mix(texture(texture1, TexCoord), texture(texture2, TexCoord), 0.2);
    }
)";

// clang-format off
GLfloat vertices[] = {
    // positions         // texture
    0.5f,  0.5f, 0.0f,   1.0f, 1.0f,   // 右上
    0.5f, -0.5f, 0.0f,   1.0f, 0.0f,   // 右下
    -0.5f, -0.5f, 0.0f,  0.0f, 0.0f,   // 左下
    -0.5f,  0.5f, 0.0f,  0.0f, 1.0f    // 左上
};
// clang-format on

GLuint indices[] = {
    0, 1, 3, // first triangle
    1, 2, 3  // second triangle
};

int main()
{
    auto module = utils::GlfwModule();
    if (!module.InitializeContext())
    {
        return -1;
    }
    utils::Shader shader{VERTEXT_SHADER_SOURCE, FRAGMENT_SHADER_SOURCE};

    // tell stb_image.h to flip loaded texture's on the y-axis.
    stbi_set_flip_vertically_on_load(true);

    int width = 0;
    int height = 0;
    int channels = 0;
    std::string image_path = utils::GetExecutableDir() + "/assets/container.jpeg";
    unsigned char* image_data = stbi_load(image_path.c_str(), &width, &height, &channels, 0);
    if (!image_data)
    {
        std::cout << "ERROR: Load image failed: " << image_path << "\n";
        return -1;
    }

    // texture 1
    // ---------
    GLuint texture1 = 0;
    glGenTextures(1, &texture1);
    glBindTexture(GL_TEXTURE_2D, texture1);
    // 为当前绑定的纹理对象设置环绕、过滤方式
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // 加载并生成纹理
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image_data);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(image_data);
    image_data = nullptr;

    image_path = utils::GetExecutableDir() + "/assets/awesomeface.png";
    image_data = stbi_load(image_path.c_str(), &width, &height, &channels, 0);
    if (!image_data)
    {
        std::cout << "ERROR: Load image failed: " << image_path << "\n";
        return -1;
    }

    // texture 2
    // ---------
    GLuint texture2 = 0;
    glGenTextures(1, &texture2);
    glBindTexture(GL_TEXTURE_2D, texture2);
    // 为当前绑定的纹理对象设置环绕、过滤方式
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // 加载并生成纹理
    // note that the awesomeface.png has transparency and thus an alpha channel,
    // so make sure to tell OpenGL the data type is of GL_RGBA
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(image_data);
    image_data = nullptr;

    GLuint vbo = 0;
    GLuint vao = 0;
    GLuint ebo = 0;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    // note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound
    // vertex buffer object so afterwards we can safely unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // uncomment this call to draw in wireframe polygons.
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // 混合模式贴图对 mix 不生效，最终效果和教程里不同
    // glEnable(GL_BLEND);                                // 开混合模式贴图
    // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // 指定混合模式算法

    // tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
    // -------------------------------------------------------------------------------------------
    shader.Use(); // don't forget to activate/use the shader before setting uniforms!
    shader.SetInt("texture1", 0);
    shader.SetInt("texture2", 1);

    module.SetBackgroundColor(0.2f, 0.3f, 0.3f);
    module.RunMessageLoop([&shader, vao, texture1, texture2] {
        // bind textures on corresponding texture units
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture1);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texture2);

        // render container
        shader.Use();
        glBindVertexArray(vao);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    });

    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);

    return 0;
}