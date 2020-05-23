cmake_minimum_required(VERSION 2.8)
project(learnOpenGL)

#SET(CMAKE_CXX_COMPILER "/usr/bin/clang++")

# Build GLAD
add_library(glad external/glad/src/glad.c)
target_include_directories(glad PRIVATE external/glad/include)

# Locate glfw3
find_package(glfw3)
# Build CGLM
add_subdirectory(external/cglm)
# Build Assimp
add_subdirectory(external/assimp)

#set(CMAKE_C_COMPILER "/usr/bin/clang")

add_executable(getting_started getting_started/main.c)
target_include_directories(getting_started PRIVATE external/glad/include external/stb)
target_link_libraries(getting_started glfw GL X11 pthread Xrandr Xi m glad ${CMAKE_DL_LIBS})

add_executable(lighting lighting/main.c)
target_include_directories(lighting PRIVATE external/glad/include external/stb)
target_link_libraries(lighting glfw GL X11 pthread Xrandr Xi m glad ${CMAKE_DL_LIBS})

add_executable(model_loading model_loading/main.c model_loading/mesh.h model_loading/model.h model_loading/shader.h)
target_include_directories(model_loading PRIVATE external/glad/include external/stb)
target_link_libraries(model_loading glfw GL X11 pthread Xrandr Xi m c glad assimp ${CMAKE_DL_LIBS})
#target_compile_options(model_loading PRIVATE -Wall -Wextra -Werror)
