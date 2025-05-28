#include "Model.h"
#include <stb_image.h>
#include <iostream>

Model::Model(const std::string& path) {
    loadModel(path);
}

void Model::Draw(Shader& shader) {
    for (auto& mesh : meshes)
        mesh.Draw(shader);
}

void Model::loadModel(const std::string& path) {
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path,
        aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenSmoothNormals);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cerr << "ASSIMP ERROR: " << importer.GetErrorString() << "\n";
        return;
    }

    directory = path.substr(0, path.find_last_of('/'));
    processNode(scene->mRootNode, scene);
}

void Model::processNode(aiNode* node, const aiScene* scene) {
    for (unsigned int i = 0; i < node->mNumMeshes; ++i) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(processMesh(mesh, scene));
    }
    for (unsigned int i = 0; i < node->mNumChildren; ++i)
        processNode(node->mChildren[i], scene);
}

Mesh Model::processMesh(aiMesh* mesh, const aiScene* scene) {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;

    for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
        Vertex vertex;
        vertex.position = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
        vertex.normal = glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
        vertex.texCoords = mesh->HasTextureCoords(0) ?
            glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y) : glm::vec2(0.0f);
        vertices.push_back(vertex);
    }

    for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; ++j)
            indices.push_back(face.mIndices[j]);
    }

    aiMaterial* mat = scene->mMaterials[mesh->mMaterialIndex];
    auto diffuseMaps = loadMaterialTextures(mat, aiTextureType_DIFFUSE, "texture_diffuse");
    textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
    auto specularMaps = loadMaterialTextures(mat, aiTextureType_SPECULAR, "texture_specular");
    textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());

    Material simpleMat;
    aiColor3D color(0.f);
    if (AI_SUCCESS == mat->Get(AI_MATKEY_COLOR_AMBIENT, color))
        simpleMat.ambient = glm::vec3(color.r, color.g, color.b);
    if (AI_SUCCESS == mat->Get(AI_MATKEY_COLOR_DIFFUSE, color))
        simpleMat.diffuse = glm::vec3(color.r, color.g, color.b);
    if (AI_SUCCESS == mat->Get(AI_MATKEY_COLOR_SPECULAR, color))
        simpleMat.specular = glm::vec3(color.r, color.g, color.b);

    float shininess = 0.0f;
    if (AI_SUCCESS == mat->Get(AI_MATKEY_SHININESS, shininess))
        simpleMat.shininess = shininess;
    else
        simpleMat.shininess = 32.0f;

    return Mesh(vertices, indices, textures, simpleMat);
}

std::vector<Texture> Model::loadMaterialTextures(aiMaterial* mat, aiTextureType type, const std::string& typeName) {
    std::vector<Texture> textures;
    for (unsigned int i = 0; i < mat->GetTextureCount(type); ++i) {
        aiString str;
        mat->GetTexture(type, i, &str);

        std::string filename = directory + '/' + std::string(str.C_Str());
        bool skip = false;
        for (auto& tex : textures_loaded) {
            if (tex.path == filename) {
                textures.push_back(tex);
                skip = true;
                break;
            }
        }
        if (!skip) {
            Texture texture;
            texture.id = TextureFromFile(str.C_Str(), directory);
            texture.type = typeName;
            texture.path = filename;
            textures.push_back(texture);
            textures_loaded.push_back(texture);
        }
    }
    return textures;
}

unsigned int TextureFromFile(const char* path, const std::string& directory, bool gamma) {
    std::string filename = directory + '/' + std::string(path);

    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
    if (data) {
        GLenum format = (nrComponents == 1) ? GL_RED :
            (nrComponents == 3) ? GL_RGB :
            GL_RGBA;
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    else {
        std::cout << "Texture failed to load at path: " << path << std::endl;
    }
    stbi_image_free(data);
    return textureID;
}