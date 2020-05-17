cmake_minimum_required(VERSION 2.8)
project(learnOpenGL)

# Build GLAD
add_library(glad external/glad/src/glad.c)
target_include_directories(glad PRIVATE external/glad/include)

# Locate glfw3 and build CGLM
find_package(glfw3)
add_subdirectory(external/cglm)

add_executable(getting_started getting_started/main.c)
target_include_directories(getting_started PRIVATE external/glad/include external/stb)
target_link_libraries(getting_started glfw GL X11 pthread Xrandr Xi m glad ${CMAKE_DL_LIBS})

add_executable(lighting lighting/main.c)
target_include_directories(lighting PRIVATE external/glad/include external/stb)
target_link_libraries(lighting glfw GL X11 pthread Xrandr Xi m glad ${CMAKE_DL_LIBS})
