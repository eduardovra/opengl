#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdbool.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>

//#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
//#include <cimgui.h>
//#include <imgui_impl_glfw.h>
// #include <imgui_impl_opengl3.h>

#include "camera.h"
#include "shader.h"
#include "mesh.h"
#include "model.h"
#include "light_cube_vertices.h"

#define SCR_WIDTH 800
#define SCR_HEIGHT 600

Camera camera;

float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

float deltaTime = 0.0f;	// Time between current frame and last frame
float lastFrame = 0.0f; // Time of last frame

struct ImGuiContext *ctx;
struct ImGuiIO *io;

void error_callback (int error, const char *description)
{
    printf("%s\n", description);
}

void framebuffer_size_callback (GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void mouse_callback (GLFWwindow *window, double xpos, double ypos)
{
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates range from bottom to top

    lastX = xpos;
    lastY = ypos;

    processMouseMovement(&camera, xoffset, yoffset);
}

void scroll_callback (GLFWwindow *window, double xoffset, double yoffset)
{
    processMouseScroll(&camera, yoffset);
}

void processInput (GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        processKeyboard(&camera, FORWARD, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        processKeyboard(&camera, BACKWARD, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        processKeyboard(&camera, LEFT, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        processKeyboard(&camera, RIGHT, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

GLFWwindow* createWindow ()
{
    glfwSetErrorCallback(error_callback);

    if (glfwInit() == GLFW_FALSE) {
        printf("Failed to initialize GLFW\n");
        exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL) {
        printf("Failed to create GLFW window\n");
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        printf("Failed to initialize GLAD\n");
        exit(EXIT_FAILURE);
    }

    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    return window;
}

void drawCube(unsigned int x, unsigned int y)
{
    mat4 model;
}

int main (int argc, char *argv[])
{
    initCamera(&camera);
    GLFWwindow *window = createWindow();

    /*
    ctx = igCreateContext(NULL);
    io  = igGetIO();

    const char *glsl_version = "#version 330 core";
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Setup style
    igStyleColorsDark(NULL);
    */

    glEnable(GL_DEPTH_TEST);

    // tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
    //stbi_set_flip_vertically_on_load(true);

    unsigned int lightProgram = createProgram("tetris/light_shader.vert", "tetris/light_shader.frag");

    // configure light cube
    unsigned int VBO, lightCubeVAO;
    glGenVertexArrays(1, &lightCubeVAO);
    glGenBuffers(1, &VBO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindVertexArray(lightCubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *) 0);
    glEnableVertexAttribArray(0);

    vec3 lightPos = {0.0f, -1.0f, 0.0f};

    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // wireframe mode
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        // view/projection transformations
        mat4 model, view, projection;
        getViewMatrix(&camera, view);
        glm_perspective(glm_rad(camera.fov), (float) SCR_WIDTH / (float) SCR_HEIGHT, 0.1f, 100.0f, projection);

        // draw point light
        glUseProgram(lightProgram);

        glm_mat4_identity(model);
        lightPos[1] += 1.0f * sin((float) glfwGetTime()) * deltaTime;
        glm_translate(model, lightPos);
        vec3 lightCubeSize = {0.2f, 0.2f, 0.2f};
        glm_scale(model, lightCubeSize);
        glUniformMatrix4fv(glGetUniformLocation(lightProgram, "view"), 1, GL_FALSE, (float *) view);
        glUniformMatrix4fv(glGetUniformLocation(lightProgram, "projection"), 1, GL_FALSE, (float *) projection);
        glUniformMatrix4fv(glGetUniformLocation(lightProgram, "model"), 1, GL_FALSE, (float *) model);

        glBindVertexArray(lightCubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        mat4 cubo2model;
        glm_mat4_identity(cubo2model);
        vec3 cubo2pos = {-1.0f, lightPos[1], lightPos[2]};
        glm_translate(cubo2model, cubo2pos);
        glm_scale(cubo2model, lightCubeSize);
        glUniformMatrix4fv(glGetUniformLocation(lightProgram, "model"), 1, GL_FALSE, (float *) cubo2model);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    //glDeleteVertexArrays(1, &cubeVAO);
    //glDeleteVertexArrays(1, &lightVAO);
    //glDeleteBuffers(1, &VBO);
    glDeleteProgram(lightProgram);

    glfwTerminate();

    return EXIT_SUCCESS;
}
