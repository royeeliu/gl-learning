cmake_minimum_required(VERSION 3.23)

project(LearningOpenGLwithGLFW)
set(CMAKE_CXX_STANDARD 20)

include_directories(./glad/include)
include_directories(/usr/local/include)

link_directories(/usr/local/lib)
link_libraries("-framework Cocoa -framework OpenGL -framework IOKit -framework CoreVideo")

# 添加子目录
add_subdirectory(glad)
add_subdirectory(utils)

add_executable(triangle triangle.cc)
add_executable(triangle-moving triangle_moving.cc)
add_executable(triangle-color triangle_color.cc)

target_link_libraries(triangle glfw3 glad utils)
target_link_libraries(triangle-moving glfw3 glad utils)
target_link_libraries(triangle-color glfw3 glad utils)