#include "SceneNuketown.h"

#include <algorithm>
#include <cmath>

namespace
{
    glm::mat4 PlaceGardenPart(glm::vec3 position, float yaw = 0.0f)
    {
        return glm::rotate(glm::translate(glm::mat4(1.0f), position),
            glm::radians(yaw), glm::vec3(0.0f, 1.0f, 0.0f));
    }
}

void SceneNuketown::BuildWestGarden()
{
    const glm::mat4 identity(1.0f);
    const glm::vec3 white(0.87f, 0.86f, 0.75f);
    const glm::vec3 wood(0.43f, 0.33f, 0.22f);
    const glm::vec3 concrete(0.60f, 0.58f, 0.52f);
    const glm::vec3 roof(0.35f, 0.39f, 0.36f);
    const glm::vec2 center(-27.1f, -1.0f);
    const glm::mat4 gazebo = PlaceGardenPart({center.x, 0.0f, center.y});

    AddBox({0.0f, 0.015f, 0.0f}, {2.45f, 0.07f, 2.85f}, concrete, gazebo, Surface::Concrete);
    AddPart(pyramid, {0.0f, 2.28f, 0.0f}, {2.85f, 0.68f, 3.22f}, roof, gazebo, Surface::Roof);
    AddBox({0.0f, 2.26f, 0.0f}, {2.75f, 0.13f, 3.12f}, white, gazebo, Surface::Paint);
    for (float x : {-1.08f, 1.08f})
    {
        for (float z : {-1.28f, 1.28f})
        {
            AddBox({x, 1.16f, z}, {0.12f, 2.27f, 0.12f}, white, gazebo, Surface::Paint);
            traversal.AddObstacle(center + glm::vec2(x, z), {0.075f, 0.075f});
        }
    }

    // Clipped diagonals form actual lattice panels; the east face remains open.
    const auto addLattice = [this, &white](glm::vec3 start, glm::vec3 end)
    {
        const glm::vec3 horizontal = end - start;
        const float width = glm::length(horizontal);
        const glm::vec3 direction = horizontal / width;
        constexpr float bottom = 0.18f;
        constexpr float height = 1.00f;
        for (int index = 0; index < 5; ++index)
        {
            const float t = static_cast<float>(index) / 4.0f;
            const float intercept = glm::mix(-height + 0.12f, width - 0.12f, t);
            const float lowX = std::max(0.0f, intercept);
            const float highX = std::min(width, intercept + height);
            const float lowY = lowX - intercept;
            const float highY = highX - intercept;
            const glm::vec3 low = start + direction * lowX;
            const glm::vec3 high = start + direction * highX;
            AddBeam(low + glm::vec3(0.0f, bottom + lowY, 0.0f),
                high + glm::vec3(0.0f, bottom + highY, 0.0f), 0.045f, white, Surface::Paint);
            AddBeam(low + glm::vec3(0.0f, bottom + height - lowY, 0.0f),
                high + glm::vec3(0.0f, bottom + height - highY, 0.0f), 0.045f, white, Surface::Paint);
        }
    };
    const glm::vec3 backStart(center.x - 1.08f, 0.0f, center.y - 1.28f);
    const glm::vec3 backEnd(center.x - 1.08f, 0.0f, center.y + 1.28f);
    addLattice(backStart, backEnd);
    addLattice(backStart, {center.x + 1.08f, 0.0f, center.y - 1.28f});
    addLattice(backEnd, {center.x + 1.08f, 0.0f, center.y + 1.28f});
    for (float y : {0.15f, 1.21f})
    {
        AddBox({-1.08f, y, 0.0f}, {0.075f, 0.09f, 2.64f}, white, gazebo, Surface::Paint);
        for (float z : {-1.28f, 1.28f})
            AddBox({0.0f, y, z}, {2.25f, 0.09f, 0.075f}, white, gazebo, Surface::Paint);
    }
    traversal.AddObstacle({center.x - 1.08f, center.y}, {0.055f, 1.28f});
    traversal.AddObstacle({center.x, center.y - 1.28f}, {1.08f, 0.055f});
    traversal.AddObstacle({center.x, center.y + 1.28f}, {1.08f, 0.055f});

    // The narrow flower bed stays along the fence rather than the spawn path.
    AddBox({-28.12f, 0.045f, 1.43f}, {0.70f, 0.09f, 2.08f},
        {0.36f, 0.28f, 0.18f}, identity, Surface::Dirt);
    for (float x : {-28.50f, -27.74f})
        AddBox({x, 0.09f, 1.43f}, {0.065f, 0.14f, 2.18f}, wood, identity, Surface::Wood);
    const glm::vec3 flowerColors[] = {
        {0.70f, 0.24f, 0.22f}, {0.55f, 0.36f, 0.67f}, {0.83f, 0.70f, 0.28f}
    };
    for (int index = 0; index < 7; ++index)
    {
        const float z = 0.60f + index * 0.28f;
        const float x = -28.12f + std::sin(index * 1.8f) * 0.12f;
        const float height = 0.37f + static_cast<float>(index % 3) * 0.065f;
        AddPart(cylinder, {x, 0.09f + height * 0.5f, z}, {0.016f, height, 0.016f},
            {0.23f, 0.34f, 0.16f});
        AddPart(sphere, {x, 0.22f, z}, {0.16f, 0.11f, 0.12f}, {0.27f, 0.38f, 0.17f});
        AddPart(sphere, {x, 0.09f + height, z}, {0.092f, 0.075f, 0.092f}, flowerColors[index % 3]);
    }
    for (int index = 0; index < 3; ++index)
        AddPart(sphere, {-26.96f + index * 0.45f, 0.26f, 1.15f},
            {0.38f, 0.31f + index * 0.03f, 0.35f}, {0.31f, 0.39f, 0.19f});
}

void SceneNuketown::BuildTestSiteFixtures()
{
    const glm::vec3 concrete(0.60f, 0.58f, 0.52f);
    // Tall test-site fixtures remain outside the playable fences.
    const glm::vec2 fixtures[] = {{-33.0f, -8.0f}, {34.0f, -5.0f}, {-30.0f, 15.0f}, {32.0f, 18.0f}};
    const glm::vec3 steel(0.56f, 0.57f, 0.51f);
    for (glm::vec2 position : fixtures)
    {
        const float yaw = glm::degrees(std::atan2(-position.x, -position.y));
        const glm::mat4 fixture = PlaceGardenPart({position.x, 0.0f, position.y}, yaw);
        AddBox({0.0f, 0.13f, 0.0f}, {0.65f, 0.26f, 0.65f}, concrete, fixture, Surface::Concrete);
        streetLightPositions.push_back({position.x, 10.15f, position.y});
        AddPart(cylinder, {0.0f, 5.20f, 0.0f}, {0.095f, 10.4f, 0.095f}, steel, fixture);
        AddBox({0.0f, 10.28f, 0.0f}, {2.02f, 0.12f, 0.14f}, steel, fixture);
        for (float side : {-1.0f, 1.0f})
        {
            AddBox({side * 0.64f, 10.34f, 0.12f}, {0.61f, 0.38f, 0.36f}, steel, fixture);
            AddBox({side * 0.64f, 10.34f, 0.315f}, {0.49f, 0.27f, 0.045f},
                {1.0f, 0.91f, 0.70f}, fixture, Surface::Emissive);
        }
    }
}
