#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>
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
#include "pieces.h"

#define SCR_WIDTH 800
#define SCR_HEIGHT 600

#define BOARD_ROWS 22
#define BOARD_COLS 12

Camera camera;

float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

float deltaTime = 0.0f;	// Time between current frame and last frame
float lastFrame = 0.0f; // Time of last frame

//struct ImGuiContext *ctx;
//struct ImGuiIO *io;

typedef struct {
    int type;
    int color;
    int rotation;
    struct {
        int row;
        int col;
    } position;
} tPiece;

typedef enum {
    COLOR_BLACK = 0,
    COLOR_GREY = 1,
    COLOR_RED = 2,
    COLOR_GREEN = 3,
    COLOR_BLUE = 4,
} colors;

vec3 colorsDef[] = {
    {0.0f, 0.0f, 0.0f}, // black
    {0.5f, 0.5f, 0.5f}, // grey
    {1.0f, 0.0f, 0.0f}, // red
    {0.0f, 1.0f, 0.0f}, // green
    {0.0f, 0.0f, 1.0f}, // blue
};

float cubeSpeed = 0.2f;
unsigned int board[BOARD_ROWS][BOARD_COLS];
tPiece currentPiece;

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

int getRandom (int start, int end)
{
    return rand () % (end - start + 1) + start;
}

bool pieceCanMove (int rotation, int x, int y)
{
    // calculate new blocks position
    for (unsigned int blockX = 0; blockX < 5; blockX++) {
        for (unsigned int blockY = 0; blockY < 5; blockY++) {
            if (GetBlockType(currentPiece.type, currentPiece.rotation, blockX, blockY)) {
                int newX = blockX + x;
                int newY = blockY + y;
                //printf("collision newX: %d newY: %d\n", newX, newY);
                if (newX >= 0 && newY >= 0 && newX < BOARD_ROWS && newY < BOARD_COLS) {
                    if (board[newX][newY]) {
                        return false;
                    }
                }
            }
        }
    }

    return true;
}

bool paintCollision ()
{
    // calculate new blocks position
    for (unsigned int blockX = 0; blockX < 5; blockX++) {
        for (unsigned int blockY = 0; blockY < 5; blockY++) {
            if (GetBlockType(currentPiece.type, currentPiece.rotation, blockX, blockY)) {
                int newX = blockX + currentPiece.position.row;
                int newY = blockY + currentPiece.position.col;
                if (newX >= 0 && newY >= 0 && newX < BOARD_ROWS && newY < BOARD_COLS) {
                    if (board[newX][newY]) {
                        board[newX][newY] = COLOR_GREEN;
                    }
                }
            }
        }
    }
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

    static bool moveLeft = true, moveRight = true;

    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
        if (moveLeft) {
            if (pieceCanMove(currentPiece.rotation, currentPiece.position.row, currentPiece.position.col - 1)) {
                currentPiece.position.col -= 1;
            }
            moveLeft = false;
            printf("row: %d col: %d\n", currentPiece.position.row, currentPiece.position.col);
        }
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_RELEASE) {
        moveLeft = true;
    }

    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
        if (moveRight) {
            if (pieceCanMove(currentPiece.rotation, currentPiece.position.row, currentPiece.position.col + 1)) {
                currentPiece.position.col += 1;
            }
            moveRight = false;
            printf("row: %d col: %d\n", currentPiece.position.row, currentPiece.position.col);
        }
    }
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_RELEASE) {
        moveRight = true;
    }

    static bool moveDown = true, moveUp = true;

    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
        if (moveDown) {
            if (pieceCanMove(currentPiece.rotation, currentPiece.position.row + 1, currentPiece.position.col)) {
                currentPiece.position.row += 1;
            }
            moveDown = false;
            printf("row: %d col: %d\n", currentPiece.position.row, currentPiece.position.col);
        }
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_RELEASE) {
        moveDown = true;
    }

    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
        if (moveUp) {
            if (pieceCanMove(currentPiece.rotation, currentPiece.position.row - 1, currentPiece.position.col)) {
                currentPiece.position.row -= 1;
            }
            moveUp = false;
            printf("row: %d col: %d\n", currentPiece.position.row, currentPiece.position.col);
        }
    }
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_RELEASE) {
        moveUp = true;
    }

    //if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
    //    cubeSpeed += 0.01f;
    //}

    static bool rotate = true;
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        if (rotate) {
            int newRotation = currentPiece.rotation + 1;
            if (newRotation == 4) {
                newRotation = 0;
            }
            if (pieceCanMove(newRotation, currentPiece.position.row, currentPiece.position.col)) {
                currentPiece.rotation = newRotation;
            }
            rotate = false;
            printf("rotation: %d\n", currentPiece.rotation);
        }
    }

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE) {
        rotate = true;
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

    //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    return window;
}

void drawCube (unsigned int program, float x, float y, float *color)
{
    mat4 model;
    vec3 pos = {x, y, 0.0f};

    glm_mat4_identity(model);
    glm_translate(model, pos);
    glUniformMatrix4fv(glGetUniformLocation(program, "model"), 1, GL_FALSE, (float *) model);
    glUniform3fv(glGetUniformLocation(program, "color"), 1, color);

    glDrawArrays(GL_TRIANGLES, 0, 36);
}

void drawCubeInBoard (unsigned int program, int row, int col, float *color)
{
    float x = col - (BOARD_COLS / 2);
    float y = (BOARD_ROWS / 2) - row;
    drawCube(program, x, y, color);
}

void initBoard ()
{
    /*
    memset(board, COLOR_BLACK, sizeof(board));

    // lateral columns
    for (unsigned int x = 0; x < BOARD_COLS; x++) {
        board[x][0] = COLOR_GREY;
        board[x][BOARD_ROWS - 1] = COLOR_GREY;
    }

    // upper and bottom rows
    for (unsigned int y = 0; y < BOARD_ROWS; y++) {
        board[0][y] = COLOR_GREY;
        board[BOARD_COLS - 1][y] = COLOR_GREY;
    }
    */

    memset(board, COLOR_BLACK, sizeof(board));

    unsigned int b[BOARD_ROWS][BOARD_COLS] = {
        {1,1,1,1,1,1,1,1,1,1,1,1},
        {1,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,1},
        {1,1,1,1,1,1,1,1,1,1,1,1},
    };

    memcpy(board, b, sizeof(b));
}

void renderBoard (unsigned int program)
{
    for (int row = 0; row < BOARD_ROWS; row++) {
        for (int col = 0; col < BOARD_COLS; col++) {
            int piece = board[row][col];
            if (piece) {
                float *color = colorsDef[piece];
                drawCubeInBoard(program, row, col, color);
            }
        }
    }
}

void renderPiece (unsigned int program)
{
    for (int row = 0; row < 5; row++) {
        for (int col = 0; col < 5; col++) {
            if (GetBlockType(currentPiece.type, currentPiece.rotation, row, col)) {
                float *color = colorsDef[currentPiece.color];
                //if (GetBlockType(currentPiece.type, currentPiece.rotation, row, col) == 2) {
                //    vec3 highlight = {1.0f, 1.0f, 0.0f};
                //    color = highlight;
                //}
                drawCubeInBoard(program, row + currentPiece.position.row, col + currentPiece.position.col, color);
            }
            //else {
            //    vec3 color = {1.0f, 1.0f, 1.0f};
            //    drawCubeInBoard(program, row + currentPiece.position.row, col + currentPiece.position.col, color);
            //}
        }
    }
}

void addCurrentPieceToBoard ()
{
    for (int row = 0; row < 5; row++) {
        for (int col = 0; col < 5; col++) {
            if (GetBlockType(currentPiece.type, currentPiece.rotation, row, col)) {
                board[row + currentPiece.position.row][col + currentPiece.position.col] = currentPiece.color;
            }
        }
    }
}

void spawnPiece ()
{
    currentPiece.color = getRandom(COLOR_RED, COLOR_BLUE);
    currentPiece.rotation = getRandom(0, 3);
    currentPiece.type = getRandom(0, 6);
    //currentPiece.position.row = GetXInitialPosition(currentPiece.type, currentPiece.rotation);
    //currentPiece.position.col = GetYInitialPosition(currentPiece.type, currentPiece.rotation);
    currentPiece.position.row = -1;
    currentPiece.position.col = 5;
}

void checkDeleteLines ()
{
    for (int row = BOARD_ROWS - 2; row >= 1; row--) {
        bool delete = true;
        for (int col = 1; col < BOARD_COLS - 1; col++) {
            if (!board[row][col]) {
                delete = false;
                break;
            }
        }

        if (delete) {
            for (int col = 1; col < BOARD_COLS - 1; col++) {
                board[row][col] = COLOR_BLACK;
            }
        }
    }
}

int main (int argc, char *argv[])
{
    // Init random numbers
    srand ((unsigned int) time(NULL));

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

    initBoard();
    spawnPiece();
    float elapsedTime = 0.0f;

    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // wireframe mode
        //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        // view/projection transformations
        mat4 view, projection;
        getViewMatrix(&camera, view);
        glm_perspective(glm_rad(camera.fov), (float) SCR_WIDTH / (float) SCR_HEIGHT, 0.1f, 100.0f, projection);

        glUniformMatrix4fv(glGetUniformLocation(lightProgram, "view"), 1, GL_FALSE, (float *) view);
        glUniformMatrix4fv(glGetUniformLocation(lightProgram, "projection"), 1, GL_FALSE, (float *) projection);

        glUseProgram(lightProgram);
        glBindVertexArray(lightCubeVAO);

        // Piece fall down
        elapsedTime += deltaTime;
        if (elapsedTime > 1.5f) { // 500 ms
            elapsedTime = 0.0f;

            if (pieceCanMove(currentPiece.rotation, currentPiece.position.row + 1, currentPiece.position.col)) {
                currentPiece.position.row += 1;
                //printf("row: %d col: %d\n", currentPiece.position.row, currentPiece.position.col);
            }
            else {
                // Collision with ground detected
                // add piece to board
                addCurrentPieceToBoard();
                // check if theres lines to be deleted
                checkDeleteLines();
                // spawn a new one
                spawnPiece();
            }
        }

        paintCollision();

        renderBoard(lightProgram);
        renderPiece(lightProgram);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &lightCubeVAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(lightProgram);

    glfwTerminate();

    return EXIT_SUCCESS;
}
