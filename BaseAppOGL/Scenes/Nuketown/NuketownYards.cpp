#include "SceneNuketown.h"

#include <cmath>

namespace
{
    glm::mat4 PlaceYardPart(glm::vec3 position, float yaw = 0.0f)
    {
        return glm::rotate(glm::translate(glm::mat4(1.0f), position),
            glm::radians(yaw), glm::vec3(0.0f, 1.0f, 0.0f));
    }
}

void SceneNuketown::BuildEastYard()
{
    const glm::mat4 identity(1.0f);
    const glm::vec3 lawn(0.38f, 0.46f, 0.26f);
    const glm::vec3 paving(0.60f, 0.58f, 0.52f);
    const glm::vec3 trim(0.87f, 0.86f, 0.75f);
    const glm::vec3 timber(0.46f, 0.35f, 0.23f);
    const glm::vec3 yellow(0.73f, 0.64f, 0.36f);
    const glm::vec3 roof(0.31f, 0.35f, 0.33f);

    // Lawn patches follow the lot, leaving the outer fence and the road visible.
    AddPath({24.2f, -5.9f}, {24.2f, 6.7f}, 10.3f, -0.003f, lawn, Surface::Grass);
    AddPath({21.4f, 8.45f}, {25.9f, 8.45f}, 5.9f, -0.003f, lawn, Surface::Grass);
    AddPath({9.5f, 7.25f}, {22.5f, 7.25f}, 6.1f, -0.003f, lawn, Surface::Grass);
    AddPath({10.0f, -12.2f}, {22.0f, -11.7f}, 4.5f, -0.003f, lawn, Surface::Grass);
    AddPath({7.25f, -9.1f}, {7.25f, 5.85f}, 2.4f, -0.003f, lawn, Surface::Grass);

    // Both lateral routes stay on the ground while the rear deck belongs to the house.
    AddPath({4.9f, -6.9f}, {10.15f, -7.6f}, 3.55f, 0.027f, paving, Surface::Concrete);
    AddPath({7.1f, 1.5f}, {7.1f, 5.7f}, 1.45f, 0.028f, paving, Surface::Concrete);
    AddPath({7.1f, 5.7f}, {10.5f, 8.1f}, 1.45f, 0.028f, paving, Surface::Concrete);
    AddPath({10.5f, 8.1f}, {19.7f, 8.5f}, 1.45f, 0.028f, paving, Surface::Concrete);
    AddPath({19.7f, 8.5f}, {23.5f, 6.0f}, 1.45f, 0.028f, paving, Surface::Concrete);
    AddPath({23.5f, 6.0f}, {25.0f, 3.0f}, 1.45f, 0.028f, paving, Surface::Concrete);
    AddPath({22.4f, 2.0f}, {25.0f, 3.0f}, 1.40f, 0.029f, paving, Surface::Concrete);
    AddPath({13.0f, -11.0f}, {20.6f, -10.4f}, 1.45f, 0.028f, paving, Surface::Concrete);
    AddPath({20.6f, -10.4f}, {24.0f, -6.0f}, 1.45f, 0.028f, paving, Surface::Concrete);
    AddPath({24.0f, -6.0f}, {26.0f, 1.0f}, 1.45f, 0.028f, paving, Surface::Concrete);

    // The small yellow shed and white crossed bracing are visible in the reference yard.
    const glm::vec2 shedCenter(28.05f, -3.60f);
    constexpr float shedYaw = -20.0f;
    const glm::mat4 shed = PlaceYardPart({shedCenter.x, 0.0f, shedCenter.y}, shedYaw);
    AddBox({0.0f, 1.04f, 0.0f}, {2.25f, 2.08f, 3.10f}, yellow, shed, Surface::Siding);
    AddPart(pyramid, {0.0f, 2.07f, 0.0f}, {2.67f, 0.62f, 3.48f}, roof, shed, Surface::Roof);
    AddBox({0.0f, 2.07f, 0.0f}, {2.55f, 0.10f, 3.35f}, trim, shed, Surface::Paint);
    for (float side : {-1.0f, 1.0f})
    {
        AddBox({side * 1.13f, 1.05f, 1.56f}, {0.105f, 2.12f, 0.09f}, trim, shed, Surface::Paint);
        AddBox({side * 1.13f, 1.05f, -1.56f}, {0.105f, 2.12f, 0.09f}, trim, shed, Surface::Paint);
        AddBox({side * 1.16f, 0.20f, 0.0f}, {0.09f, 0.12f, 3.15f}, trim, shed, Surface::Paint);
        AddBox({side * 1.16f, 1.90f, 0.0f}, {0.09f, 0.12f, 3.15f}, trim, shed, Surface::Paint);
        const auto brace = [&shed](float sideValue, glm::vec3 point)
        {
            point.x = sideValue * 1.18f;
            return glm::vec3(shed * glm::vec4(point, 1.0f));
        };
        AddBeam(brace(side, {0.0f, 0.22f, -1.49f}), brace(side, {0.0f, 1.86f, 1.49f}),
            0.105f, trim, Surface::Paint);
        AddBeam(brace(side, {0.0f, 1.86f, -1.49f}), brace(side, {0.0f, 0.22f, 1.49f}),
            0.105f, trim, Surface::Paint);
    }
    AddBox({0.0f, 0.98f, 1.585f}, {1.00f, 1.85f, 0.065f}, timber, shed, Surface::Wood);
    for (float side : {-1.0f, 1.0f})
        AddBox({side * 0.56f, 0.99f, 1.63f}, {0.11f, 1.98f, 0.11f}, trim, shed, Surface::Paint);
    AddBox({0.0f, 1.96f, 1.63f}, {1.23f, 0.10f, 0.11f}, trim, shed, Surface::Paint);
    AddPart(sphere, {0.34f, 0.92f, 1.67f}, {0.035f, 0.035f, 0.035f},
        {0.57f, 0.53f, 0.40f}, shed);
    AddBox({0.0f, 0.055f, 1.94f}, {1.40f, 0.11f, 0.63f}, paving, shed, Surface::Concrete);
    traversal.AddObstacle(shedCenter, {1.15f, 1.58f}, shedYaw);

    // Low garden beds sit beside the back fence, away from the circulation paths.
    AddBox({29.45f, 0.055f, 1.95f}, {0.76f, 0.11f, 4.75f},
        {0.36f, 0.28f, 0.18f}, identity, Surface::Dirt);
    for (float x : {29.04f, 29.86f})
        AddBox({x, 0.11f, 1.95f}, {0.075f, 0.17f, 4.88f}, timber, identity, Surface::Wood);
    for (float z : {-0.45f, 4.35f})
        AddBox({29.45f, 0.11f, z}, {0.90f, 0.17f, 0.075f}, timber, identity, Surface::Wood);
    const glm::vec3 flowers[] = {
        {0.69f, 0.23f, 0.19f}, {0.78f, 0.67f, 0.28f},
        {0.52f, 0.35f, 0.63f}, {0.81f, 0.74f, 0.65f}
    };
    for (int index = 0; index < 11; ++index)
    {
        const float z = -0.12f + index * 0.405f;
        const float x = 29.44f + std::sin(index * 2.1f) * 0.15f;
        const float height = 0.36f + static_cast<float>(index % 3) * 0.065f;
        AddPart(cylinder, {x, 0.14f + height * 0.5f, z}, {0.017f, height, 0.017f},
            {0.24f, 0.34f, 0.17f});
        AddPart(sphere, {x, 0.27f, z}, {0.16f, 0.10f, 0.13f}, {0.28f, 0.39f, 0.18f});
        AddPart(sphere, {x, height + 0.14f, z}, {0.095f, 0.075f, 0.095f}, flowers[index % 4]);
    }
    traversal.AddObstacle({29.45f, 1.95f}, {0.45f, 2.45f});

    // A brick barbecue stands outside the diagonal route between the deck and the lawn.
    const glm::vec2 barbecueCenter(24.5f, 8.20f);
    const glm::mat4 barbecue = PlaceYardPart({barbecueCenter.x, 0.0f, barbecueCenter.y});
    AddBox({0.0f, 0.045f, 0.0f}, {1.92f, 0.09f, 1.48f}, paving, barbecue, Surface::Concrete);
    const glm::vec3 brick(0.29f, 0.24f, 0.20f);
    const glm::vec3 mortar(0.37f, 0.35f, 0.29f);
    for (float x : {-0.61f, 0.61f})
    {
        AddBox({x, 0.57f, 0.0f}, {0.30f, 1.04f, 1.05f}, mortar, barbecue);
        for (int row = 0; row < 6; ++row)
            for (int block = 0; block < 3; ++block)
                AddBox({x, 0.18f + row * 0.16f, -0.345f + block * 0.345f},
                    {0.32f, 0.14f, 0.32f}, brick * (0.96f + block * 0.035f), barbecue);
    }
    AddBox({0.0f, 0.70f, 0.0f}, {1.26f, 0.16f, 1.03f}, mortar, barbecue);
    AddBox({0.0f, 0.81f, 0.0f}, {1.02f, 0.08f, 0.82f}, {0.09f, 0.09f, 0.08f}, barbecue);
    AddBox({0.0f, 0.94f, 0.49f}, {1.22f, 0.28f, 0.12f}, brick, barbecue);
    for (float x = -0.47f; x < 0.50f; x += 0.105f)
        AddBox({x, 0.92f, -0.02f}, {0.018f, 0.018f, 0.86f}, {0.35f, 0.36f, 0.32f}, barbecue);
    for (int coal = 0; coal < 9; ++coal)
        AddPart(sphere, {-0.36f + (coal % 3) * 0.34f, 0.86f, -0.27f + (coal / 3) * 0.25f},
            {0.12f, 0.07f, 0.095f}, {0.065f, 0.065f, 0.058f}, barbecue);
    traversal.AddObstacle(barbecueCenter, {0.80f, 0.58f});

    // Two posts and four parallel lines hold a few pale pieces of laundry.
    const glm::vec3 clothesPole(0.69f, 0.70f, 0.63f);
    for (float z : {2.3f, 6.7f})
    {
        AddPart(cylinder, {27.0f, 1.13f, z}, {0.045f, 2.26f, 0.045f}, clothesPole);
        AddBox({27.0f, 2.24f, z}, {1.62f, 0.055f, 0.055f}, clothesPole);
        traversal.AddObstacle({27.0f, z}, {0.06f, 0.06f});
    }
    for (float x : {26.4f, 26.8f, 27.2f, 27.6f})
        AddBeam({x, 2.25f, 2.3f}, {x, 2.25f, 6.7f}, 0.010f, {0.41f, 0.41f, 0.35f});
    AddBox({26.8f, 1.73f, 3.46f}, {0.018f, 1.03f, 0.88f},
        {0.82f, 0.80f, 0.69f}, identity, Surface::Paint);
    AddBox({27.2f, 1.80f, 5.32f}, {0.018f, 0.90f, 0.65f},
        {0.65f, 0.73f, 0.69f}, identity, Surface::Paint);
    AddBox({26.4f, 1.87f, 4.60f}, {0.018f, 0.74f, 0.45f},
        {0.85f, 0.83f, 0.74f}, identity, Surface::Paint);
    for (float side : {-1.0f, 1.0f})
        AddBox({26.4f, 2.08f, 4.60f + side * 0.30f}, {0.018f, 0.24f, 0.20f},
            {0.85f, 0.83f, 0.74f}, identity, Surface::Paint);
}
