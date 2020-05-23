#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "mesh.h"
#include "model.h"

#define SCR_WIDTH 800
#define SCR_HEIGHT 600

vec3 cameraPos = {0.0f, 0.0f, 3.0f};
vec3 cameraFront = {0.0f, 0.0f, -1.0f};
vec3 cameraUp = {0.0f, 1.0f, 0.0f};

bool firstMouse = true;
float yaw   = -90.0f; // yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction vector pointing to the right so we initially rotate a bit to the left.
float pitch =  0.0f;
float lastX = SCR_WIDTH / 2, lastY = SCR_HEIGHT / 2;
float fov   =  45.0f;

float deltaTime = 0.0f;	// Time between current frame and last frame
float lastFrame = 0.0f; // Time of last frame

vec3 lightPos = {1.2f, 1.0f, 2.0f};

void error_callback (int error, const char* description)
{
    printf("%s\n", description);
}

void framebuffer_size_callback (GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void processInput (GLFWwindow *window)
{
    float cameraSpeed = 2.5f * deltaTime;
    vec3 tmp, tmp2;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        glm_vec3_scale(cameraFront, cameraSpeed, tmp);
        glm_vec3_add(cameraPos, tmp, cameraPos);
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        glm_vec3_scale(cameraFront, cameraSpeed, tmp);
        glm_vec3_sub(cameraPos, tmp, cameraPos);
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        glm_vec3_cross(cameraFront, cameraUp, tmp);
        glm_vec3_normalize(tmp);
        glm_vec3_scale(tmp, cameraSpeed, tmp2);
        glm_vec3_sub(cameraPos, tmp2, cameraPos);
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        glm_vec3_cross(cameraFront, cameraUp, tmp);
        glm_vec3_normalize(tmp);
        glm_vec3_scale(tmp, cameraSpeed, tmp2);
        glm_vec3_add(cameraPos, tmp2, cameraPos);
    }
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

void mouse_callback (GLFWwindow* window, double xpos, double ypos)
{
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates range from bottom to top
    lastX = xpos;
    lastY = ypos;

    const float sensitivity = 0.05f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f) {
        pitch = 89.0f;
    }
    if (pitch < -89.0f) {
        pitch = -89.0f;
    }

    float x = cos(glm_rad(yaw)) * cos(glm_rad(pitch));
    float y = sin(glm_rad(pitch));
    float z = sin(glm_rad(yaw)) * cos(glm_rad(pitch));
    vec3 direction = {x, y, z};
    glm_vec3_normalize_to(direction, cameraFront);
}

void scroll_callback (GLFWwindow* window, double xoffset, double yoffset)
{
    if (fov >= 1.0f && fov <= 45.0f) {
        fov -= yoffset;
    }
    if (fov < 1.0f) {
        fov = 1.0f;
    }
    if (fov > 45.0f) {
        fov = 45.0f;
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

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
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

    //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    return window;
}

char * read_file (const char *path)
{
    FILE *f = fopen(path, "r");
    if (!f) {
        printf("Error opening file %s\n", path);
        exit(EXIT_FAILURE);
    }

    fseek(f, 0, SEEK_END);
    long length = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *buffer = malloc(length + 1);
    size_t read = fread(buffer, 1, length, f);
    fclose(f);

    buffer[read] = '\0';

    return buffer;
}

unsigned int createShader (const char *shaderPath, GLuint shaderType)
{
    char infoLog[512];
    int success;

    unsigned int shader = glCreateShader(shaderType);
    const char *shaderSource = read_file(shaderPath);

    glShaderSource(shader, 1, &shaderSource, NULL);
    glCompileShader(shader);

    glGetProgramiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, sizeof(infoLog), NULL, infoLog);
        printf("ERROR::SHADER::COMPILATION_FAILED\n%s\n%s\n", shaderPath, infoLog);
        exit(EXIT_FAILURE);
    }

    free((void *) shaderSource);

    return shader;
}

unsigned int createProgram (const char *vertexShaderPath, const char *fragmentShaderPath)
{
    char infoLog[512];
    int success;

    unsigned int vertexShader = createShader(vertexShaderPath, GL_VERTEX_SHADER);
    unsigned int fragmentShader = createShader(fragmentShaderPath, GL_FRAGMENT_SHADER);

    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, sizeof(infoLog), NULL, infoLog);
        printf("ERROR::SHADER::PROGRAM::LINKING_FAILED\n%s\n", infoLog);
        exit(EXIT_FAILURE);
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

int main (int argc, char *argv[])
{
    GLFWwindow *window = createWindow();

    glEnable(GL_DEPTH_TEST);

    unsigned int program = createProgram("model_loading/shader.vert", "model_loading/shader.frag");
    Model model = createModel("resources/backpack/backpack.obj");

    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        drawModel(&model, program);

        // transformations
        mat4 view, projection;
        vec3 center;
        glm_vec3_add(cameraPos, cameraFront, center);
        glm_lookat(cameraPos, center, cameraUp, view);
        glm_perspective(glm_rad(fov), (float) SCR_WIDTH / (float) SCR_HEIGHT, 0.1f, 100.0f, projection);

        glUniformMatrix4fv(glGetUniformLocation(program, "view"), 1, GL_FALSE, (float *) view);
        glUniformMatrix4fv(glGetUniformLocation(program, "projection"), 1, GL_FALSE, (float *) projection);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    //glDeleteVertexArrays(1, &cubeVAO);
    //glDeleteVertexArrays(1, &lightVAO);
    //glDeleteBuffers(1, &VBO);
    //glDeleteProgram(lightingShader);

    glfwTerminate();

    return EXIT_SUCCESS;
}
