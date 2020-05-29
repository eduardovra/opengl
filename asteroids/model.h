#ifndef _MODEL_H_
#define _MODEL_H_

#include <stdio.h>
#include <stdlib.h>
#include <libgen.h>
#include <limits.h>

#include <cglm/cglm.h>
#include <assimp/cimport.h>        // Plain-C interface
#include <assimp/scene.h>          // Output data structure
#include <assimp/postprocess.h>    // Post processing flags

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "mesh.h"

typedef struct {
    Mesh *meshes;
    unsigned int numMeshes;
    char *directory;

    Texture *loadedTextures;
    unsigned int numLoadedTextures;
} Model;

void drawModel(Model *model, unsigned int shader)
{
    for (unsigned int i = 0; i < model->numMeshes; i++) {
        drawMesh(&model->meshes[i], shader);
    }
}

unsigned int TextureFromFile(char *imagePath, char *directory)
{
    char filename[PATH_MAX];
    snprintf(filename, sizeof(filename), "%s/%s", directory, imagePath);

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
    unsigned char *data = stbi_load(filename, &width, &height, &nrChannels, 0);
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

void loadMaterialTextures(Model *model, struct aiMaterial *mat, enum aiTextureType type,
    const char *typeName, Texture **textures, unsigned int *numTextures)
{
    *numTextures = aiGetMaterialTextureCount(mat, type);
    *textures = malloc(*numTextures * sizeof(Texture));

    for (unsigned int i = 0; i < *numTextures; i++) {
        struct aiString path;
        aiReturn ret = aiGetMaterialTexture(mat, type, i, &path, NULL, NULL, NULL, NULL, NULL, NULL);
        if (ret != aiReturn_SUCCESS) {
            printf("Error loading material textures\n");
            exit(EXIT_FAILURE);
        }

        Texture texture;
        // check if texture is already loaded
        bool loaded = false;
        for (unsigned int t = 0; t < model->numLoadedTextures; t++) {
            Texture *loadedTex = &model->loadedTextures[t];
            if (strcmp(loadedTex->path, path.data) == 0) {
                texture = *loadedTex;
                loaded = true;
                break;
            }
        }

        if (!loaded) {
            texture.id = TextureFromFile(path.data, model->directory);
            strcpy(texture.type, typeName);
            strcpy(texture.path, path.data);
            model->loadedTextures = realloc(model->loadedTextures, ++model->numLoadedTextures * sizeof(Texture));
            model->loadedTextures[model->numLoadedTextures - 1] = texture;
        }

        *textures[i] = texture;
    }
}

Mesh processMesh(Model *model, struct aiMesh *mesh, const struct aiScene *scene)
{
    // process vertices
    Vertex *vertices = malloc(mesh->mNumVertices * sizeof(Vertex));
    unsigned int numVertices = mesh->mNumVertices;

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

    // process indices
    // allocate memory
    unsigned int numIndices = 0;

    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        struct aiFace face = mesh->mFaces[i];
        numIndices += face.mNumIndices;
    }
    unsigned int *indices = malloc(numIndices * sizeof(unsigned int));
    unsigned int idx = 0;
    // copy indices
    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        struct aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++, idx++) {
            memcpy(&indices[idx], &face.mIndices[j], sizeof(unsigned int));
        }
    }

    // process material
    Texture *textures = NULL;
    unsigned int numTextures = 0;
    if (mesh->mMaterialIndex >= 0) {
        struct aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
        // load diffuse maps
        Texture *diffuseMaps;
        unsigned int numDiffuseMaps;
        loadMaterialTextures(model, material, aiTextureType_DIFFUSE,
            "texture_diffuse", &diffuseMaps, &numDiffuseMaps);

        // load specular maps
        Texture *specularMaps;
        unsigned int numSpecularMaps;
        loadMaterialTextures(model, material, aiTextureType_SPECULAR,
            "texture_specular", &specularMaps, &numSpecularMaps);

        // add to the mesh
        numTextures = numDiffuseMaps + numSpecularMaps;
        textures = malloc(numTextures * sizeof(Texture));
        memcpy(textures, diffuseMaps, numDiffuseMaps * sizeof(Texture));
        memcpy(&textures[numDiffuseMaps], specularMaps, numSpecularMaps * sizeof(Texture));
    }

    return createMesh(vertices, numVertices, indices, numIndices, textures, numTextures);
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
        model->meshes[meshesOffset + i] = processMesh(model, mesh, scene);
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

    char *canonicalPath = realpath(path, NULL);
    model->directory = dirname(canonicalPath); // base path for textures

    processNode(model, scene->mRootNode, scene);
}

Model createModel(const char *path)
{
    Model model = {
        .meshes = NULL,
        .numMeshes = 0,
        .loadedTextures = NULL,
        .numLoadedTextures = 0,
    };

    loadModel(&model, path);

    return model;
}

#endif // _MODEL_H_
