#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

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

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
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

unsigned int createTexture (const char *imagePath)
{
    unsigned int texture;

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    // set the texture wrapping/filtering options (on the currently bound texture object)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // load and generate the texture
    int width, height, nrChannels;
    unsigned char *data = stbi_load(imagePath, &width, &height, &nrChannels, 0);
    if (!data)
    {
        printf("Failed to load texture %s\n", imagePath);
        exit(EXIT_FAILURE);
    }

    GLenum format;
    if (nrChannels == 1) {
        format = GL_RED;
    }
    else if (nrChannels == 3) {
        format = GL_RGB;
    }
    else if (nrChannels == 4) {
        format = GL_RGBA;
    }
    else {
        printf("Unknown number of channels in %s: %d\n", imagePath, nrChannels);
        exit(EXIT_FAILURE);
    }

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(data);

    return texture;
}

int main (int argc, char *argv[])
{
    GLFWwindow *window = createWindow();

    glEnable(GL_DEPTH_TEST);

    unsigned int lightingShader = createProgram("lighting/lighting.vert", "lighting/lighting.frag");
    unsigned int lampShader = createProgram("lighting/lamp.vert", "lighting/lamp.frag");

    float vertices[] = {
        // positions          // normals           // texture coords
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,

        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,

        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,

        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f
    };

    // first, configure the cube's VAO (and VBO)
    unsigned int VBO, cubeVAO;
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &VBO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindVertexArray(cubeVAO);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *) 0);
    glEnableVertexAttribArray(0);
    // normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *) (3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // texture coordinates attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *) (6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // Load diffuse and specular maps texture
    unsigned int diffuseMap = createTexture("resources/container2.png");
    unsigned int specularMap = createTexture("resources/container2_specular.png");

    // second, configure the light's VAO (VBO stays the same; the vertices are the same for the light object which is also a 3D cube)
    unsigned int lightVAO;
    glGenVertexArrays(1, &lightVAO);
    glBindVertexArray(lightVAO);

    // we only need to bind to the VBO (to link it with glVertexAttribPointer), no need to fill it; the VBO's data already contains all we need (it's already bound, but we do it again for educational purposes)
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *) 0);
    glEnableVertexAttribArray(0);

    glUseProgram(lightingShader);
    glUniform1i(glGetUniformLocation(lightingShader, "material.diffuse"), 0);
    glUniform1i(glGetUniformLocation(lightingShader, "material.specular"), 1);

    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // draw the cube
        glUseProgram(lightingShader);
        float materialShininess = 64.0f;
        glUniform1f(glGetUniformLocation(lightingShader, "material.shininess"), materialShininess);

        // Bind texture units for diffuse and specular maps
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, diffuseMap);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, specularMap);

        vec3 lightAmbient = {0.2f, 0.2f, 0.2f};
        vec3 lightDiffuse = {0.5f, 0.5f, 0.5f};
        vec3 lightSpecular = {1.0f, 1.0f, 1.0f};
        glUniform3fv(glGetUniformLocation(lightingShader, "light.ambient"), 1, lightAmbient);
        glUniform3fv(glGetUniformLocation(lightingShader, "light.diffuse"), 1, lightDiffuse);
        glUniform3fv(glGetUniformLocation(lightingShader, "light.specular"), 1, lightSpecular);
        glUniform3fv(glGetUniformLocation(lightingShader, "light.position"), 1, lightPos);
        glUniform3fv(glGetUniformLocation(lightingShader, "viewPos"), 1, cameraPos);

        // transformations
        mat4 view, projection;
        vec3 center;
        glm_vec3_add(cameraPos, cameraFront, center);
        glm_lookat(cameraPos, center, cameraUp, view);
        glm_perspective(glm_rad(fov), (float) SCR_WIDTH / (float) SCR_HEIGHT, 0.1f, 100.0f, projection);

        glUniformMatrix4fv(glGetUniformLocation(lightingShader, "view"), 1, GL_FALSE, (float *) view);
        glUniformMatrix4fv(glGetUniformLocation(lightingShader, "projection"), 1, GL_FALSE, (float *) projection);

        mat4 model;
        glm_mat4_identity(model);
        glUniformMatrix4fv(glGetUniformLocation(lightingShader, "model"), 1, GL_FALSE, (float *) model);

        glBindVertexArray(cubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // draw the lamp object
        glUseProgram(lampShader);
        glUniformMatrix4fv(glGetUniformLocation(lampShader, "view"), 1, GL_FALSE, (float *) view);
        glUniformMatrix4fv(glGetUniformLocation(lampShader, "projection"), 1, GL_FALSE, (float *) projection);

        glm_mat4_identity(model);
        glm_translate(model, lightPos);
        vec3 scale = {0.2f, 0.2f, 0.2f};
        glm_scale(model, scale);
        glUniformMatrix4fv(glGetUniformLocation(lampShader, "model"), 1, GL_FALSE, (float *) model);

        glBindVertexArray(lightVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteVertexArrays(1, &lightVAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(lightingShader);
    glDeleteProgram(lampShader);

    glfwTerminate();

    return EXIT_SUCCESS;
}
