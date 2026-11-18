#pragma once

#include "Engine/CSceneBase.h"
#include "Engine/CCamera.h"
#include "Engine/CPrimitives.h"
#include "Engine/CShader.h"
#include "Engine/CTextures.h"
#include "NuketownTraversal.h"

#include <memory>
#include <unordered_map>

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
    enum class Surface { Solid, Asphalt, Concrete, Grass, Dirt, Siding, Wood, Roof, Glass, Paint,
        Sign, PoliceMark, Delivery, Stone, VerticalSiding, Shadow, VehiclePaint, Chrome, VehicleGlass, Tire,
        Emissive, Sand, Mountains, RefractiveGlass };

    struct Part
    {
        CPrimitives* mesh;
        glm::mat4 model;
        glm::vec3 color;
        glm::vec3 textureSize;
        Surface surface;
        glm::mat3 normalMatrix;
        glm::vec3 boundsCenter;
        float boundsRadius;
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
    void BuildEnvironment();
    void BuildStreetLighting();
    void BakeStreetLightMap();
    void BuildSpawnYard();
    void BuildPerimeter();
    void BuildBus();
    void BuildTruck();
    void BuildHouses();
    void BuildHouse(glm::vec2 center, float yaw, bool yellow);
    void BuildPorch(const glm::mat4& house, bool yellow);
    void BuildEastYard();
    void BuildWestGarden();
    void BuildTestSiteFixtures();
    void BuildLandmarks();
    void BuildParkedCar(glm::vec2 center, float yaw, glm::vec3 color, bool police = false);
    void BuildClockTower();
    void ToggleFlyCamera();
    void ResetFlyCamera();
    void MoveFlyCamera(glm::vec3 direction, float deltaTime, bool accelerated);
    void DrawMenu();
    void InitializeRenderingResources();
    static glm::vec3 SunDirection();
    void RenderShadowMap();
    void BakeSkyEnvironment();
    void DrawSky(const glm::mat4& viewProjection, glm::vec3 viewPosition);
    void CaptureOpaqueScene(int width, int height);
    void LoadMaterialMaps();
    void BindMaterialMaps();
    void VisitGarage(bool inside = false);

    CCamera camera;
    bool menuOpen = false;
    NuketownTraversal traversal;
    glm::vec3 garageLampPosition;
    glm::vec3 secondGarageLampPosition;
    glm::mat4 garageWorldToLocal[2]{glm::mat4(1.0f), glm::mat4(1.0f)};
    CShader shader;
    CTextures surfaceTextures;
    CPrimitives box;
    CPrimitives cylinder;
    CPrimitives sphere;
    CPrimitives pyramid;
    CPrimitives roundedBox;
    CPrimitives carBody;
    CPrimitives carCabin;
    CPrimitives truckHood;
    CPrimitives tire;
    CPrimitives desertBasin;
    CPrimitives glassPane;
    struct MeshBounds { glm::vec3 center; glm::vec3 half; };
    std::unordered_map<const CPrimitives*, MeshBounds> meshBounds;
    unsigned int shadowFramebuffer = 0;
    unsigned int shadowTexture = 0;
    unsigned int skyVao = 0;
    bool shadowReady = false;
    glm::mat4 lightViewProjection{1.0f};
    std::vector<Part> parts;
    std::unique_ptr<CAsset> busAsset;
    glm::mat4 busModel{1.0f};
    unsigned int environmentTexture = 0;
    std::vector<glm::vec3> streetLightPositions;
    unsigned int streetLightTexture = 0;
    unsigned int refractionFramebuffer = 0;
    unsigned int refractionColorTexture = 0;
    unsigned int refractionDepthTexture = 0;
    int refractionWidth = 0;
    int refractionHeight = 0;
    bool refractionEnabled = true;
    bool normalMappingEnabled = true;
    bool bakedShadowsEnabled = true;
    bool flyCameraEnabled = false;
    CCamera savedWalkingCamera;
    CCamera inspectionCamera;
};
