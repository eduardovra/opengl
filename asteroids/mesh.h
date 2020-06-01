#ifndef _MESH_H_
#define _MESH_H_

#include <limits.h>

#include <glad/glad.h>
#include <cglm/cglm.h>

typedef struct {
    vec3 position;
    vec3 normal;
    vec2 texCoords;
} Vertex;

typedef struct {
    unsigned int id;
    char type[50];
    char path[PATH_MAX];  // we store the path of the texture to compare with other textures
} Texture;

typedef struct {
    Vertex *vertices;
    unsigned int *indices;
    Texture *textures;

    unsigned int numVertices, numIndices, numTextures;

    unsigned int VAO, VBO, EBO;
} Mesh;

void setupMesh(Mesh *mesh)
{
    glGenVertexArrays(1, &mesh->VAO);
    glGenBuffers(1, &mesh->VBO);
    glGenBuffers(1, &mesh->EBO);

    glBindVertexArray(mesh->VAO);

    glBindBuffer(GL_ARRAY_BUFFER, mesh->VBO);
    glBufferData(GL_ARRAY_BUFFER, mesh->numVertices * sizeof(Vertex),
                 mesh->vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->numIndices * sizeof(unsigned int), 
                 mesh->indices, GL_STATIC_DRAW);

    // vertex positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *) 0);
    // vertex normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *) offsetof(Vertex, normal));
    // vertex texture coords
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *) offsetof(Vertex, texCoords));

    glBindVertexArray(0);
}

Mesh createMesh(Vertex *vertices, unsigned int numVertices, unsigned int *indices,
    unsigned int numIndices, Texture *textures, unsigned int numTextures)
{
    Mesh mesh = {
        vertices = vertices,
        indices = indices,
        textures = textures,

        numVertices = numVertices,
        numIndices = numIndices,
        numTextures = numTextures,
    };

    setupMesh(&mesh);

    return mesh;
}

void drawMesh(Mesh *mesh, unsigned int shader)
{
    unsigned int diffuseNr = 1;
    unsigned int specularNr = 1;

    for (unsigned int i = 0; i < mesh->numTextures; i++) {
        glActiveTexture(GL_TEXTURE0 + i); // activate proper texture unit before binding
        // retrieve texture number (the N in diffuse_textureN)
        char number[50], uniform[100];
        char *name = mesh->textures[i].type;
        if (strcmp(name, "texture_diffuse") == 0) {
            sprintf(number, "%d", diffuseNr++);
        }
        else if (strcmp(name, "texture_specular") == 0) {
            sprintf(number, "%d", specularNr++);
        }
        else {
            printf("Texture type not supported\n");
            exit(EXIT_FAILURE);
        }

        sprintf(uniform, "material.%s%s", name, number);
        glUniform1f(glGetUniformLocation(shader, uniform), i);
        glBindTexture(GL_TEXTURE_2D, mesh->textures[i].id);
    }
    glActiveTexture(GL_TEXTURE0);

    // draw mesh
    glBindVertexArray(mesh->VAO);
    glDrawElements(GL_TRIANGLES, mesh->numIndices, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

#endif // _MESH_H_
