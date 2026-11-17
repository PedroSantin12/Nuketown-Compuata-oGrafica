#include "SceneNuketown.h"
#include "Engine/CAsset.h"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <iterator>

namespace
{
    const glm::vec3 hazeColor(0.71f, 0.71f, 0.63f);
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

    glm::mat4 PlaceInMap(glm::vec3 position, float yaw)
    {
        return glm::rotate(glm::translate(glm::mat4(1.0f), position),
            glm::radians(yaw), glm::vec3(0.0f, 1.0f, 0.0f));
    }
}

SceneNuketown::SceneNuketown() : camera(MakeStartingCamera())
{
    shader.LoadShader("Nuketown", "Scenes/Nuketown/Nuketown.vert", "Scenes/Nuketown/Nuketown.frag");

    // Reuse the image loader and texture resources supplied in the class base.
    surfaceTextures.LoadTextureAnisotropic(0, "Scenes/Textures/grass.bmp");
    surfaceTextures.LoadTextureAnisotropic(1, "Scenes/Textures/mud.bmp");
    surfaceTextures.LoadTextureAnisotropic(2, "Scenes/Textures/P1.jpg");
    surfaceTextures.LoadTextureAnisotropic(3, "Scenes/Textures/R1.bmp");

    box.generateCube(1.0f);
    cylinder.generateCylinder(1.0f, 1.0f, 72);
    sphere.generateSphere(1.0f, 24, 12);
    pyramid.generatePyramid(1.0f, 1.0f);

    box.setupBuffers();
    cylinder.setupBuffers();
    sphere.setupBuffers();
    pyramid.setupBuffers();

    BuildStreet();
    BuildSpawnYard();
    BuildHouses();
    BuildPerimeter();
    BuildClockTower();
    BuildBus();
    BuildTruck();
}

SceneNuketown::~SceneNuketown() = default;

void SceneNuketown::DrawScene(float)
{
    int width = 0;
    int height = 0;
    glfwGetFramebufferSize(glfwGetCurrentContext(), &width, &height);
    if (width == 0 || height == 0)
        return;

    glClearColor(hazeColor.r, hazeColor.g, hazeColor.b, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    const float aspect = static_cast<float>(width) / static_cast<float>(height);
    const glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), aspect, Z_NEAR, Z_FAR);
    const glm::vec3 viewPosition = GetViewPosition();
    const glm::mat4 view = thirdPerson
        ? glm::lookAt(viewPosition, camera.Position + camera.Front * 3.0f - glm::vec3(0.0f, 0.65f, 0.0f), camera.WorldUp)
        : camera.GetViewMatrix();
    shader.Use("Nuketown");
    shader.SetMat4("uViewProjection", projection * view);
    shader.SetVec3("uCameraPosition", viewPosition);
    shader.SetVec3("uHazeColor", hazeColor);
    shader.SetVec3("uGarageLampPosition", garageLampPosition);

    shader.SetInt("uGrassMap", 0);
    shader.SetInt("uSoilMap", 1);
    shader.SetInt("uWoodMap", 2);
    shader.SetInt("uRoofMap", 3);
    for (unsigned int index = 0; index < 4; ++index)
    {
        glActiveTexture(GL_TEXTURE0 + index);
        surfaceTextures.ApplyTexture(index);
    }
    glActiveTexture(GL_TEXTURE0);
    shader.SetBool("uUseAssetMaterials", false);
    for (const Part& part : parts)
    {
        shader.SetMat4("uModel", part.model);
        shader.SetMat3("uNormalMatrix", glm::transpose(glm::inverse(glm::mat3(part.model))));
        shader.SetVec3("uColor", part.color);
        shader.SetVec3("uObjectSize", part.textureSize);
        shader.SetInt("uSurface", static_cast<int>(part.surface));
        part.mesh->render();
    }

    shader.SetInt("uSurface", static_cast<int>(Surface::Solid));
    if (thirdPerson && glm::distance(viewPosition, camera.Position) > 0.85f)
        DrawAvatar();

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
    }

    if (menuOpen)
        return;

    if (input.IsKeyJustPressed(GLFW_KEY_F5))
        thirdPerson = !thirdPerson;

    if (input.IsKeyJustPressed(GLFW_KEY_R))
    {
        camera = MakeStartingCamera();
        return;
    }

    const glm::vec2 mouse = input.GetMouseDelta();
    camera.ProcessMouseMovement(mouse.x, mouse.y);
    const float forwardInput = static_cast<float>(input.IsKeyPressed(GLFW_KEY_W))
        - static_cast<float>(input.IsKeyPressed(GLFW_KEY_S));
    const float sideInput = static_cast<float>(input.IsKeyPressed(GLFW_KEY_D))
        - static_cast<float>(input.IsKeyPressed(GLFW_KEY_A));
    if (forwardInput == 0.0f && sideInput == 0.0f)
        return;

    const glm::vec2 forward = glm::normalize(glm::vec2(camera.Front.x, camera.Front.z));
    const glm::vec2 right(camera.Right.x, camera.Right.z);
    const glm::vec2 direction = glm::normalize(forward * forwardInput + right * sideInput);
    const float speed = input.IsKeyPressed(GLFW_KEY_LEFT_SHIFT) ? 6.3f : 3.6f;
    const float stepTime = std::clamp(deltaTime, 0.0f, 0.05f);
    const glm::vec2 position = traversal.Move({camera.Position.x, camera.Position.z},
        direction * (speed * stepTime));
    camera.Position = {position.x, eyeHeight, position.y};
}

glm::vec3 SceneNuketown::GetViewPosition() const
{
    if (!thirdPerson)
        return camera.Position;

    glm::vec3 desired = camera.Position - camera.Front * cameraDistance + glm::vec3(0.0f, 0.8f, 0.0f);
    desired.y = std::max(desired.y, 0.35f);
    return traversal.ConstrainCamera(camera.Position, desired);
}

void SceneNuketown::DrawAvatar()
{
    const glm::mat4 avatar = PlaceInMap({camera.Position.x, 0.05f, camera.Position.z}, 90.0f - camera.Yaw);
    const auto draw = [this, &avatar](CPrimitives& mesh, glm::vec3 center, glm::vec3 size, glm::vec3 color)
    {
        const glm::mat4 model = glm::scale(glm::translate(avatar, center), size);
        shader.SetMat4("uModel", model);
        shader.SetMat3("uNormalMatrix", glm::transpose(glm::inverse(glm::mat3(model))));
        shader.SetVec3("uColor", color);
        mesh.render();
    };
    const glm::vec3 clothing(0.27f, 0.36f, 0.40f);
    const glm::vec3 trousers(0.23f, 0.26f, 0.24f);
    const glm::vec3 skin(0.70f, 0.54f, 0.39f);
    draw(box, {0.0f, 1.15f, 0.0f}, {0.48f, 0.60f, 0.28f}, clothing);
    draw(box, {0.0f, 0.79f, 0.0f}, {0.36f, 0.19f, 0.25f}, trousers);
    draw(sphere, {0.0f, 1.60f, 0.0f}, {0.17f, 0.19f, 0.16f}, skin);
    draw(box, {0.0f, 1.64f, 0.151f}, {0.23f, 0.06f, 0.045f}, tireRubber);
    for (float side : {-1.0f, 1.0f})
    {
        draw(box, {side * 0.105f, 0.42f, 0.0f}, {0.15f, 0.65f, 0.18f}, trousers);
        draw(box, {side * 0.105f, 0.09f, 0.045f}, {0.18f, 0.16f, 0.29f}, tireRubber);
        draw(box, {side * 0.33f, 1.13f, 0.0f}, {0.14f, 0.54f, 0.17f}, clothing);
        draw(sphere, {side * 0.33f, 0.82f, 0.0f}, {0.073f, 0.083f, 0.072f}, skin);
    }
}

void SceneNuketown::DrawMenu()
{
    if (!menuOpen)
        return;

    ImGui::SetNextWindowPos(ImVec2(12.0f, 12.0f), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Nuketown", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse))
    {
        ImGui::Checkbox("Terceira pessoa (F5)", &thirdPerson);
        if (thirdPerson)
            ImGui::SliderFloat("Distancia da camera", &cameraDistance, 1.8f, 5.0f, "%.1f m");
        if (ImGui::Button("Voltar ao ponto inicial (R)"))
            camera = MakeStartingCamera();
        ImGui::Separator();
        ImGui::TextUnformatted("WASD  andar | Mouse  olhar");
        ImGui::TextUnformatted("Shift  correr | Enter  fechar menu");
    }
    ImGui::End();
}

void SceneNuketown::AddPart(CPrimitives& mesh, glm::vec3 center, glm::vec3 size, glm::vec3 color,
    const glm::mat4& parent, Surface surface)
{
    parts.push_back({&mesh, glm::scale(glm::translate(parent, center), size), color, size, surface});
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
    parts.push_back({&box, model, color, {width, glm::length(span), width}, surface});
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
    const glm::vec3 desert(0.66f, 0.57f, 0.43f);
    const glm::vec3 grass(0.36f, 0.43f, 0.24f);
    const glm::vec3 timber(0.34f, 0.28f, 0.20f);

    AddBox({0.0f, -0.22f, -30.0f}, {280.0f, 0.36f, 240.0f}, desert, glm::mat4(1.0f), Surface::Dirt);

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

    for (float x : {-65.0f, -43.0f, -20.0f, 4.0f, 29.0f, 57.0f, 76.0f})
    {
        const float height = 9.0f + std::abs(std::sin(x * 0.16f)) * 12.0f;
        AddPart(pyramid, {x, -0.04f, -73.0f}, {37.0f, height, 27.0f}, {0.65f, 0.54f, 0.40f});
    }
    AddPart(pyramid, {36.0f, -0.04f, -53.0f}, {34.0f, 18.0f, 25.0f}, {0.67f, 0.55f, 0.42f});
    AddPart(pyramid, {-36.0f, -0.04f, -55.0f}, {31.0f, 12.0f, 25.0f}, {0.62f, 0.52f, 0.40f});
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

void SceneNuketown::BuildPerimeter()
{
    const glm::vec3 wood(0.52f, 0.40f, 0.28f);
    const glm::vec3 cutFence(0.47f, 0.43f, 0.35f);
    const glm::vec2 outline[] = {
        {-28.75f, 2.7f}, {-29.75f, -10.5f}, {-24.1f, -17.2f}, {-17.2f, -17.2f},
        {-10.45f, -14.7f}, {-4.85f, -16.8f}, {6.0f, -18.0f},
        {14.8f, 20.0f}, {6.15f, 20.0f}, {-5.85f, 20.0f}, {-5.85f, 12.1f},
        {-10.35f, 8.25f}, {-19.85f, 8.25f}
    };
    for (size_t index = 0; index < std::size(outline); ++index)
    {
        const bool eastCut = index == 6;
        AddFence(outline[index], outline[(index + 1) % std::size(outline)], eastCut ? 2.7f : 2.15f,
            eastCut ? cutFence : wood);
    }

    AddFence({-10.2f, 6.36f}, {-8.3f, 6.7f}, 1.03f, warmWhite);
    AddFence({-7.1f, 7.0f}, {-6.6f, 9.02f}, 1.03f, warmWhite);
}

void SceneNuketown::BuildHouses()
{
    const glm::vec3 mint(0.26f, 0.60f, 0.47f);
    const glm::vec3 blueGray(0.67f, 0.75f, 0.72f);
    const glm::vec3 roof(0.32f, 0.37f, 0.36f);
    const glm::vec3 wood(0.57f, 0.47f, 0.33f);
    const glm::vec3 brickRed(0.55f, 0.28f, 0.23f);
    const glm::vec3 recess(0.14f, 0.19f, 0.18f);
    constexpr float houseYaw = 68.0f;
    const glm::vec2 houseCenter(-14.1f, -6.4f);
    const glm::mat4 house = PlaceInMap({houseCenter.x, 0.0f, houseCenter.y}, houseYaw);
    traversal.AddObstacle(houseCenter, {3.75f, 4.60f}, houseYaw);

    // Separate frame bars leave the glass slightly recessed behind their front edge.
    const auto addWindow = [this, &recess](const glm::mat4& parent, glm::vec3 center,
        float width, float height, float yaw)
    {
        const glm::mat4 frame = glm::rotate(glm::translate(parent, center),
            glm::radians(yaw), glm::vec3(0.0f, 1.0f, 0.0f));
        AddBox({0.0f, 0.0f, 0.0f}, {width + 0.35f, height + 0.28f, 0.06f}, recess, frame);
        AddBox({0.0f, 0.0f, 0.075f}, {width, height, 0.035f}, windowGlass, frame, Surface::Glass);
        for (float side : {-1.0f, 1.0f})
        {
            AddBox({side * (width * 0.5f + 0.075f), 0.0f, 0.09f},
                {0.15f, height + 0.23f, 0.16f}, warmWhite, frame, Surface::Paint);
            AddBox({0.0f, side * (height * 0.5f + 0.0575f), 0.09f},
                {width, 0.115f, 0.16f}, warmWhite, frame, Surface::Paint);
        }
        AddBox({0.0f, 0.0f, 0.115f}, {0.055f, height, 0.08f}, warmWhite, frame, Surface::Paint);
        AddBox({0.0f, -0.055f, 0.115f}, {width, 0.045f, 0.08f}, warmWhite, frame, Surface::Paint);
        AddBox({0.0f, -height * 0.5f - 0.14f, 0.125f},
            {width + 0.43f, 0.095f, 0.34f}, warmWhite * 0.96f, frame, Surface::Paint);
        AddBox({0.0f, height * 0.5f + 0.145f, 0.07f},
            {width + 0.37f, 0.045f, 0.21f}, warmWhite, frame, Surface::Paint);
    };

    AddBox({0.0f, 1.30f, 0.0f}, {7.50f, 2.60f, 9.20f}, mint, house, Surface::Siding);
    AddBox({0.0f, 3.84f, 0.0f}, {7.50f, 2.48f, 9.20f}, blueGray, house, Surface::Siding);
    AddBox({0.0f, 2.63f, 0.0f}, {7.60f, 0.23f, 9.29f}, warmWhite, house, Surface::Paint);
    AddPart(pyramid, {0.0f, 5.02f, 0.0f}, {8.1f, 1.22f, 9.90f}, roof, house, Surface::Roof);
    AddBox({0.0f, 5.02f, 0.0f}, {7.91f, 0.12f, 9.65f}, warmWhite, house, Surface::Paint);
    for (float side : {-1.0f, 1.0f})
    {
        const float windowYaw = side > 0.0f ? 0.0f : 180.0f;
        for (float x : {-1.95f, 1.90f})
            addWindow(house, {x, 3.92f, side * 4.68f}, 1.78f, 0.84f, windowYaw);
        addWindow(house, {1.46f, 1.38f, side * 4.68f}, 2.09f, 0.91f, windowYaw);
        addWindow(house, {side * 3.80f, 3.71f, -1.80f}, 1.48f, 0.82f, side * 90.0f);

        const glm::mat4 door = glm::rotate(glm::translate(house,
            {-1.26f, 1.13f, side * 4.68f}), glm::radians(windowYaw), {0.0f, 1.0f, 0.0f});
        AddBox({0.0f, 0.0f, 0.0f}, {1.13f, 2.19f, 0.09f}, recess, door);
        AddBox({0.0f, 0.0f, 0.05f}, {0.97f, 2.07f, 0.065f}, wood, door, Surface::Wood);
        for (float edge : {-1.0f, 1.0f})
            AddBox({edge * 0.535f, 0.0f, 0.055f}, {0.10f, 2.19f, 0.15f},
                warmWhite, door, Surface::Paint);
        AddBox({0.0f, 1.09f, 0.055f}, {1.17f, 0.11f, 0.15f}, warmWhite, door, Surface::Paint);
        for (float y : {-0.47f, 0.43f})
            AddBox({0.0f, y, 0.092f}, {0.69f, 0.68f, 0.025f}, wood * 0.88f, door, Surface::Wood);
        AddBox({0.35f, -0.03f, 0.105f}, {0.07f, 0.16f, 0.022f}, {0.48f, 0.43f, 0.29f}, door);
        AddPart(sphere, {0.35f, -0.03f, 0.14f}, {0.038f, 0.038f, 0.038f},
            {0.63f, 0.57f, 0.38f}, door);

        // Painted corner boards finish the siding without changing the house outline.
        for (float x : {-3.77f, 3.77f})
            AddBox({x, 2.55f, side * 4.61f}, {0.12f, 4.86f, 0.10f},
                warmWhite, house, Surface::Paint);
    }
    AddBox({-1.35f, 0.09f, -4.96f}, {1.68f, 0.18f, 0.59f}, concrete, house, Surface::Concrete);

    const glm::vec3 garageCenter = glm::vec3(house * glm::vec4(-5.62f, 0.0f, 1.78f, 1.0f));
    const glm::mat4 garage = PlaceInMap(garageCenter, houseYaw);
    traversal.AddObstacle({garageCenter.x, garageCenter.z}, {2.23f, 3.24f}, houseYaw);
    AddBox({0.0f, 1.39f, 0.0f}, {4.46f, 2.78f, 6.48f}, blueGray, garage, Surface::Siding);
    AddPart(pyramid, {0.0f, 2.76f, 0.0f}, {4.96f, 0.95f, 6.94f}, roof, garage, Surface::Roof);
    AddBox({0.0f, 2.76f, 0.0f}, {4.79f, 0.11f, 6.75f}, warmWhite, garage, Surface::Paint);
    AddBox({0.0f, 1.21f, 3.28f}, {3.43f, 2.22f, 0.095f}, recess, garage);
    AddBox({0.0f, 1.21f, 3.34f}, {3.16f, 2.02f, 0.062f},
        {0.39f, 0.44f, 0.39f}, garage, Surface::Paint);
    for (float side : {-1.0f, 1.0f})
        AddBox({side * 1.67f, 1.21f, 3.37f}, {0.15f, 2.23f, 0.12f},
            warmWhite, garage, Surface::Paint);
    AddBox({0.0f, 2.31f, 3.37f}, {3.49f, 0.13f, 0.12f}, warmWhite, garage, Surface::Paint);
    for (float y = 0.35f; y < 2.15f; y += 0.36f)
        AddBox({0.0f, y, 3.39f}, {3.15f, 0.035f, 0.04f}, blueGray, garage, Surface::Paint);
    for (float x : {-1.05f, -0.35f, 0.35f, 1.05f})
    {
        AddBox({x, 1.99f, 3.39f}, {0.50f, 0.26f, 0.035f}, warmWhite, garage, Surface::Paint);
        AddBox({x, 1.99f, 3.415f}, {0.40f, 0.16f, 0.025f}, windowGlass, garage, Surface::Glass);
    }
    AddBox({0.0f, 2.48f, 3.34f}, {0.34f, 0.14f, 0.12f}, {1.0f, 0.84f, 0.53f}, garage);
    AddBox({0.0f, 0.017f, 4.53f}, {3.82f, 0.045f, 2.72f}, concrete, garage, Surface::Concrete);
    garageLampPosition = glm::vec3(garage * glm::vec4(0.0f, 2.58f, 3.54f, 1.0f));

    // A low north-side exterior frames the plaza without extending the route.
    const glm::mat4 identity(1.0f);
    AddBox({-0.9f, 1.31f, -14.88f}, {6.2f, 2.62f, 2.7f}, brickRed, identity, Surface::Siding);
    AddBox({-0.9f, 2.70f, -14.88f}, {6.45f, 0.18f, 2.97f}, warmWhite, identity, Surface::Paint);
    addWindow(identity, {-2.36f, 1.46f, -13.49f}, 2.02f, 1.12f, 0.0f);
    AddBox({0.72f, 1.20f, -13.50f}, {1.53f, 2.34f, 0.11f},
        {0.65f, 0.31f, 0.26f}, identity, Surface::Paint);
    traversal.AddObstacle({-0.9f, -14.88f}, {3.1f, 1.35f});
}

void SceneNuketown::BuildClockTower()
{
    const glm::vec3 steel(0.39f, 0.35f, 0.28f);
    const glm::vec3 clockFace(0.77f, 0.75f, 0.66f);
    const glm::vec3 hands(0.18f, 0.18f, 0.16f);
    constexpr float x = 2.6f;
    constexpr float z = -11.50f;
    traversal.AddObstacle({x, z}, {1.88f, 0.17f});

    for (float side : {-1.0f, 1.0f})
        AddBeam({x + side * 1.80f, 0.25f, z}, {x + side * 0.72f, 9.75f, z}, 0.17f, steel);

    const auto halfWidth = [](float y) { return 1.80f - (y - 0.25f) * 1.08f / 9.50f; };
    for (float baseY : {1.1f, 3.25f, 5.4f, 7.55f})
    {
        const float topY = baseY + 2.15f;
        AddBeam({x - halfWidth(baseY), baseY, z}, {x + halfWidth(baseY), baseY, z}, 0.11f, steel);
        AddBeam({x - halfWidth(baseY), baseY, z}, {x + halfWidth(topY), topY, z}, 0.09f, steel);
        AddBeam({x + halfWidth(baseY), baseY, z}, {x - halfWidth(topY), topY, z}, 0.09f, steel);
    }

    glm::mat4 dial = glm::translate(glm::mat4(1.0f), {x, 8.95f, z + 0.13f});
    dial = glm::rotate(dial, glm::radians(90.0f), {1.0f, 0.0f, 0.0f});
    AddPart(cylinder, {0.0f, 0.0f, 0.0f}, {1.29f, 0.28f, 1.29f}, steel, dial);
    AddPart(cylinder, {0.0f, 0.16f, 0.0f}, {1.15f, 0.08f, 1.15f}, clockFace, dial);
    for (int tick = 0; tick < 12; ++tick)
    {
        const float angle = glm::radians(30.0f * tick);
        const glm::vec3 direction(std::cos(angle), std::sin(angle), 0.0f);
        const glm::vec3 center(x, 8.95f, z + 0.35f);
        AddBeam(center + direction * 0.91f, center + direction * 1.04f, 0.058f, hands);
    }
    AddBeam({x, 8.95f, z + 0.39f}, {x - 0.36f, 9.32f, z + 0.39f}, 0.09f, hands);
    AddBeam({x, 8.95f, z + 0.39f}, {x + 0.19f, 9.67f, z + 0.39f}, 0.065f, hands);
}

void SceneNuketown::AddWheels(const glm::mat4& vehicle, float halfAxle, float radius, float frontZ, float rearZ)
{
    for (float side : {-1.0f, 1.0f})
    {
        for (float z : {frontZ, rearZ})
        {
            glm::mat4 axle = glm::translate(vehicle, {side * halfAxle, radius + 0.04f, z});
            axle = glm::rotate(axle, glm::radians(side * -90.0f), {0.0f, 0.0f, 1.0f});
            AddPart(cylinder, {0.0f, 0.0f, 0.0f}, {radius, 0.37f, radius}, tireRubber, axle);
            AddPart(cylinder, {0.0f, 0.20f, 0.0f}, {radius * 0.59f, 0.07f, radius * 0.59f},
                {0.51f, 0.47f, 0.37f}, axle);
        }
    }
}

void SceneNuketown::BuildBus()
{
    const glm::vec2 center(-1.55f, 3.90f);
    constexpr float heading = 12.0f;
    constexpr float busLength = 8.20f;
    const glm::mat4 vehicle = PlaceInMap({center.x, 0.0f, center.y}, heading);
    AddPart(cylinder, {0.0f, 0.051f, 0.0f}, {1.83f, 0.012f, 4.53f}, {0.19f, 0.21f, 0.21f}, vehicle);

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

void SceneNuketown::BuildTruck()
{
    const glm::mat4 vehicle = PlaceInMap({2.54f, 0.0f, -2.85f}, 13.0f);
    const glm::vec3 collisionCenter = glm::vec3(vehicle * glm::vec4(0.0f, 0.0f, -0.44f, 1.0f));
    traversal.AddObstacle({collisionCenter.x, collisionCenter.z}, {1.39f, 3.77f}, 13.0f);
    const glm::vec3 red(0.62f, 0.22f, 0.20f);
    const glm::vec3 silver(0.60f, 0.63f, 0.59f);
    const glm::vec3 bumper(0.34f, 0.36f, 0.34f);
    AddPart(cylinder, {0.0f, 0.051f, -0.15f}, {1.80f, 0.012f, 4.41f}, {0.19f, 0.21f, 0.21f}, vehicle);
    AddBox({0.0f, 0.73f, -0.4f}, {2.45f, 0.28f, 7.20f}, tireRubber, vehicle);
    AddBox({0.0f, 2.19f, -1.75f}, {2.72f, 2.65f, 4.72f}, silver, vehicle);
    AddBox({0.0f, 3.55f, -1.75f}, {2.78f, 0.17f, 4.78f}, {0.71f, 0.72f, 0.65f}, vehicle);
    for (float side : {-1.0f, 1.0f})
    {
        for (float y = 1.12f; y < 3.55f; y += 0.25f)
            AddBox({side * 1.375f, y, -1.75f}, {0.035f, 0.048f, 4.64f}, {0.55f, 0.57f, 0.53f}, vehicle);
    }

    AddBox({0.0f, 1.35f, 1.88f}, {2.47f, 1.32f, 2.51f}, red, vehicle);
    AddBox({0.0f, 2.19f, 1.57f}, {2.34f, 0.88f, 1.60f}, red, vehicle);
    AddBox({0.0f, 2.67f, 1.61f}, {2.46f, 0.19f, 1.78f}, warmWhite, vehicle);
    AddBox({0.0f, 2.20f, 2.40f}, {2.13f, 0.54f, 0.12f}, warmWhite, vehicle);
    AddBox({0.0f, 1.18f, 3.164f}, {1.15f, 0.92f, 0.13f}, bumper, vehicle);
    AddBox({0.0f, 1.16f, 3.246f}, {0.92f, 0.75f, 0.058f}, tireRubber, vehicle);
    AddBox({0.0f, 0.58f, 3.19f}, {2.63f, 0.21f, 0.26f}, bumper, vehicle);
    for (float x = -0.38f; x <= 0.39f; x += 0.12f)
        AddBox({x, 1.16f, 3.284f}, {0.038f, 0.73f, 0.042f}, bumper, vehicle);

    for (float side : {-1.0f, 1.0f})
    {
        AddBox({side * 0.51f, 2.20f, 2.478f}, {0.93f, 0.41f, 0.062f}, windowGlass, vehicle, Surface::Glass);
        AddBox({side * 1.184f, 2.20f, 1.68f}, {0.065f, 0.52f, 0.91f}, windowGlass, vehicle, Surface::Glass);
        AddPart(sphere, {side * 1.02f, 1.03f, 3.18f}, {0.19f, 0.19f, 0.10f}, warmWhite, vehicle);
    }
    AddWheels(vehicle, 1.16f, 0.56f, 2.14f, -2.80f);
}
