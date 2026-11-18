#include "SceneNuketown.h"

#include <cmath>

namespace
{
    const glm::vec3 houseTrim(0.91f, 0.90f, 0.83f);
    const glm::vec3 houseGlass(0.22f, 0.35f, 0.37f);
    const glm::vec3 houseConcrete(0.60f, 0.58f, 0.52f);

    glm::mat4 PlaceHouse(glm::vec2 center, float yaw)
    {
        return glm::rotate(glm::translate(glm::mat4(1.0f), {center.x, 0.0f, center.y}),
            glm::radians(yaw), glm::vec3(0.0f, 1.0f, 0.0f));
    }
}

void SceneNuketown::BuildHouses()
{
    BuildHouse({-14.1f, -6.4f}, 68.0f, false);
    BuildHouse({14.0f, 0.0f}, -112.0f, true);
}

void SceneNuketown::BuildHouse(glm::vec2 center, float yaw, bool yellow)
{
    const glm::mat4 house = PlaceHouse(center, yaw);
    const glm::vec3 lower = yellow ? glm::vec3(0.76f, 0.68f, 0.34f) : glm::vec3(0.26f, 0.60f, 0.47f);
    const glm::vec3 upper = yellow ? glm::vec3(0.79f, 0.74f, 0.48f) : glm::vec3(0.67f, 0.75f, 0.72f);
    const glm::vec3 roof(0.32f, 0.37f, 0.36f);
    const glm::vec3 doorWood(0.57f, 0.47f, 0.33f);
    const glm::vec3 recess(0.14f, 0.19f, 0.18f);
    const Surface wall = yellow ? Surface::VerticalSiding : Surface::Siding;
    traversal.AddObstacle(center, {3.75f, 4.60f}, yaw);

    const auto addWindow = [this, &recess](const glm::mat4& parent, glm::vec3 position,
        float width, float height, float windowYaw)
    {
        const glm::mat4 frame = glm::rotate(glm::translate(parent, position),
            glm::radians(windowYaw), glm::vec3(0.0f, 1.0f, 0.0f));
        // A dark reveal sits behind the glass; slender bars cast a visible frame.
        AddBox({0.0f, 0.0f, 0.0f}, {width + 0.24f, height + 0.20f, 0.045f}, recess, frame);
        AddBox({0.0f, 0.0f, 0.042f}, {width, height, 0.018f}, houseGlass, frame, Surface::Glass);
        for (float side : {-1.0f, 1.0f})
        {
            AddBox({side * (width * 0.5f + 0.05f), 0.0f, 0.082f},
                {0.10f, height + 0.16f, 0.115f}, houseTrim, frame, Surface::Paint);
            AddBox({0.0f, side * (height * 0.5f + 0.04f), 0.082f},
                {width + 0.20f, 0.08f, 0.115f}, houseTrim, frame, Surface::Paint);
        }
        AddBox({0.0f, 0.0f, 0.088f}, {0.040f, height, 0.075f}, houseTrim, frame, Surface::Paint);
        AddBox({0.0f, -0.055f, 0.088f}, {width, 0.032f, 0.075f}, houseTrim, frame, Surface::Paint);
        AddBox({0.0f, -height * 0.5f - 0.115f, 0.095f},
            {width + 0.34f, 0.075f, 0.28f}, houseTrim * 0.97f, frame, Surface::Paint);
        AddBox({0.0f, height * 0.5f + 0.118f, 0.070f},
            {width + 0.30f, 0.035f, 0.18f}, houseTrim, frame, Surface::Paint);
    };
    const auto addDoor = [this, &recess, &doorWood](const glm::mat4& parent,
        glm::vec3 position, float doorYaw)
    {
        const glm::mat4 door = glm::rotate(glm::translate(parent, position),
            glm::radians(doorYaw), glm::vec3(0.0f, 1.0f, 0.0f));
        AddBox({0.0f, 0.0f, 0.0f}, {1.13f, 2.19f, 0.09f}, recess, door);
        AddBox({0.0f, 0.0f, 0.05f}, {0.97f, 2.07f, 0.065f}, doorWood, door, Surface::Wood);
        for (float edge : {-1.0f, 1.0f})
            AddBox({edge * 0.535f, 0.0f, 0.055f}, {0.10f, 2.19f, 0.15f},
                houseTrim, door, Surface::Paint);
        AddBox({0.0f, 1.09f, 0.055f}, {1.17f, 0.11f, 0.15f}, houseTrim, door, Surface::Paint);
        for (float y : {-0.47f, 0.43f})
            AddBox({0.0f, y, 0.092f}, {0.69f, 0.68f, 0.025f}, doorWood * 0.88f, door, Surface::Wood);
        AddBox({0.35f, -0.03f, 0.105f}, {0.07f, 0.16f, 0.022f}, {0.48f, 0.43f, 0.29f}, door);
        AddPart(sphere, {0.35f, -0.03f, 0.14f}, {0.038f, 0.038f, 0.038f},
            {0.63f, 0.57f, 0.38f}, door);
    };
    const auto world = [&house](glm::vec3 local)
    {
        return glm::vec3(house * glm::vec4(local, 1.0f));
    };

    AddBox({0.0f, 1.30f, 0.0f}, {7.50f, 2.60f, 9.20f}, lower, house, wall);
    AddBox({0.0f, 3.84f, 0.0f}, {7.50f, 2.48f, 9.20f}, upper, house, wall);
    if (yellow)
        AddBox({0.0f, 0.30f, 0.0f}, {7.58f, 0.60f, 9.28f}, {0.59f, 0.54f, 0.40f}, house, Surface::Stone);
    else
        AddBox({0.0f, 0.08f, 0.0f}, {7.54f, 0.16f, 9.24f}, houseTrim, house, Surface::Paint);
    AddBox({0.0f, 2.63f, 0.0f}, {7.60f, 0.23f, 9.29f}, houseTrim, house, Surface::Paint);

    // Two sloping panels share a ridge instead of a pyramid-shaped house roof.
    const float roofAngle = std::atan2(1.22f, 4.10f);
    const float roofLength = std::sqrt(4.10f * 4.10f + 1.22f * 1.22f);
    for (float side : {-1.0f, 1.0f})
    {
        const glm::mat4 panel = glm::rotate(glm::translate(house, {side * 2.05f, 5.63f, 0.0f}),
            -side * roofAngle, glm::vec3(0.0f, 0.0f, 1.0f));
        AddBox({0.0f, 0.0f, 0.0f}, {roofLength, 0.13f, 9.90f}, roof, panel, Surface::Roof);
        AddPart(pyramid, {0.0f, 5.02f, side * 4.61f}, {7.50f, 1.18f, 0.08f}, upper, house, wall);
        for (float edge : {-1.0f, 1.0f})
            AddBeam(world({0.0f, 6.24f, side * 4.99f}), world({edge * 4.10f, 5.02f, side * 4.99f}),
                0.10f, houseTrim, Surface::Paint);
        AddBox({side * 4.10f, 5.02f, 0.0f}, {0.11f, 0.12f, 9.98f}, houseTrim, house, Surface::Paint);
    }
    AddBox({0.0f, 6.27f, 0.0f}, {0.12f, 0.10f, 9.98f}, roof, house, Surface::Roof);

    for (float side : {-1.0f, 1.0f})
    {
        const float windowYaw = side > 0.0f ? 0.0f : 180.0f;
        for (float x : {-1.95f, 1.90f})
            addWindow(house, {x, 3.92f, side * 4.68f}, 1.78f, 0.84f, windowYaw);
        addWindow(house, {1.46f, 1.38f, side * 4.68f}, 2.09f, 0.91f, windowYaw);
        addWindow(house, {side * 3.80f, 3.71f, -1.80f}, 1.48f, 0.82f, side * 90.0f);
        addDoor(house, {-1.26f, 1.13f, side * 4.68f}, windowYaw);
        for (float x : {-3.77f, 3.77f})
            AddBox({x, 2.55f, side * 4.61f}, {0.12f, 4.86f, 0.10f}, houseTrim, house, Surface::Paint);
    }
    addDoor(house, {0.0f, 3.685f, -4.68f}, 180.0f);
    if (yellow)
        addDoor(house, {0.0f, 3.685f, 4.68f}, 0.0f);
    AddBox({-1.35f, 0.09f, -4.96f}, {1.68f, 0.18f, 0.59f}, houseConcrete, house, Surface::Concrete);
    AddBox({-1.26f, 0.09f, 4.96f}, {1.68f, 0.18f, 0.59f}, houseConcrete, house, Surface::Concrete);

    const glm::vec3 garageCenter = glm::vec3(house * glm::vec4(-5.62f, 0.0f, 1.78f, 1.0f));
    const glm::mat4 garage = PlaceHouse({garageCenter.x, garageCenter.z}, yaw);
    const auto garagePoint = [&garage](glm::vec3 point)
    {
        return glm::vec3(garage * glm::vec4(point, 1.0f));
    };
    const auto garageSolid = [this, yaw, &garage, &garagePoint](glm::vec3 position,
        glm::vec3 size, glm::vec3 color, Surface surface)
    {
        AddBox(position, size, color, garage, surface);
        const glm::vec3 point = garagePoint(position);
        traversal.AddObstacle({point.x, point.z}, {size.x * 0.5f, size.z * 0.5f}, yaw,
            position.y - size.y * 0.5f, position.y + size.y * 0.5f);
    };
    const glm::vec3 interior(0.69f, 0.68f, 0.60f);
    const glm::vec3 metal(0.43f, 0.48f, 0.46f);

    // A thin shell replaces the solid block. The central aisle joins two real openings.
    // The attached house overlaps 0.36 m of the original footprint on the right;
    // its lining therefore stops before that solid house wall.
    garageSolid({-2.13f, 1.39f, 0.0f}, {0.20f, 2.78f, 6.48f}, upper, wall);
    garageSolid({1.82f, 1.39f, 0.0f}, {0.20f, 2.78f, 6.48f}, upper, wall);
    AddBox({-2.024f, 1.35f, 0.0f}, {0.012f, 2.66f, 6.20f}, interior, garage, Surface::Paint);
    AddBox({1.714f, 1.35f, 0.0f}, {0.012f, 2.66f, 6.20f}, interior, garage, Surface::Paint);
    AddBox({-0.155f, 0.002f, 0.0f}, {4.35f, 0.040f, 6.48f},
        houseConcrete * 0.83f, garage, Surface::Concrete);
    garageSolid({-0.155f, 2.72f, 0.0f}, {4.35f, 0.12f, 6.48f}, interior, Surface::Paint);
    AddPart(pyramid, {0.0f, 2.78f, 0.0f}, {4.96f, 0.95f, 6.94f}, roof, garage, Surface::Roof);
    AddBox({0.0f, 2.78f, 0.0f}, {4.79f, 0.11f, 6.75f}, houseTrim, garage, Surface::Paint);

    // The main entrance has a 3.70 m clear opening and a 2.32 m lintel.
    for (float side : {-1.0f, 1.0f})
    {
        garageSolid({side * 2.04f, 1.16f, 3.24f}, {0.38f, 2.32f, 0.20f}, upper, wall);
        AddBox({side * 1.93f, 1.16f, 3.365f}, {0.16f, 2.32f, 0.14f},
            houseTrim, garage, Surface::Paint);
        AddBox({side * 1.97f, 1.34f, 3.10f}, {0.045f, 2.26f, 0.045f}, metal, garage, Surface::Chrome);
        AddBox({side * 1.97f, 2.50f, 1.39f}, {0.045f, 0.045f, 3.48f}, metal, garage, Surface::Chrome);
    }
    garageSolid({0.0f, 2.55f, 3.24f}, {4.46f, 0.46f, 0.20f}, upper, wall);
    AddBox({0.0f, 2.36f, 3.365f}, {4.02f, 0.14f, 0.14f}, houseTrim, garage, Surface::Paint);

    // A raised sectional gate rests beneath its tracks, safely above the player's head.
    garageSolid({0.0f, 2.52f, 1.90f}, {3.70f, 0.10f, 2.55f},
        yellow ? glm::vec3(0.62f, 0.59f, 0.44f) : glm::vec3(0.39f, 0.44f, 0.39f), Surface::Paint);
    for (float z = 0.80f; z < 3.16f; z += 0.36f)
        AddBox({0.0f, 2.462f, z}, {3.65f, 0.018f, 0.025f}, metal, garage, Surface::Chrome);

    // A wide back doorway opens toward the yard, without a threshold collider.
    constexpr float rearHalfOpening = 0.82f;
    for (float side : {-1.0f, 1.0f})
    {
        garageSolid({side * 1.525f, 1.10f, -3.24f}, {1.41f, 2.20f, 0.20f}, upper, wall);
        AddBox({side * 1.525f, 1.10f, -3.128f}, {1.41f, 2.20f, 0.018f}, interior, garage, Surface::Paint);
        AddBox({side * (rearHalfOpening + 0.05f), 1.10f, -3.36f}, {0.10f, 2.20f, 0.15f},
            houseTrim, garage, Surface::Paint);
    }
    garageSolid({0.0f, 2.49f, -3.24f}, {4.46f, 0.58f, 0.20f}, upper, wall);
    AddBox({0.0f, 2.25f, -3.36f}, {1.94f, 0.10f, 0.15f}, houseTrim, garage, Surface::Paint);
    const glm::mat4 openDoor = glm::rotate(glm::translate(garage, {-0.89f, 0.0f, -3.36f}),
        glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    AddBox({0.79f, 1.10f, 0.0f}, {1.58f, 2.14f, 0.075f}, doorWood, openDoor, Surface::Wood);
    for (float y : {0.61f, 1.57f})
        AddBox({0.79f, y, 0.044f}, {1.30f, 0.70f, 0.018f}, doorWood * 0.86f, openDoor, Surface::Wood);
    AddPart(sphere, {1.40f, 1.08f, 0.086f}, {0.040f, 0.040f, 0.040f},
        {0.63f, 0.57f, 0.38f}, openDoor, Surface::Chrome);
    const glm::vec3 openDoorCenter = garagePoint({-0.89f, 1.10f, -4.15f});
    traversal.AddObstacle({openDoorCenter.x, openDoorCenter.z}, {0.79f, 0.045f},
        yaw + 90.0f, 0.03f, 2.17f);
    AddBox({0.0f, 0.007f, -3.70f}, {1.92f, 0.030f, 0.92f},
        houseConcrete, garage, Surface::Concrete);

    if (yellow)
    {
        const glm::vec3 stone(0.59f, 0.54f, 0.40f);
        AddBox({-2.245f, 0.30f, 0.0f}, {0.04f, 0.60f, 6.48f}, stone, garage, Surface::Stone);
        for (float side : {-1.0f, 1.0f})
        {
            AddBox({side * 2.04f, 0.30f, 3.355f}, {0.38f, 0.60f, 0.04f}, stone, garage, Surface::Stone);
            AddBox({side * 1.525f, 0.30f, -3.355f}, {1.41f, 0.60f, 0.04f}, stone, garage, Surface::Stone);
        }
    }

    // Crates stay against the walls; the aisle at local X=0 remains unobstructed.
    const auto crate = [this, &garage, &garageSolid](glm::vec3 position, glm::vec3 size)
    {
        const glm::vec3 wood(0.57f, 0.42f, 0.25f);
        garageSolid(position, size, wood, Surface::Wood);
        for (float side : {-1.0f, 1.0f})
        {
            AddBox(position + glm::vec3(side * (size.x * 0.5f - 0.045f), 0.0f, size.z * 0.5f + 0.012f),
                {0.075f, size.y, 0.025f}, wood * 0.78f, garage, Surface::Wood);
            AddBox(position + glm::vec3(0.0f, side * (size.y * 0.5f - 0.045f), size.z * 0.5f + 0.018f),
                {size.x, 0.075f, 0.025f}, wood * 0.78f, garage, Surface::Wood);
        }
    };
    crate({-1.48f, 0.37f, -1.96f}, {0.84f, 0.70f, 0.88f});
    crate({-1.48f, 0.96f, -1.96f}, {0.64f, 0.48f, 0.66f});
    crate({1.18f, 0.40f, -1.99f}, {0.78f, 0.76f, 0.86f});
    crate({-1.46f, 0.29f, 0.39f}, {0.76f, 0.54f, 0.72f});
    garageSolid({1.30f, 0.64f, 0.37f}, {0.58f, 1.20f, 1.45f},
        {0.60f, 0.46f, 0.28f}, Surface::Wood);
    AddBox({1.30f, 1.27f, 0.37f}, {0.67f, 0.075f, 1.54f}, metal, garage, Surface::Paint);
    AddBox({1.26f, 1.48f, 0.43f}, {0.38f, 0.34f, 0.47f},
        {0.63f, 0.52f, 0.35f}, garage, Surface::Paint);
    AddBox({1.26f, 1.48f, 0.43f}, {0.070f, 0.35f, 0.48f},
        {0.81f, 0.75f, 0.59f}, garage, Surface::Paint);

    AddBox({-0.16f, 2.61f, -0.15f}, {1.16f, 0.08f, 0.32f}, metal, garage, Surface::Paint);
    AddBox({-0.16f, 2.558f, -0.15f}, {1.05f, 0.028f, 0.22f},
        {1.0f, 0.85f, 0.60f}, garage, Surface::Emissive);
    AddBox({0.0f, 2.52f, 3.37f}, {0.34f, 0.14f, 0.12f},
        {1.0f, 0.84f, 0.53f}, garage, Surface::Emissive);
    AddBox({0.0f, 0.017f, 4.53f}, {3.82f, 0.045f, 2.72f}, houseConcrete, garage, Surface::Concrete);
    const glm::vec3 lamp = garagePoint({-0.16f, 2.48f, -0.15f});
    if (yellow)
        secondGarageLampPosition = lamp;
    else
        garageLampPosition = lamp;

    BuildPorch(house, yellow);
}

void SceneNuketown::BuildPorch(const glm::mat4& house, bool yellow)
{
    constexpr float deckHeight = 2.65f;
    constexpr float railTop = 3.65f;
    constexpr float stairWidth = 1.22f;
    constexpr float stairStart = -11.1f;
    constexpr float stairEnd = -7.0f;
    const float stairX = yellow ? -2.65f : 2.65f;
    const float yaw = glm::degrees(std::atan2(house[2].x, house[2].z));
    const auto world = [&house](glm::vec3 local)
    {
        return glm::vec3(house * glm::vec4(local, 1.0f));
    };
    const auto pointXZ = [&world](glm::vec3 local)
    {
        const glm::vec3 point = world(local);
        return glm::vec2(point.x, point.z);
    };
    const auto obstacle = [this, yaw, &pointXZ](glm::vec3 center, glm::vec2 half,
        float bottom, float top)
    {
        traversal.AddObstacle(pointXZ(center), half, yaw, bottom, top);
    };
    const auto rail = [this, &house, &obstacle](float x0, float z0, float x1, float z1,
        float base, float top)
    {
        const bool alongX = std::abs(x1 - x0) > std::abs(z1 - z0);
        const float length = alongX ? std::abs(x1 - x0) : std::abs(z1 - z0);
        if (length < 0.04f)
            return;
        const glm::vec3 center((x0 + x1) * 0.5f, 0.0f, (z0 + z1) * 0.5f);
        AddBox({center.x, top - 0.045f, center.z},
            alongX ? glm::vec3(length, 0.09f, 0.10f) : glm::vec3(0.10f, 0.09f, length),
            houseTrim, house, Surface::Paint);
        AddBox({center.x, base + 0.19f, center.z},
            alongX ? glm::vec3(length, 0.065f, 0.075f) : glm::vec3(0.075f, 0.065f, length),
            houseTrim, house, Surface::Paint);
        const int count = 1 + static_cast<int>(length / 0.34f);
        for (int index = 0; index <= count; ++index)
        {
            const float t = static_cast<float>(index) / count;
            AddBox({glm::mix(x0, x1, t), (base + top) * 0.5f, glm::mix(z0, z1, t)},
                {0.047f, top - base - 0.08f, 0.047f}, houseTrim, house, Surface::Paint);
        }
        obstacle(center, alongX ? glm::vec2(length * 0.5f, 0.055f) : glm::vec2(0.055f, length * 0.5f), base, top + 0.05f);
    };

    // Deck edges and the ramp meet exactly at local Z=-7; the underside stays open.
    AddBox({0.0f, 2.59f, -5.80f}, {7.20f, 0.12f, 2.40f},
        houseTrim * 0.86f, house, Surface::Wood);
    traversal.AddPlatform(pointXZ({0.0f, 0.0f, -5.80f}), {3.60f, 1.20f}, yaw, deckHeight);
    for (float x : {-3.40f, 0.0f, 3.40f})
    {
        AddBox({x, 1.325f, -6.84f}, {0.12f, deckHeight, 0.12f}, houseTrim, house, Surface::Paint);
        obstacle({x, 0.0f, -6.84f}, {0.06f, 0.06f}, 0.0f, deckHeight);
    }
    for (float x : {-3.40f, 3.40f})
    {
        AddBox({x, 1.325f, -4.86f}, {0.12f, deckHeight, 0.12f}, houseTrim, house, Surface::Paint);
        obstacle({x, 0.0f, -4.86f}, {0.06f, 0.06f}, 0.0f, deckHeight);
    }
    rail(-3.60f, -7.0f, stairX - 0.70f, -7.0f, deckHeight, railTop);
    rail(stairX + 0.70f, -7.0f, 3.60f, -7.0f, deckHeight, railTop);
    rail(-3.60f, -7.0f, -3.60f, -4.64f, deckHeight, railTop);
    rail(3.60f, -7.0f, 3.60f, -4.64f, deckHeight, railTop);

    traversal.AddRamp(pointXZ({stairX, 0.0f, stairStart}), pointXZ({stairX, 0.0f, stairEnd}),
        stairWidth, 0.0f, deckHeight);
    constexpr int steps = 16;
    constexpr float run = (stairEnd - stairStart) / steps;
    constexpr float rise = deckHeight / steps;
    for (int index = 0; index < steps; ++index)
    {
        const float y = (index + 1) * rise;
        const float z = stairStart + (index + 0.5f) * run;
        AddBox({stairX, y - 0.05f, z}, {stairWidth, 0.10f, run},
            houseTrim * 0.87f, house, Surface::Wood);
        AddBox({stairX, y - rise * 0.5f, stairStart + index * run + 0.0175f},
            {stairWidth, rise, 0.035f}, houseTrim, house, Surface::Paint);
    }
    for (float side : {-1.0f, 1.0f})
    {
        const float x = stairX + side * (stairWidth * 0.5f + 0.055f);
        AddBeam(world({x, 0.04f, stairStart}), world({x, deckHeight - 0.07f, stairEnd}),
            0.10f, houseTrim * 0.78f, Surface::Paint);
        AddBeam(world({x, 1.00f, stairStart}), world({x, railTop, stairEnd}),
            0.08f, houseTrim, Surface::Paint);
        AddBeam(world({x, 0.32f, stairStart}), world({x, deckHeight + 0.32f, stairEnd}),
            0.06f, houseTrim, Surface::Paint);
        for (int index = 0; index <= 8; ++index)
        {
            const float t = static_cast<float>(index) / 8;
            const float floor = deckHeight * t;
            AddBox({x, floor + 0.50f, glm::mix(stairStart, stairEnd, t)},
                {0.055f, 0.96f, 0.055f}, houseTrim, house, Surface::Paint);
        }
        for (int segment = 0; segment < 4; ++segment)
        {
            const float t0 = static_cast<float>(segment) / 4;
            const float t1 = static_cast<float>(segment + 1) / 4;
            const float z0 = glm::mix(stairStart, stairEnd, t0);
            const float z1 = glm::mix(stairStart, stairEnd, t1);
            obstacle({x, 0.0f, (z0 + z1) * 0.5f}, {0.05f, (z1 - z0) * 0.5f},
                deckHeight * t0, deckHeight * t1 + 1.0f);
        }
    }

    if (yellow)
    {
        AddBox({0.0f, 2.59f, 5.25f}, {7.30f, 0.12f, 1.30f},
            houseTrim * 0.90f, house, Surface::Wood);
        traversal.AddPlatform(pointXZ({0.0f, 0.0f, 5.25f}), {3.65f, 0.65f}, yaw, deckHeight);
        for (float x : {-3.25f, 3.25f})
        {
            AddBox({x, 1.325f, 5.55f}, {0.17f, deckHeight, 0.17f}, houseTrim, house, Surface::Paint);
            obstacle({x, 0.0f, 5.55f}, {0.085f, 0.085f}, 0.0f, deckHeight);
        }
        rail(-3.65f, 5.90f, 3.65f, 5.90f, deckHeight, railTop);
        rail(-3.65f, 4.64f, -3.65f, 5.90f, deckHeight, railTop);
        rail(3.65f, 4.64f, 3.65f, 5.90f, deckHeight, railTop);
    }
}
