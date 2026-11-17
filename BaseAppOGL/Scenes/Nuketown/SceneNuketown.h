#pragma once

#include "Engine/CSceneBase.h"
#include "Engine/CCamera.h"
#include "Engine/CPrimitives.h"
#include "Engine/CShader.h"
#include "Engine/CTextures.h"
#include "NuketownTraversal.h"

#include <memory>

class CAsset;

class SceneNuketown final : public CSceneBase
{
public:
    SceneNuketown();
    ~SceneNuketown() override;

    void DrawScene(float deltaTime) override;
    void ProcessSceneInput(GLFWwindow* window, float deltaTime) override;

private:
    // These values are shared with the surface branches in Nuketown.frag.
    enum class Surface { Solid, Asphalt, Concrete, Grass, Dirt, Siding, Wood, Roof, Glass, Paint };

    struct Part
    {
        CPrimitives* mesh;
        glm::mat4 model;
        glm::vec3 color;
        glm::vec3 textureSize;
        Surface surface;
    };

    void AddPart(CPrimitives& mesh, glm::vec3 center, glm::vec3 size, glm::vec3 color,
        const glm::mat4& parent = glm::mat4(1.0f), Surface surface = Surface::Solid);
    void AddBox(glm::vec3 center, glm::vec3 size, glm::vec3 color,
        const glm::mat4& parent = glm::mat4(1.0f), Surface surface = Surface::Solid);
    void AddBeam(glm::vec3 start, glm::vec3 end, float width, glm::vec3 color,
        Surface surface = Surface::Solid);
    void AddWheels(const glm::mat4& vehicle, float halfAxle, float radius, float frontZ, float rearZ);
    void AddPath(glm::vec2 start, glm::vec2 end, float width, float top, glm::vec3 color,
        Surface surface = Surface::Concrete);
    void AddFence(glm::vec2 start, glm::vec2 end, float height, glm::vec3 color);
    void BuildStreet();
    void BuildSpawnYard();
    void BuildPerimeter();
    void BuildBus();
    void BuildTruck();
    void BuildHouses();
    void BuildClockTower();
    glm::vec3 GetViewPosition() const;
    void DrawAvatar();
    void DrawMenu();

    CCamera camera;
    bool thirdPerson = false;
    bool menuOpen = false;
    float cameraDistance = 3.4f;
    NuketownTraversal traversal;
    glm::vec3 garageLampPosition;
    CShader shader;
    CTextures surfaceTextures;
    CPrimitives box;
    CPrimitives cylinder;
    CPrimitives sphere;
    CPrimitives pyramid;
    std::vector<Part> parts;
    std::unique_ptr<CAsset> busAsset;
    glm::mat4 busModel{1.0f};
};
