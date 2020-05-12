cmake_minimum_required(VERSION 2.8)
project(sampleCmake)

add_library(glad glad/src/glad.c)
target_include_directories(glad PRIVATE glad/include)

add_executable(opengl main.c)
target_include_directories(opengl PRIVATE glad/include)
find_package(glfw3)
target_link_libraries(opengl glfw GL X11 pthread Xrandr Xi dl glad ${CMAKE_DL_LIBS})
