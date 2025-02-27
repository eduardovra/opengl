#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdbool.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>

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

vec3 lightPos = {1.2f, 1.0f, 2.0f};

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

int main (int argc, char *argv[])
{
    initCamera(&camera);
    GLFWwindow *window = createWindow();

    glEnable(GL_DEPTH_TEST);

    // tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
    stbi_set_flip_vertically_on_load(true);

    unsigned int program = createProgram("asteroids/shader.vert", "asteroids/shader.frag");
    unsigned int asteroidsProgram = createProgram("asteroids/asteroids_shader.vert", "asteroids/shader.frag");
    unsigned int lightProgram = createProgram("asteroids/light_shader.vert", "asteroids/light_shader.frag");
    Model planet = createModel("resources/planet/planet.obj");
    Model rock = createModel("resources/rock/rock.obj");

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

    unsigned int amount = 100000;
    mat4 *modelMatrices = calloc(amount, sizeof(mat4));
    srand(glfwGetTime()); // initialize random seed
    float radius = 50.0;
    float offset = 2.5f;
    for (unsigned int i = 0; i < amount; i++)
    {
        mat4 model;
        glm_mat4_identity(model);
        // 1. translation: displace along circle with 'radius' in range [-offset, offset]
        float angle = (float) i / (float) amount * 360.0f;
        float displacement = (rand() % (int) (2 * offset * 100)) / 100.0f - offset;
        float x = sin(angle) * radius + displacement;
        displacement = (rand() % (int) (2 * offset * 100)) / 100.0f - offset;
        float y = displacement * 0.4f; // keep height of field smaller compared to width of x and z
        displacement = (rand() % (int) (2 * offset * 100)) / 100.0f - offset;
        float z = cos(angle) * radius + displacement;
        vec3 t = {x, y, z};
        glm_translate(model, t);

        // 2. scale: scale between 0.05 and 0.25f
        float scale = (rand() % 20) / 100.0f + 0.05;
        vec3 s = {scale, scale, scale};
        glm_scale(model, s);

        // 3. rotation: add random rotation around a (semi)randomly picked rotation axis vector
        float rotAngle = (rand() % 360);
        vec3 r = {0.4f, 0.6f, 0.8f};
        glm_rotate(model, rotAngle, r);

        // 4. now add to list of matrices
        memcpy(modelMatrices[i], model, sizeof(model));
    }

    // vertex buffer object
    unsigned int buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, amount * sizeof(mat4), modelMatrices, GL_STATIC_DRAW);

    for (unsigned int i = 0; i < rock.numMeshes; i++) {
        unsigned int VAO = rock.meshes[i].VAO;
        glBindVertexArray(VAO);
        // vertex attributes
        size_t vec4Size = sizeof(vec4);
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void *) 0);
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void *) (1 * vec4Size));
        glEnableVertexAttribArray(5);
        glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void *) (2 * vec4Size));
        glEnableVertexAttribArray(6);
        glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void *) (3 * vec4Size));

        glVertexAttribDivisor(3, 1);
        glVertexAttribDivisor(4, 1);
        glVertexAttribDivisor(5, 1);
        glVertexAttribDivisor(6, 1);

        glBindVertexArray(0);
    }

    free(modelMatrices);

    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(program);

        // wireframe mode
        //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        // light properties
        glUniform3fv(glGetUniformLocation(program, "viewPos"), 1, camera.cameraPos);
        glUniform3fv(glGetUniformLocation(program, "light.position"), 1, lightPos);
        vec3 lightAmbient = {0.2f, 0.2f, 0.2f};
        vec3 lightDiffuse = {0.5f, 0.5f, 0.5f};
        vec3 lightSpecular = {1.0f, 1.0f, 1.0f};
        glUniform3fv(glGetUniformLocation(program, "light.ambient"), 1, lightAmbient);
        glUniform3fv(glGetUniformLocation(program, "light.diffuse"), 1, lightDiffuse);
        glUniform3fv(glGetUniformLocation(program, "light.specular"), 1, lightSpecular);

        // view/projection transformations
        mat4 view, projection;
        getViewMatrix(&camera, view);
        glm_perspective(glm_rad(camera.fov), (float) SCR_WIDTH / (float) SCR_HEIGHT, 0.1f, 100.0f, projection);
        glUniformMatrix4fv(glGetUniformLocation(program, "view"), 1, GL_FALSE, (float *) view);
        glUniformMatrix4fv(glGetUniformLocation(program, "projection"), 1, GL_FALSE, (float *) projection);

        // render the loaded model
        mat4 modelMatrix;
        glm_mat4_identity(modelMatrix);
        vec3 auxTranslate = {0.0f, -3.0f, 0.0f};
        glm_translate(modelMatrix, auxTranslate);
        vec3 auxScale = {4.0f, 4.0f, 4.0f};
        glm_scale(modelMatrix, auxScale);
        glUniformMatrix4fv(glGetUniformLocation(program, "model"), 1, GL_FALSE, (float *) modelMatrix);
        drawModel(&planet, program);

        // draw meteorites
        glUseProgram(asteroidsProgram);
        glUniformMatrix4fv(glGetUniformLocation(asteroidsProgram, "view"), 1, GL_FALSE, (float *) view);
        glUniformMatrix4fv(glGetUniformLocation(asteroidsProgram, "projection"), 1, GL_FALSE, (float *) projection);
        glUniform1d(glGetUniformLocation(asteroidsProgram, "texture_diffuse1"), 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, rock.loadedTextures[0].id);
        for (unsigned int i = 0; i < rock.numMeshes; i++) {
            glBindVertexArray(rock.meshes[i].VAO);
            glDrawElementsInstanced(
                GL_TRIANGLES, rock.meshes[i].numIndices, GL_UNSIGNED_INT, 0, amount
            );
            glBindVertexArray(0);
        }

        // draw point light
        glUseProgram(lightProgram);
        glUniformMatrix4fv(glGetUniformLocation(lightProgram, "view"), 1, GL_FALSE, (float *) view);
        glUniformMatrix4fv(glGetUniformLocation(lightProgram, "projection"), 1, GL_FALSE, (float *) projection);
        glm_mat4_identity(modelMatrix);
        glm_translate(modelMatrix, lightPos);
        vec3 lightCubeSize = {0.2f, 0.2f, 0.2f};
        glm_scale(modelMatrix, lightCubeSize);
        glUniformMatrix4fv(glGetUniformLocation(program, "model"), 1, GL_FALSE, (float *) modelMatrix);

        glBindVertexArray(lightCubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    //glDeleteVertexArrays(1, &cubeVAO);
    //glDeleteVertexArrays(1, &lightVAO);
    //glDeleteBuffers(1, &VBO);
    glDeleteProgram(program);
    glDeleteProgram(lightProgram);

    glfwTerminate();

    return EXIT_SUCCESS;
}
