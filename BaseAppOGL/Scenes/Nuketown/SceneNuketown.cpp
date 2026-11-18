#include "SceneNuketown.h"
#include "Engine/CAsset.h"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <iterator>

namespace
{
    const glm::vec3 hazeColor(0.74f, 0.79f, 0.80f);
    const glm::vec3 asphalt(0.27f, 0.29f, 0.29f);
    const glm::vec3 concrete(0.60f, 0.58f, 0.52f);
    const glm::vec3 warmWhite(0.87f, 0.86f, 0.75f);
    const glm::vec3 windowGlass(0.22f, 0.35f, 0.37f);
    const glm::vec3 tireRubber(0.09f, 0.10f, 0.10f);
    constexpr char busAssetPath[] = "Scenes/Nuketown/Assets/Bus/BUS.3DS";

    constexpr float eyeHeight = 1.73f;

    CCamera MakeStartingCamera()
    {
        return CCamera({-25.0f, eyeHeight, -3.5f}, {0.0f, 1.0f, 0.0f}, 36.0f, -2.5f);
    }

    CCamera MakeOverviewCamera()
    {
        CCamera overview({0.0f, 48.0f, 36.0f}, {0.0f, 1.0f, 0.0f}, -90.0f, -53.13f);
        overview.Zoom = 60.0f;
        return overview;
    }

    glm::mat4 PlaceInMap(glm::vec3 position, float yaw)
    {
        return glm::rotate(glm::translate(glm::mat4(1.0f), position),
            glm::radians(yaw), glm::vec3(0.0f, 1.0f, 0.0f));
    }
}

SceneNuketown::SceneNuketown() : camera(MakeStartingCamera()), inspectionCamera(MakeOverviewCamera())
{
    shader.LoadShader("Nuketown", "Scenes/Nuketown/Nuketown.vert", "Scenes/Nuketown/Nuketown.frag");

    InitializeRenderingResources();

    // Reuse the image loader and texture resources supplied in the class base.
    surfaceTextures.LoadTextureAnisotropic(0, "Scenes/Textures/grass.bmp");
    surfaceTextures.LoadTextureAnisotropic(1, "Scenes/Textures/mud.bmp");
    surfaceTextures.LoadTextureAnisotropic(2, "Scenes/Textures/P1.jpg");
    surfaceTextures.LoadTextureAnisotropic(3, "Scenes/Textures/R1.bmp");
    surfaceTextures.LoadTextureAnisotropic(4, "Scenes/Nuketown/Assets/Signs/NUKETOWN.BMP");
    surfaceTextures.LoadTextureAnisotropic(5, "Scenes/Nuketown/Assets/Signs/POLICE.BMP");
    surfaceTextures.LoadTextureAnisotropic(6, "Scenes/Nuketown/Assets/Signs/DELIVERY.BMP");
    surfaceTextures.LoadTextureAnisotropic(7, "Scenes/Textures/rock.jpg");

    box.generateCube(1.0f);
    cylinder.generateCylinder(1.0f, 1.0f, 72);
    sphere.generateSphere(1.0f, 24, 12);
    pyramid.generatePyramid(1.0f, 1.0f);
    roundedBox.generateRoundedCube(0.10f, 5);
    carBody.generateCarBody();
    carCabin.generateCarCabin();
    truckHood.generateTruckHood();
    tire.generateWheelTorus();
    desertBasin.generateDesertBasin();

    box.setupBuffers();
    cylinder.setupBuffers();
    sphere.setupBuffers();
    pyramid.setupBuffers();
    roundedBox.setupBuffers(); carBody.setupBuffers(); carCabin.setupBuffers();
    truckHood.setupBuffers(); tire.setupBuffers(); desertBasin.setupBuffers();

    BuildEnvironment();
    BuildStreet();
    BuildSpawnYard();
    BuildHouses();
    BuildEastYard();
    BuildWestGarden();
    BuildStreetLighting();
    BuildTestSiteFixtures();
    BuildPerimeter();
    BuildClockTower();
    BuildBus();
    BuildTruck();
    BuildLandmarks();
    BakeStreetLightMap();
}

SceneNuketown::~SceneNuketown()
{
    glDeleteFramebuffers(1, &shadowFramebuffer);
    glDeleteTextures(1, &shadowTexture);
    glDeleteTextures(1, &environmentTexture);
    glDeleteTextures(1, &streetLightTexture);
    glDeleteVertexArrays(1, &skyVao);
}

void SceneNuketown::DrawScene(float)
{
    int width = 0;
    int height = 0;
    glfwGetFramebufferSize(glfwGetCurrentContext(), &width, &height);
    if (width == 0 || height == 0)
        return;

    if (!shadowReady)
        RenderShadowMap();
    glClearColor(hazeColor.r, hazeColor.g, hazeColor.b, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    const float aspect = static_cast<float>(width) / static_cast<float>(height);
    const glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), aspect, Z_NEAR, Z_FAR);
    const glm::vec3 viewPosition = camera.Position;
    const glm::mat4 view = camera.GetViewMatrix();
    const glm::mat4 viewProjection = projection * view;
    DrawSky(viewProjection, viewPosition);
    shader.Use("Nuketown");
    shader.SetMat4("uViewProjection", viewProjection);
    shader.SetMat4("uLightViewProjection", lightViewProjection);
    shader.SetVec3("uSunDirection", SunDirection());
    shader.SetInt("uStreetLightMap", 10);
    glActiveTexture(GL_TEXTURE10);
    glBindTexture(GL_TEXTURE_2D, streetLightTexture);
    shader.SetInt("uEnvironmentMap", 9);
    glActiveTexture(GL_TEXTURE9);
    glBindTexture(GL_TEXTURE_CUBE_MAP, environmentTexture);
    shader.SetInt("uShadowMap", 8);
    glActiveTexture(GL_TEXTURE8);
    glBindTexture(GL_TEXTURE_2D, shadowTexture);
    shader.SetVec3("uCameraPosition", viewPosition);
    shader.SetVec3("uHazeColor", hazeColor);
    shader.SetVec3("uGarageLampPosition", garageLampPosition);
    shader.SetVec3("uSecondGarageLampPosition", secondGarageLampPosition);
    const int lightCount = static_cast<int>(std::min<size_t>(streetLightPositions.size(), 8));
    shader.SetInt("uStreetLightCount", lightCount);
    for (int index = 0; index < lightCount; ++index)
        shader.SetVec3("uStreetLightPositions[" + std::to_string(index) + "]", streetLightPositions[index]);

    shader.SetInt("uGrassMap", 0);
    shader.SetInt("uSoilMap", 1);
    shader.SetInt("uWoodMap", 2);
    shader.SetInt("uRoofMap", 3);
    shader.SetInt("uSignMap", 4);
    shader.SetInt("uPoliceMap", 5);
    shader.SetInt("uDeliveryMap", 6);
    shader.SetInt("uStoneMap", 7);
    for (unsigned int index = 0; index < 8; ++index)
    {
        glActiveTexture(GL_TEXTURE0 + index);
        surfaceTextures.ApplyTexture(index);
    }
    glActiveTexture(GL_TEXTURE0);
    shader.SetBool("uUseAssetMaterials", false);
    // Cull static pieces outside the camera's six frustum planes. Bounds and
    // normal matrices are computed once, when the map is built.
    const glm::mat4 rows = glm::transpose(viewProjection);
    glm::vec4 planes[] = {rows[3] + rows[0], rows[3] - rows[0],
        rows[3] + rows[1], rows[3] - rows[1], rows[3] + rows[2], rows[3] - rows[2]};
    for (glm::vec4& plane : planes)
        plane /= glm::length(glm::vec3(plane));
    for (const Part& part : parts)
    {
        bool visible = true;
        for (const glm::vec4& plane : planes)
            if (glm::dot(glm::vec3(plane), part.boundsCenter) + plane.w < -part.boundsRadius)
            {
                visible = false;
                break;
            }
        if (!visible)
            continue;
        shader.SetMat4("uModel", part.model);
        shader.SetMat3("uNormalMatrix", part.normalMatrix);
        shader.SetVec3("uColor", part.color);
        shader.SetVec3("uObjectSize", part.textureSize);
        shader.SetInt("uSurface", static_cast<int>(part.surface));
        const bool softShadow = part.surface == Surface::Shadow;
        if (softShadow)
        {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glDepthMask(GL_FALSE);
        }
        part.mesh->render();
        if (softShadow)
        {
            glDepthMask(GL_TRUE);
            glDisable(GL_BLEND);
        }
    }

    shader.SetInt("uSurface", static_cast<int>(Surface::Solid));

    if (busAsset)
    {
        shader.SetBool("uUseAssetMaterials", true);
        shader.SetMat4("uModel", busModel);
        shader.SetMat3("uNormalMatrix", glm::transpose(glm::inverse(glm::mat3(busModel))));
        busAsset->Draw(shader.GetProgram("Nuketown"));
        glActiveTexture(GL_TEXTURE0);
    }
    DrawMenu();
}

void SceneNuketown::ProcessSceneInput(GLFWwindow*, float deltaTime)
{
    const InputManager& input = InputManager::GetInstance();
    if (input.IsKeyJustPressed(GLFW_KEY_ENTER))
    {
        menuOpen = !menuOpen;
        InputManager::GetInstance().SetCursorMode(menuOpen ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
        return;
    }

    if (input.IsKeyJustPressed(GLFW_KEY_F6))
    {
        ToggleFlyCamera();
        return;
    }

    if (menuOpen)
        return;

    if (input.IsKeyJustPressed(GLFW_KEY_R))
    {
        if (flyCameraEnabled)
            ResetFlyCamera();
        else
            camera = MakeStartingCamera();
        return;
    }

    const glm::vec2 mouse = input.GetMouseDelta();
    camera.ProcessMouseMovement(mouse.x, mouse.y);
    const float forwardInput = static_cast<float>(input.IsKeyPressed(GLFW_KEY_W))
        - static_cast<float>(input.IsKeyPressed(GLFW_KEY_S));
    const float sideInput = static_cast<float>(input.IsKeyPressed(GLFW_KEY_D))
        - static_cast<float>(input.IsKeyPressed(GLFW_KEY_A));
    if (flyCameraEnabled)
    {
        const float verticalInput = static_cast<float>(input.IsKeyPressed(GLFW_KEY_SPACE))
            - static_cast<float>(input.IsKeyPressed(GLFW_KEY_LEFT_CONTROL));
        MoveFlyCamera(camera.Front * forwardInput + camera.Right * sideInput
            + camera.WorldUp * verticalInput, deltaTime, input.IsKeyPressed(GLFW_KEY_LEFT_SHIFT));
        return;
    }
    if (forwardInput == 0.0f && sideInput == 0.0f)
        return;

    const glm::vec2 forward = glm::normalize(glm::vec2(camera.Front.x, camera.Front.z));
    const glm::vec2 right(camera.Right.x, camera.Right.z);
    const glm::vec2 direction = glm::normalize(forward * forwardInput + right * sideInput);
    const float speed = input.IsKeyPressed(GLFW_KEY_LEFT_SHIFT) ? 6.3f : 3.6f;
    const float stepTime = std::clamp(deltaTime, 0.0f, 0.05f);
    const glm::vec3 feet = camera.Position - glm::vec3(0.0f, eyeHeight, 0.0f);
    camera.Position = traversal.MoveFeet(feet, direction * (speed * stepTime))
        + glm::vec3(0.0f, eyeHeight, 0.0f);
}

void SceneNuketown::ToggleFlyCamera()
{
    if (flyCameraEnabled)
    {
        inspectionCamera = camera;
        camera = savedWalkingCamera;
    }
    else
    {
        savedWalkingCamera = camera;
        camera = inspectionCamera;
    }
    flyCameraEnabled = !flyCameraEnabled;
}

void SceneNuketown::ResetFlyCamera()
{
    inspectionCamera = MakeOverviewCamera();
    if (flyCameraEnabled)
        camera = inspectionCamera;
}

void SceneNuketown::MoveFlyCamera(glm::vec3 direction, float deltaTime, bool accelerated)
{
    const float distance = glm::length(direction);
    if (!flyCameraEnabled || distance < 0.00001f)
        return;

    // Inspection ignores scene collisions, while the walking camera stays saved.
    const float speed = accelerated ? 32.0f : 12.0f;
    const float stepTime = std::clamp(deltaTime, 0.0f, 0.05f);
    camera.Position += direction / distance * speed * stepTime;
    camera.Position = glm::clamp(camera.Position,
        glm::vec3(-350.0f, 0.5f, -350.0f), glm::vec3(350.0f, 180.0f, 350.0f));
}

void SceneNuketown::DrawMenu()
{
    if (!menuOpen)
        return;

    ImGui::SetNextWindowPos(ImVec2(12.0f, 12.0f), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Nuketown", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse))
    {
        bool requestedFlight = flyCameraEnabled;
        if (ImGui::Checkbox("Camera livre (F6)", &requestedFlight))
            ToggleFlyCamera();
        if (flyCameraEnabled && ImGui::Button("Vista geral (R)"))
            ResetFlyCamera();
        if (ImGui::Button(flyCameraEnabled ? "Ir ao quintal verde" : "Quintal verde (R)"))
        {
            if (flyCameraEnabled)
                ToggleFlyCamera();
            camera = MakeStartingCamera();
        }
        if (ImGui::Button("Ir ao quintal amarelo"))
        {
            if (flyCameraEnabled)
                ToggleFlyCamera();
            camera = CCamera({25.0f, eyeHeight, 3.0f}, {0.0f, 1.0f, 0.0f}, -145.0f, -2.5f);
        }
        ImGui::Separator();
        ImGui::TextUnformatted("Mapa completo | Exteriores");
        ImGui::TextUnformatted(flyCameraEnabled ? "WASD  voar | Mouse  olhar" : "WASD  andar | Mouse  olhar");
        if (flyCameraEnabled)
            ImGui::TextUnformatted("Espaco  subir | Ctrl esquerdo  descer");
        ImGui::TextUnformatted(flyCameraEnabled ? "Shift  acelerar | R  vista geral" : "Shift  correr | R  inicio");
        ImGui::TextUnformatted("F6  camera livre | F11  tela cheia");
        ImGui::TextUnformatted("Enter  fechar menu");
    }
    ImGui::End();
}

void SceneNuketown::AddPart(CPrimitives& mesh, glm::vec3 center, glm::vec3 size, glm::vec3 color,
    const glm::mat4& parent, Surface surface)
{
    const glm::mat4 model = glm::scale(glm::translate(parent, center), size);
    auto bounds = meshBounds.find(&mesh);
    if (bounds == meshBounds.end())
    {
        glm::vec3 minimum(1.0e9f), maximum(-1.0e9f);
        for (const PrimVertex& vertex : mesh.getVertices())
        {
            const glm::vec3 position(vertex.position[0], vertex.position[1], vertex.position[2]);
            minimum = glm::min(minimum, position);
            maximum = glm::max(maximum, position);
        }
        bounds = meshBounds.emplace(&mesh, MeshBounds{(minimum + maximum) * 0.5f,
            (maximum - minimum) * 0.5f}).first;
    }
    parts.push_back({&mesh, model, color, size, surface,
        glm::transpose(glm::inverse(glm::mat3(model))),
        glm::vec3(model * glm::vec4(bounds->second.center, 1.0f)),
        glm::length(size * bounds->second.half)});
}

void SceneNuketown::AddBox(glm::vec3 center, glm::vec3 size, glm::vec3 color, const glm::mat4& parent, Surface surface)
{
    AddPart(box, center, size, color, parent, surface);
}

void SceneNuketown::AddBeam(glm::vec3 start, glm::vec3 end, float width, glm::vec3 color, Surface surface)
{
    const glm::vec3 span = end - start;
    const glm::vec3 up = glm::normalize(span);
    const glm::vec3 reference = std::abs(up.z) < 0.95f ? glm::vec3(0.0f, 0.0f, 1.0f) : glm::vec3(1.0f, 0.0f, 0.0f);
    const glm::vec3 right = glm::normalize(glm::cross(up, reference));
    const glm::vec3 forward = glm::cross(right, up);
    const glm::mat4 model(glm::vec4(right * width, 0.0f), glm::vec4(span, 0.0f),
        glm::vec4(forward * width, 0.0f), glm::vec4((start + end) * 0.5f, 1.0f));
    const glm::vec3 size(width, glm::length(span), width);
    parts.push_back({&box, model, color, size, surface,
        glm::transpose(glm::inverse(glm::mat3(model))),
        (start + end) * 0.5f, glm::length(size) * 0.5f});
}

void SceneNuketown::AddPath(glm::vec2 start, glm::vec2 end, float width, float top, glm::vec3 color, Surface surface)
{
    const glm::vec2 span = end - start;
    const glm::vec2 center = (start + end) * 0.5f;
    const float yaw = glm::degrees(std::atan2(span.x, span.y));
    const glm::mat4 segment = PlaceInMap({center.x, top - 0.024f, center.y}, yaw);
    AddBox({0.0f, 0.0f, 0.0f}, {width, 0.048f, glm::length(span) + 0.08f}, color, segment, surface);   
}

void SceneNuketown::AddFence(glm::vec2 start, glm::vec2 end, float height, glm::vec3 color)
{
    const glm::vec2 span = end - start;
    const float length = glm::length(span);
    const glm::vec2 center = (start + end) * 0.5f;
    const float yaw = glm::degrees(std::atan2(span.x, span.y));
    const glm::mat4 fence = PlaceInMap({center.x, 0.0f, center.y}, yaw);
    traversal.AddObstacle(center, {0.12f, length * 0.5f + 0.09f}, yaw);

    AddBox({0.0f, height * 0.5f + 0.09f, 0.0f}, {0.15f, height, length}, color, fence, Surface::Wood);
    const int posts = static_cast<int>(std::ceil(length / 1.8f));
    for (int post = 0; post <= posts; ++post)
    {
        const float offset = length * (static_cast<float>(post) / posts - 0.5f);
        AddBox({0.0f, height * 0.5f + 0.15f, offset}, {0.24f, height + 0.14f, 0.23f},
            color * 0.82f, fence, Surface::Wood);
    }
    for (float y : {height * 0.31f, height * 0.78f})
        AddBox({0.0f, y, 0.0f}, {0.20f, 0.095f, length}, color * 0.88f, fence, Surface::Wood);
}

void SceneNuketown::BuildStreet()
{
    const glm::vec3 grass(0.36f, 0.43f, 0.24f);
    const glm::vec3 timber(0.34f, 0.28f, 0.20f);


    // The oval is elongated along the approach road, as in the overhead reference.
    constexpr float centerZ = -0.85f;
    constexpr float radiusX = 6.45f;
    constexpr float radiusZ = 8.70f;
    constexpr float stemLeft = -3.65f;
    constexpr float stemRight = 4.55f;
    constexpr float endZ = 19.2f;
    constexpr float stemCenter = (stemRight + stemLeft) * 0.5f;
    constexpr float stemWidth = stemRight - stemLeft;
    AddPart(cylinder, {0.0f, -0.018f, centerZ}, {radiusX + 0.92f, 0.074f, radiusZ + 0.85f}, concrete, glm::mat4(1.0f), Surface::Concrete);
    AddPath({stemCenter, 3.9f}, {stemCenter, endZ}, stemWidth + 1.8f, 0.018f, concrete);
    AddPart(cylinder, {0.0f, 0.016f, centerZ}, {radiusX, 0.065f, radiusZ}, asphalt, glm::mat4(1.0f), Surface::Asphalt);
    AddPath({stemCenter, 3.9f}, {stemCenter, endZ}, stemWidth, 0.05f, asphalt, Surface::Asphalt);

    // Continue the curb along the two straight sides of the road at the oval joints.
    const float start = -std::acos(stemRight / radiusX);
    const float end = glm::radians(360.0f) - std::acos(stemLeft / radiusX);
    const auto curbPoint = [centerZ, radiusX, radiusZ](float angle)
    {
        return glm::vec3(radiusX * std::cos(angle), 0.099f, centerZ - radiusZ * std::sin(angle));
    };
    constexpr int segments = 46;
    for (int index = 0; index < segments; ++index)
    {
        const float angle = glm::mix(start, end, static_cast<float>(index) / segments);
        const float next = glm::mix(start, end, static_cast<float>(index + 1) / segments);
        AddBeam(curbPoint(angle), curbPoint(next), 0.12f, concrete, Surface::Concrete);
    }
    AddBeam(curbPoint(start), {stemRight, 0.099f, endZ}, 0.12f, concrete, Surface::Concrete);
    AddBeam(curbPoint(end), {stemLeft, 0.099f, endZ}, 0.12f, concrete, Surface::Concrete);

    AddPath({-9.1f, -8.2f}, {-10.5f, 5.7f}, 5.6f, -0.005f, grass, Surface::Grass);
    AddPath({5.8f, -13.7f}, {13.0f, 17.4f}, 2.0f, -0.004f, grass, Surface::Grass);
    AddPath({-9.0f, 5.6f}, {-9.0f, 7.98f}, 2.6f, -0.005f, grass, Surface::Grass);
    AddPath({-8.9f, 7.98f}, {-5.30f, 11.80f}, 1.80f, -0.005f, grass, Surface::Grass);
    AddPath({-4.93f, 11.80f}, {-4.93f, 18.8f}, 1.45f, -0.005f, grass, Surface::Grass);
    AddPath({-6.95f, 5.0f}, {-4.3f, 4.95f}, 2.34f, 0.040f, concrete);

    AddPart(cylinder, {-5.1f, 3.65f, 9.8f}, {0.14f, 7.3f, 0.14f}, timber);
    AddBox({-5.1f, 6.73f, 9.8f}, {2.5f, 0.16f, 0.19f}, timber);
    AddBox({-5.1f, 6.0f, 9.8f}, {1.95f, 0.13f, 0.16f}, timber);
    traversal.AddObstacle({-5.1f, 9.8f}, {0.15f, 0.15f});
    AddPart(cylinder, {-6.9f, 3.15f, -6.65f}, {0.11f, 6.3f, 0.11f}, timber);
    AddBox({-6.9f, 5.9f, -6.65f}, {2.0f, 0.13f, 0.16f}, timber);
    traversal.AddObstacle({-6.9f, -6.65f}, {0.12f, 0.12f});
    AddBeam({-4.10f, 6.75f, 9.8f}, {-5.5f, 5.23f, 1.75f}, 0.025f, timber);
    AddBeam({-5.5f, 5.23f, 1.75f}, {-6.0f, 5.95f, -6.65f}, 0.025f, timber);


}

void SceneNuketown::BuildSpawnYard()
{
    const glm::vec3 lawn(0.38f, 0.46f, 0.26f);
    const glm::vec3 soil(0.46f, 0.34f, 0.23f);
    AddPath({-23.75f, -13.5f}, {-22.05f, 2.55f}, 10.8f, -0.003f, lawn, Surface::Grass);
    AddPath({-17.5f, -14.1f}, {-13.3f, -1.6f}, 8.2f, -0.004f, lawn, Surface::Grass);
    AddPath({-25.45f, -4.5f}, {-24.30f, 2.95f}, 2.05f, 0.01f, soil, Surface::Dirt);
    AddPath({-24.3f, 2.95f}, {-18.7f, 4.46f}, 1.89f, 0.022f, concrete);
    AddPath({-18.7f, 4.46f}, {-13.1f, 4.94f}, 2.16f, 0.026f, concrete);
    AddPath({-13.1f, 4.94f}, {-6.95f, 5.0f}, 2.34f, 0.031f, concrete);

    const glm::mat4 shed = PlaceInMap({-26.6f, 0.0f, -8.8f}, -18.0f);
    const glm::vec3 siding(0.39f, 0.49f, 0.44f);
    AddBox({0.0f, 1.08f, 0.0f}, {2.4f, 2.16f, 3.8f}, siding, shed, Surface::Siding);
    AddPart(pyramid, {0.0f, 2.14f, 0.0f}, {2.80f, 0.60f, 4.15f}, {0.36f, 0.39f, 0.37f}, shed, Surface::Roof);
    AddBox({0.15f, 0.94f, 1.925f}, {0.94f, 1.74f, 0.09f}, {0.40f, 0.32f, 0.23f}, shed, Surface::Wood);
    traversal.AddObstacle({-26.6f, -8.8f}, {1.22f, 1.93f}, -18.0f);

    const glm::vec3 stone(0.68f, 0.65f, 0.56f);
    for (int index = 0; index < 6; ++index)
    {
        const float t = static_cast<float>(index) / 5.0f;
        AddBox({glm::mix(-24.65f, -22.0f, t), 0.026f, glm::mix(-2.3f, 2.95f, t)},
            {0.64f, 0.05f, 0.51f}, stone, glm::mat4(1.0f), Surface::Concrete);
    }
}

void SceneNuketown::AddWheels(const glm::mat4& vehicle, float halfAxle, float radius, float frontZ, float rearZ)
{
    for (float side : {-1.0f, 1.0f})
        for (float z : {frontZ, rearZ})
        {
            const glm::mat4 wheel = glm::rotate(glm::translate(vehicle, {side * halfAxle, radius + 0.05f, z}),
                glm::radians(side * 90.0f), {0.0f, 1.0f, 0.0f});
            AddPart(tire, {0.0f, 0.0f, 0.0f}, {radius, radius, 0.56f}, {0.08f, 0.085f, 0.09f}, wheel, Surface::Tire);
            const glm::mat4 hub = glm::rotate(glm::translate(wheel, {0.0f, 0.0f, 0.10f}),
                glm::radians(90.0f), {1.0f, 0.0f, 0.0f});
            AddPart(cylinder, {0.0f, 0.0f, 0.0f}, {radius * 0.72f, 0.055f, radius * 0.72f},
                {0.51f, 0.54f, 0.52f}, hub, Surface::Chrome);
            AddPart(sphere, {0.0f, 0.0f, 0.15f}, {radius * 0.48f, radius * 0.48f, 0.035f},
                {0.67f, 0.69f, 0.66f}, wheel, Surface::Chrome);
            for (int bolt = 0; bolt < 5; ++bolt)
            {
                const float angle = glm::radians(bolt * 72.0f);
                AddPart(sphere, {std::cos(angle) * radius * 0.28f, std::sin(angle) * radius * 0.28f, 0.19f},
                    {0.018f, 0.018f, 0.012f}, {0.69f, 0.70f, 0.67f}, wheel, Surface::Chrome);
            }
        }
}

void SceneNuketown::BuildBus()
{
    const glm::vec2 center(-1.55f, 3.90f);
    constexpr float heading = 12.0f;
    constexpr float busLength = 8.20f;
    const glm::mat4 vehicle = PlaceInMap({center.x, 0.0f, center.y}, heading);
    AddPart(cylinder, {0.0f, 0.051f, 0.0f}, {1.83f, 0.012f, 4.53f}, {0.19f, 0.21f, 0.21f}, vehicle, Surface::Shadow);

    std::error_code fileError;
    if (std::filesystem::is_regular_file(busAssetPath, fileError))
    {
        auto asset = std::make_unique<CAsset>(busAssetPath);
        glm::vec3 minimum, maximum;
        if (asset->TryGetBounds(minimum, maximum))
        {
            const glm::vec3 size = maximum - minimum;
            if (size.x > 0.0f && size.y > 0.0f && size.z > 0.001f &&
                std::isfinite(size.x) && std::isfinite(size.y) && std::isfinite(size.z))
            {
                // Assimp converts the Max coordinate system to Y up and positive Z forward.
                const float scale = busLength / size.z;
                const glm::vec3 pivot((minimum.x + maximum.x) * 0.5f, minimum.y,
                    (minimum.z + maximum.z) * 0.5f);
                busModel = glm::translate(vehicle, {0.0f, 0.05f, 0.0f});
                busModel = glm::scale(busModel, glm::vec3(scale));
                busModel = glm::translate(busModel, -pivot);
                traversal.AddObstacle(center, {size.x * scale * 0.5f, busLength * 0.5f}, heading);
                busAsset = std::move(asset);
                std::cout << "Onibus 3DS carregado: " << busAssetPath << std::endl;
                return;
            }
        }
        std::cerr << "Onibus 3DS invalido. Usando o modelo provisorio." << std::endl;
    }

    traversal.AddObstacle(center, {1.32f, 4.16f}, heading);
    const glm::vec3 yellow(0.89f, 0.65f, 0.19f);
    const glm::vec3 straw(0.93f, 0.75f, 0.34f);
    const glm::vec3 roof(0.71f, 0.73f, 0.68f);
    AddBox({0.0f, 1.01f, -0.16f}, {2.48f, 0.91f, 7.70f}, yellow, vehicle);
    AddBox({0.0f, 1.96f, -0.17f}, {2.45f, 1.04f, 7.61f}, straw, vehicle);
    AddBox({0.0f, 2.54f, -0.19f}, {2.51f, 0.20f, 7.62f}, roof, vehicle);
    AddBox({0.0f, 2.67f, -0.19f}, {2.20f, 0.12f, 7.50f}, roof, vehicle);
    AddPart(sphere, {0.0f, 2.50f, 3.50f}, {1.25f, 0.19f, 0.44f}, roof, vehicle);
    AddBox({0.0f, 1.08f, 3.73f}, {2.35f, 1.00f, 0.58f}, yellow, vehicle);
    AddBox({0.0f, 0.54f, 4.00f}, {2.58f, 0.22f, 0.28f}, {0.29f, 0.30f, 0.27f}, vehicle);
    AddBox({0.0f, 0.95f, 4.04f}, {1.23f, 0.32f, 0.064f}, {0.31f, 0.29f, 0.22f}, vehicle);

    for (float side : {-1.0f, 1.0f})
    {
        AddBox({side * 0.56f, 1.99f, 3.661f}, {1.01f, 0.61f, 0.075f}, windowGlass, vehicle, Surface::Glass);
        AddPart(sphere, {side * 0.94f, 1.08f, 4.055f}, {0.15f, 0.15f, 0.067f}, warmWhite, vehicle);
        AddPart(sphere, {side * 1.03f, 2.38f, 3.68f}, {0.087f, 0.083f, 0.060f}, {0.81f, 0.28f, 0.12f}, vehicle);
        for (float y : {1.35f, 1.50f})
            AddBox({side * 1.249f, y, -0.16f}, {0.042f, 0.10f, 7.70f}, tireRubber, vehicle);
        for (int window = 0; window < 8; ++window)
        {
            const float z = -3.44f + window * 0.89f;
            AddBox({side * 1.23f, 1.99f, z}, {0.06f, 0.62f, 0.72f}, windowGlass, vehicle, Surface::Glass);
            AddBox({side * 1.266f, 2.05f, z}, {0.035f, 0.033f, 0.72f}, roof, vehicle);
        }
    }
    AddWheels(vehicle, 1.13f, 0.53f, 2.43f, -2.65f);
}
