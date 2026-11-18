#include "SceneNuketown.h"

#include <cmath>
#include <iterator>

void SceneNuketown::BuildPerimeter()
{
    const glm::vec3 wood(0.52f, 0.40f, 0.28f);
    const glm::vec3 warmWhite(0.87f, 0.86f, 0.75f);
    const glm::vec2 outline[] = {
        {-28.75f, 2.7f}, {-29.75f, -10.5f}, {-24.1f, -17.2f}, {-17.2f, -17.2f},
        {-10.45f, -14.7f}, {-4.85f, -16.8f}, {6.0f, -18.0f}, {21.3f, -15.6f},
        {30.4f, -7.5f}, {31.1f, 5.5f}, {27.0f, 12.4f}, {19.1f, 13.7f},
        {12.8f, 11.4f}, {8.5f, 14.0f}, {6.15f, 19.5f}, {-5.85f, 19.5f},
        {-5.85f, 12.1f}, {-10.35f, 8.25f}, {-19.85f, 8.25f}
    };
    for (size_t index = 0; index < std::size(outline); ++index)
    {
        const glm::vec2 start = outline[index];
        const glm::vec2 end = outline[(index + 1) % std::size(outline)];
        AddFence(start, end, 1.15f, wood);
    }
    // Carry the road up to its closed fence without continuing beyond the map.
    AddPath({0.45f, 19.1f}, {0.45f, 19.47f}, 10.0f, 0.017f,
        {0.60f, 0.58f, 0.52f}, Surface::Concrete);
    AddPath({0.45f, 19.1f}, {0.45f, 19.47f}, 8.20f, 0.049f,
        {0.27f, 0.29f, 0.29f}, Surface::Asphalt);
    AddFence({-10.2f, 6.36f}, {-8.3f, 6.7f}, 1.03f, warmWhite);
    AddFence({-7.1f, 7.0f}, {-6.6f, 9.02f}, 1.03f, warmWhite);
    AddFence({8.75f, 6.3f}, {12.2f, 7.0f}, 0.92f, warmWhite);
}

void SceneNuketown::BuildClockTower()
{
    const glm::vec3 steel(0.39f, 0.35f, 0.28f);
    const glm::vec3 clockFace(0.77f, 0.75f, 0.66f);
    const glm::vec3 hands(0.18f, 0.18f, 0.16f);
    constexpr float x = -1.8f;
    constexpr float z = -20.3f;
    traversal.AddObstacle({x, z}, {1.88f, 0.17f});

    for (float side : {-1.0f, 1.0f})
    {
        AddBox({x + side * 1.80f, 0.08f, z}, {0.48f, 0.24f, 0.50f},
            {0.54f, 0.52f, 0.45f}, glm::mat4(1.0f), Surface::Concrete);
        AddBeam({x + side * 1.80f, 0.25f, z}, {x + side * 0.72f, 9.75f, z}, 0.17f, steel);
    }

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
