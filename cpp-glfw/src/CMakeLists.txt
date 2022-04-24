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
add_subdirectory(stb_image)

add_executable(triangle triangle.cc)
add_executable(triangle-moving triangle_moving.cc)
add_executable(triangle-color triangle_color.cc)
add_executable(texture texture.cc)

target_link_libraries(triangle glfw3 glad utils)
target_link_libraries(triangle-moving glfw3 glad utils)
target_link_libraries(triangle-color glfw3 glad utils)
target_link_libraries(texture glfw3 glad utils stb_image)

# 拷贝 assets 文件夹
add_custom_target(copy_assets ALL  
  VERBATIM 
  # COMMAND_EXPAND_LISTS 
  COMMAND ${CMAKE_COMMAND} -E make_directory "assets" 
  COMMAND ${CMAKE_COMMAND} -E 
                  copy_directory  
                  "${CMAKE_CURRENT_SOURCE_DIR}/assets" 
                  "assets" 
) 