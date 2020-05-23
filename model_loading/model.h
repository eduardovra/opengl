#ifndef _MODEL_H_
#define _MODEL_H_

#include <libgen.h>

#include <cglm/cglm.h>
#include <assimp/cimport.h>        // Plain-C interface
#include <assimp/scene.h>          // Output data structure
#include <assimp/postprocess.h>    // Post processing flags
//#include <assimp/mesh.h>

#include "mesh.h"

typedef struct {
    Mesh *meshes;
    unsigned int numMeshes;
    char *directory;
} Model;

void drawModel(Model *model, unsigned int shader)
{
    for (unsigned int i = 0; i < model->numMeshes; i++) {
        drawMesh(&model->meshes[i], shader);
    }
}

Mesh processMesh(struct aiMesh *mesh, const struct aiScene *scene)
{
    Vertex *vertices = malloc(mesh->mNumVertices * sizeof(Vertex));
    unsigned int *indices = NULL;
    Texture *textures = NULL;

    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        // process vertex positions, normals and texture coordinates
        Vertex vertex = {
            .position = {mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z},
            .normal = {mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z},
            .texCoords = {0.0f, 0.0f},
        };

        if (mesh->mTextureCoords[0]) { // does the mesh contain texture coordinates?
            vec2 texCoords = {mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y};
            memcpy(vertex.texCoords, texCoords, sizeof(texCoords));
        };

        memcpy(&vertices[i], &vertex, sizeof(vertex));
    }
    /*
    // process indices
    ...
    // process material
    if(mesh->mMaterialIndex >= 0) {
        ...
    }
    */

    return createMesh(vertices, mesh->mNumVertices, indices, 0, textures, 0);
}

void processNode(Model *model, struct aiNode *node, const struct aiScene *scene);

void processNode(Model *model, struct aiNode *node, const struct aiScene *scene)
{
    unsigned int meshesOffset = model->numMeshes;
    // alloc memory for meshes
    model->numMeshes += node->mNumMeshes;
    if (model->meshes == NULL) {
        model->meshes = malloc(model->numMeshes * sizeof(Mesh));
    }
    else {
        model->meshes = realloc(model->meshes, model->numMeshes * sizeof(Mesh));
    }

    // process all the node's meshes (if any)
    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        struct aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        model->meshes[meshesOffset + i] = processMesh(mesh, scene);
    }
    // then do the same for each of its children
    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        processNode(model, node->mChildren[i], scene);
    }
}

void loadModel(Model *model, const char *path)
{
    const struct aiScene *scene = aiImportFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        printf("ERROR::ASSIMP::%s\n", aiGetErrorString());
        exit(EXIT_FAILURE);
    }
    char *directory = dirname((char *) path);
    model->directory = strdup(directory);

    processNode(model, scene->mRootNode, scene);
}

Model createModel(const char *path)
{
    Model model = {
        .meshes = NULL,
        .numMeshes = 0,
    };

    loadModel(&model, path);

    return model;
}

#endif // _MODEL_H_
