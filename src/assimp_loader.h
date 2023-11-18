#include "mesh_loader.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>

#pragma once

namespace std
{
    template <>
    struct hash<vt::VtModel::Vertex>
    {
        size_t operator()(vt::VtModel::Vertex const &vertex) const
        {
            size_t seed{0};
            vt::hashCombine(seed, vertex.position, vertex.color, vertex.normal, vertex.uv);
            return seed;
        }
    };
}

namespace vt {

class AssimpModelLoader : public ModelLoader {
public:

    bool loadModel(const std::string& path, std::vector<VtModel::Vertex>& vertices, std::vector<uint32_t>& indices) override {
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(path,
                                                  aiProcess_Triangulate | aiProcess_FlipUVs);

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            std::cerr << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
            return false;
        }

        vertices.clear();
        indices.clear();

        std::unordered_map<VtModel::Vertex, uint32_t> uniqueVertices;

        for (unsigned int i = 0; i < scene->mNumMeshes; i++) {
            aiMesh* mesh = scene->mMeshes[i];
            for (unsigned int j = 0; j < mesh->mNumVertices; j++) {
                VtModel::Vertex vertex{};
                vertex.position = {mesh->mVertices[j].x, mesh->mVertices[j].y, mesh->mVertices[j].z};

                if (mesh->mColors[0]) {
                    vertex.color = {mesh->mColors[0][j].r, mesh->mColors[0][j].g, mesh->mColors[0][j].b};
                } else {
                    vertex.color = {1.0f, 1.0f, 1.0f};
                }

                if (mesh->mNormals) {
                    vertex.normal = {mesh->mNormals[j].x, mesh->mNormals[j].y, mesh->mNormals[j].z};
                }

                if (mesh->mTextureCoords[0]) {
                    vertex.uv = {mesh->mTextureCoords[0][j].x, mesh->mTextureCoords[0][j].y};
                } else {
                    vertex.uv = {0.0f, 0.0f};
                }

                if (uniqueVertices.count(vertex) == 0) {
                    uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                    vertices.push_back(vertex);
                }
                indices.push_back(uniqueVertices[vertex]);
            }
        }
        return true;
    }

};

}