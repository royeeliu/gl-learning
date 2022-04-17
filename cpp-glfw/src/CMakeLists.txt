cmake_minimum_required(VERSION 3.23)

project(LearningOpenGLwithGLFW)
set(CMAKE_CXX_STANDARD 20)

include_directories(./glad/include)
include_directories(/usr/local/include)

link_directories(/usr/local/lib)
link_libraries("-framework Cocoa -framework OpenGL -framework IOKit -framework CoreVideo")

# 添加 math 子目录
add_subdirectory(glad)

add_executable(triangle triangle.cc)

target_link_libraries(triangle glfw3 glad)