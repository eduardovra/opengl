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
# Im GUI
#add_subdirectory(external/cimgui)

set(CMAKE_VERBOSE_MAKEFILE ON)
#set(CMAKE_C_COMPILER "/usr/bin/clang")

#set(cimgui external/cimgui)
#set(imgui_impl ${cimgui}/imgui/examples)
#file(GLOB
#	cimgui_files
#	"${cimgui}/*.cpp" 
#	"${cimgui}/imgui/*.cpp"
#	"${imgui_impl}/imgui_impl_glfw.cpp"
#	"${imgui_impl}/imgui_impl_opengl3.cpp")

#add_library(cimgui ${cimgui_files})
#target_include_directories(cimgui PRIVATE ${cimgui} ${cimgui}/imgui ${imgui_impl} external/glad/include)
#target_compile_definitions(cimgui PRIVATE
#	IMGUI_IMPL_API=extern\ \"C\"
#	IMGUI_IMPL_OPENGL_LOADER_GLAD)

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

add_executable(asteroids asteroids/main.c asteroids/mesh.h asteroids/model.h asteroids/shader.h asteroids/camera.h)
target_include_directories(asteroids PRIVATE external/glad/include external/stb)
target_link_libraries(asteroids glfw GL X11 pthread Xrandr Xi m c glad assimp ${CMAKE_DL_LIBS})
#target_compile_options(asteroids PRIVATE -Wall -Wextra -Werror)

add_executable(tetris tetris/main.c tetris/mesh.h tetris/model.h tetris/shader.h tetris/camera.h)
target_include_directories(tetris PRIVATE external/glad/include external/stb)
target_link_libraries(tetris glfw GL X11 pthread Xrandr Xi m c glad assimp ${CMAKE_DL_LIBS})
#target_compile_definitions(tetris PRIVATE
#	IMGUI_IMPL_API=\ )
