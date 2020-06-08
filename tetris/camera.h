#ifndef _CAMERA_H_
#define _CAMERA_H_

//#include <stdbool.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>

// Defines several possible options for camera movement. Used as abstraction to stay away from window-system specific input methods
typedef enum {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
} Camera_Movement;

// Default camera values
const float YAW         = -90.0f;
const float PITCH       =  0.0f;
const float SPEED       =  1.0f;
const float SENSITIVITY =  0.1f;
const float ZOOM        =  45.0f;

typedef struct {
    vec3 cameraPos;
    vec3 cameraFront;
    vec3 cameraUp;

    float yaw, pitch, fov;
} Camera;

void updateCameraVectors (Camera *camera)
{
    vec3 direction = {
        cos(glm_rad(camera->yaw)) * cos(glm_rad(camera->pitch)),
        sin(glm_rad(camera->pitch)),
        sin(glm_rad(camera->yaw)) * cos(glm_rad(camera->pitch)),
    };

    glm_vec3_normalize_to(direction, camera->cameraFront);
}

void initCamera (Camera *camera)
{
    Camera cameraData = {
        .cameraPos = {0.0f, 0.0f, 32.0f},
        .cameraFront = {0.0f, 0.0f, -1.0f},
        .cameraUp = {0.0f, 1.0f, 0.0f},

        .yaw = -90.0f, // yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction vector pointing to the right so we initially rotate a bit to the left.
        .pitch = 0.0f,
        .fov = 45.0f,
    };

    memcpy(camera, &cameraData, sizeof(cameraData));

    updateCameraVectors(camera);
}

void processKeyboard (Camera *camera, Camera_Movement direction, float deltaTime)
{
    float cameraSpeed = 15.0f * deltaTime;
    vec3 tmp, tmp2;

    if (direction == FORWARD) {
        glm_vec3_scale(camera->cameraFront, cameraSpeed, tmp);
        glm_vec3_add(camera->cameraPos, tmp, camera->cameraPos);
    }
    if (direction == BACKWARD) {
        glm_vec3_scale(camera->cameraFront, cameraSpeed, tmp);
        glm_vec3_sub(camera->cameraPos, tmp, camera->cameraPos);
    }
    if (direction == LEFT) {
        glm_vec3_cross(camera->cameraFront, camera->cameraUp, tmp);
        glm_vec3_normalize(tmp);
        glm_vec3_scale(tmp, cameraSpeed, tmp2);
        glm_vec3_sub(camera->cameraPos, tmp2, camera->cameraPos);
    }
    if (direction == RIGHT) {
        glm_vec3_cross(camera->cameraFront, camera->cameraUp, tmp);
        glm_vec3_normalize(tmp);
        glm_vec3_scale(tmp, cameraSpeed, tmp2);
        glm_vec3_add(camera->cameraPos, tmp2, camera->cameraPos);
    }
}

void processMouseMovement (Camera *camera, float xoffset, float yoffset)
{
    const float sensitivity = 0.05f;

    xoffset *= sensitivity;
    yoffset *= sensitivity;

    camera->yaw += xoffset;
    camera->pitch += yoffset;

    if (camera->pitch > 89.0f) {
        camera->pitch = 89.0f;
    }
    if (camera->pitch < -89.0f) {
        camera->pitch = -89.0f;
    }

    updateCameraVectors(camera);
}

void processMouseScroll (Camera *camera, double yoffset)
{
    if (camera->fov >= 1.0f && camera->fov <= 45.0f) {
        camera->fov -= yoffset;
    }
    if (camera->fov < 1.0f) {
        camera->fov = 1.0f;
    }
    if (camera->fov > 45.0f) {
        camera->fov = 45.0f;
    }
}

void getViewMatrix (Camera *camera, vec4 *view)
{
    vec3 center;

    glm_vec3_add(camera->cameraPos, camera->cameraFront, center);
    glm_lookat(camera->cameraPos, center, camera->cameraUp, view);
}

#endif // _CAMERA_H_
