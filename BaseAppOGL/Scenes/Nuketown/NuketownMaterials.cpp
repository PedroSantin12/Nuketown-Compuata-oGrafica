#include "SceneNuketown.h"

#include <cmath>
#include <stdexcept>

void SceneNuketown::LoadMaterialMaps()
{
    // RGB normals are linear data. The class loader does not apply sRGB decoding.
    surfaceTextures.LoadTextureAnisotropic(8,
        "Scenes/Nuketown/Assets/Materials/CONCRETE_NORMAL.BMP", true);
    surfaceTextures.LoadTextureAnisotropic(9,
        "Scenes/Nuketown/Assets/Materials/ASPHALT_NORMAL.BMP", true);
    surfaceTextures.LoadTextureClamp(10,
        "Scenes/Nuketown/Assets/Bake/GarageShadow.bmp", true);

    // A missing map must not silently turn the garage floor black.
    for (unsigned int index = 8; index <= 10; ++index)
    {
        surfaceTextures.ApplyTexture(index);
        int width = 0;
        glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
        if (width == 0)
            throw std::runtime_error("Nuketown: mapa de normal ou bake ausente em Scenes/Nuketown/Assets.");
    }
    glBindTexture(GL_TEXTURE_2D, 0);
}

void SceneNuketown::BindMaterialMaps()
{
    shader.SetInt("uConcreteNormalMap", 12);
    shader.SetInt("uAsphaltNormalMap", 13);
    shader.SetInt("uGarageShadowBake", 14);
    shader.SetBool("uEnableNormalMapping", normalMappingEnabled);
    shader.SetBool("uEnableBakedShadows", bakedShadowsEnabled);
    for (unsigned int index = 8; index <= 10; ++index)
    {
        glActiveTexture(GL_TEXTURE0 + index + 4);
        surfaceTextures.ApplyTexture(index);
    }
    shader.SetMat4("uGarageWorldToLocal[0]", garageWorldToLocal[0]);
    shader.SetMat4("uGarageWorldToLocal[1]", garageWorldToLocal[1]);
    glActiveTexture(GL_TEXTURE0);
}

void SceneNuketown::VisitGarage(bool inside)
{
    if (flyCameraEnabled)
        ToggleFlyCamera();
    const glm::mat4 garage = glm::inverse(garageWorldToLocal[0]);
    const glm::vec3 eye(garage * glm::vec4(inside ? glm::vec3(-0.15f, 1.73f, 1.70f)
        : glm::vec3(-4.15f, 1.73f, 1.10f), 1.0f));
    const glm::vec3 target(garage * glm::vec4(inside ? glm::vec3(-0.90f, 0.10f, -1.20f)
        : glm::vec3(-0.6f, 1.35f, -0.5f), 1.0f));
    const glm::vec3 direction = glm::normalize(target - eye);
    camera = CCamera(eye, {0.0f, 1.0f, 0.0f},
        glm::degrees(std::atan2(direction.z, direction.x)), glm::degrees(std::asin(direction.y)));
    camera.Zoom = inside ? 62.0f : 56.0f;
}
