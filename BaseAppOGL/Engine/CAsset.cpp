#pragma warning(disable : 4267)
#define STB_IMAGE_IMPLEMENTATION 

#include "CAsset.h"
#include <iostream>
#include <limits>

// Mesh Implementation
Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures,
    glm::vec3 diffuseColor)
    : vertices(vertices), indices(indices), textures(textures), diffuseColor(diffuseColor)
{
    setupMesh();
}

void Mesh::setupMesh() 
{
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    // Vertex positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

    // Normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));

    // Texture coords
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));

	// Tangents
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));

	// Bitangents
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Bitangent));
    

    glBindVertexArray(0);
}

void Mesh::Draw(unsigned int shaderProgram) 
{
    bool hasDiffuseMap = false;
    for (unsigned int i = 0; i < textures.size(); ++i)
    {
        const Texture& texture = textures[i];
        glActiveTexture(GL_TEXTURE0 + i);
        glUniform1i(glGetUniformLocation(shaderProgram, texture.type.c_str()), i);
        glBindTexture(GL_TEXTURE_2D, texture.id);
        hasDiffuseMap |= texture.type == "uDiffuseMap";
    }
    glUniform1i(glGetUniformLocation(shaderProgram, "uHasDiffuseMap"), hasDiffuseMap);
    glUniform3fv(glGetUniformLocation(shaderProgram, "uMaterialColor"), 1, &diffuseColor[0]);

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    
}

void Mesh::DrawWithShadows(unsigned int shaderProgram)
{
    glActiveTexture(GL_TEXTURE1);
    glUniform1i(glGetUniformLocation(shaderProgram, "uDiffuseTexture"), 1);
    glBindTexture(GL_TEXTURE_2D, textures.empty() ? 0 : textures.front().id);

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void Mesh::DeleteBuffers()
{
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    DeleteFaceNormals();
}

void Mesh::BuildFaceNormals(float length)
{
    DeleteFaceNormals();

    std::vector<glm::vec3> lines;
    lines.reserve((indices.size() / 3) * 2);

    for (size_t i = 0; i + 2 < indices.size(); i += 3)
    {
        const glm::vec3& v0 = vertices[indices[i]].Position;
        const glm::vec3& v1 = vertices[indices[i + 1]].Position;
        const glm::vec3& v2 = vertices[indices[i + 2]].Position;

        glm::vec3 n = glm::cross(v1 - v0, v2 - v0);
        float len = glm::length(n);
        if (len < 1e-8f)
            continue; // triangulo degenerado
        n /= len;

        glm::vec3 center = (v0 + v1 + v2) / 3.0f;
        lines.push_back(center);
        lines.push_back(center + n * length);
    }

    normalsVertexCount = static_cast<unsigned int>(lines.size());
    normalsLength = length;
    if (lines.empty())
        return;

    glGenVertexArrays(1, &normalsVAO);
    glGenBuffers(1, &normalsVBO);

    glBindVertexArray(normalsVAO);
    glBindBuffer(GL_ARRAY_BUFFER, normalsVBO);
    glBufferData(GL_ARRAY_BUFFER, lines.size() * sizeof(glm::vec3), lines.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);

    glBindVertexArray(0);
}

void Mesh::DrawFaceNormals(unsigned int shaderProgram, const glm::vec3& color)
{
    if (normalsVAO == 0 || normalsVertexCount == 0)
        return;

    GLint colorLoc = glGetUniformLocation(shaderProgram, "uColor");
    if (colorLoc != -1)
        glUniform3fv(colorLoc, 1, &color[0]);

    glBindVertexArray(normalsVAO);
    glDrawArrays(GL_LINES, 0, normalsVertexCount);
    glBindVertexArray(0);
}

void Mesh::DeleteFaceNormals()
{
    if (normalsVBO) glDeleteBuffers(1, &normalsVBO);
    if (normalsVAO) glDeleteVertexArrays(1, &normalsVAO);
    normalsVBO = 0;
    normalsVAO = 0;
    normalsVertexCount = 0;
}

// CAsset Implementation
CAsset::CAsset(const std::string& path, bool _flipTexture) 
    : flipTexture(_flipTexture)
{
    loadModel(path);
}

CAsset::~CAsset()
{
    // Cleanup textures
    for (auto& mesh : meshes) 
    {
        for (auto& texture : mesh.textures) 
        {
            glDeleteTextures(1, &texture.id);
        }
        mesh.DeleteBuffers();
	}
}

bool CAsset::TryGetBounds(glm::vec3& minimum, glm::vec3& maximum) const
{
    if (meshes.empty())
        return false;

    minimum = glm::vec3(std::numeric_limits<float>::max());
    maximum = -minimum;
    for (const Mesh& mesh : meshes)
    {
        for (const Vertex& vertex : mesh.vertices)
        {
            minimum = glm::min(minimum, vertex.Position);
            maximum = glm::max(maximum, vertex.Position);
        }
    }
    return true;
}

void CAsset::DrawFaceNormals(unsigned int shaderProgram, float length, const glm::vec3& color)
{
    for (auto& mesh : meshes)
    {
        // Constroi (ou reconstroi se o tamanho mudou) as linhas das normais
        if (mesh.normalsBuilt() == false || mesh.normalsLengthValue() != length)
            mesh.BuildFaceNormals(length);
        mesh.DrawFaceNormals(shaderProgram, color);
    }
}

void CAsset::Draw(unsigned int shaderProgram, bool shadows)
{
	if (!shadows)
	{
        for (auto& mesh : meshes)
            mesh.Draw(shaderProgram);
	}
    else
    {
        for (auto& mesh : meshes)
            mesh.DrawWithShadows(shaderProgram);
    }
    
}

void CAsset::loadModel(const std::string& path) 
{
    Assimp::Importer importer;
    importer.SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS, 
        aiComponent_CAMERAS | 
        aiComponent_LIGHTS | 
        aiComponent_COLORS | 
        aiComponent_ANIMATIONS | 
        aiComponent_BONEWEIGHTS
    );

    const aiScene* scene = importer.ReadFile(path,
        aiProcess_Triangulate | 
        aiProcess_FlipUVs | 
        aiProcess_GenNormals |
        aiProcess_CalcTangentSpace | 
        aiProcess_PreTransformVertices |
        aiProcess_RemoveComponent
    );

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) 
    {
        std::cerr << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
        return;
    }
    directory = path.substr(0, path.find_last_of('/'));
    processNode(scene->mRootNode, scene);
}

void CAsset::processNode(aiNode* node, const aiScene* scene) 
{
    for (unsigned int i = 0; i < node->mNumMeshes; i++) 
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        if (mesh->HasPositions() && mesh->HasFaces())
            meshes.push_back(processMesh(mesh, scene));
    }
    for (unsigned int i = 0; i < node->mNumChildren; i++) 
    {
        processNode(node->mChildren[i], scene);
    }
}

Mesh CAsset::processMesh(aiMesh* mesh, const aiScene* scene) 
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;

  

    for (unsigned int i = 0; i < mesh->mNumVertices; i++) 
    {
        Vertex vertex{};
        vertex.Position = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
        if (mesh->HasNormals())
            vertex.Normal = glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);

        if (mesh->mTextureCoords[0]) 
        {
            vertex.TexCoords = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
        }
        else 
        {
            vertex.TexCoords = glm::vec2(0.0f, 0.0f);
        }

        if (mesh->HasTangentsAndBitangents())
        {
            vertex.Tangent = glm::vec3(mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z);
            vertex.Bitangent = glm::vec3(mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z);
        }

        vertices.push_back(vertex);
    }

    for (unsigned int i = 0; i < mesh->mNumFaces; i++) 
    {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }

    glm::vec3 diffuseColor(1.0f);
    if (mesh->mMaterialIndex < scene->mNumMaterials)
    {
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
        aiColor3D diffuse;
        if (material->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse) == AI_SUCCESS)
            diffuseColor = {diffuse.r, diffuse.g, diffuse.b};
        auto diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "uDiffuseMap");
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
        auto normalMaps = loadMaterialTextures(material, aiTextureType_NORMALS, "uNormalMap");
        textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
        auto heightMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "uHeightMap");
        textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());
    }

    numVertices = mesh->mNumVertices;
	numIndices = indices.size();
    numFaces = mesh->mNumFaces;

    return Mesh(vertices, indices, textures, diffuseColor);
}

std::vector<Texture> CAsset::loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName) 
{
    std::vector<Texture> textures;
    for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) 
    {
        aiString str;
        mat->GetTexture(type, i, &str);

        Texture texture;
        texture.id = TextureFromFile(str.C_Str(), directory);
        texture.type = typeName;
        texture.path = str.C_Str();
        if (texture.id != 0)
            textures.push_back(texture);
    }
    return textures;
}


unsigned int CAsset::TextureFromFile(const char* path, const std::string& directory) 
{
	std::string filename = std::string(path);
    //filename = filename.substr(filename.find_last_of('\\') + 1); // Extract the filename from the path
    filename = directory + '/' + filename;

    // Flip the image vertically during loading
    stbi_set_flip_vertically_on_load(flipTexture);

    unsigned int textureID = 0;
    int width, height, nrComponents;
    unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
    const std::string mapPath(path);
    const size_t separator = mapPath.find_last_of("/\\");
    if (!data && separator != std::string::npos)
    {
        // Some exporters store the original absolute bitmap path in the material.
        filename = directory + '/' + mapPath.substr(separator + 1);
        data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
    }
    if (data) 
    {
        glGenTextures(1, &textureID);
        GLenum format = GL_RG;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        GLint previousAlignment = 0;
        glGetIntegerv(GL_UNPACK_ALIGNMENT, &previousAlignment);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glPixelStorei(GL_UNPACK_ALIGNMENT, previousAlignment);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // Check if anisotropic filtering is supported and get the maximum anisotropy level
        GLfloat maxAnisotropy = 0.0f;
        if (glewIsSupported("GL_EXT_texture_filter_anisotropic"))
        {
            // Get the maximum anisotropy level supported by the GPU
            glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAnisotropy);

            // Set anisotropic filtering
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAnisotropy);
        }

        stbi_image_free(data);
    }
    else 
    {
        std::cerr << "Texture failed to load at path: " << filename << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

