#include "SceneNuketown.h"

#include <algorithm>
#include <cmath>

namespace
{
    glm::mat4 LandscapeFrame(glm::vec3 center, glm::vec3 up)
    {
        up = glm::normalize(up);
        const glm::vec3 reference = std::abs(up.z) < 0.90f
            ? glm::vec3(0.0f, 0.0f, 1.0f) : glm::vec3(1.0f, 0.0f, 0.0f);
        const glm::vec3 right = glm::normalize(glm::cross(up, reference));
        const glm::vec3 forward = glm::cross(right, up);
        return glm::mat4(glm::vec4(right, 0.0f), glm::vec4(up, 0.0f),
            glm::vec4(forward, 0.0f), glm::vec4(center, 1.0f));
    }

    float RiverCenter(float z)
    {
        return 52.0f + 4.0f * std::sin(z * 0.07f);
    }
}

void SceneNuketown::BuildPalm(glm::vec2 position, float height, float yaw)
{
    height = std::clamp(height, 4.0f, 9.0f);
    const float angle = glm::radians(yaw);
    const glm::vec3 lean(std::cos(angle) * height * 0.055f, 0.0f,
        std::sin(angle) * height * 0.055f);
    const glm::vec3 root(position.x, -0.035f, position.y);
    const glm::vec3 bark(0.47f, 0.37f, 0.24f);
    AddPart(cylinder, {position.x, -0.032f, position.y}, {0.48f, 0.004f, 0.48f},
        {0.12f, 0.10f, 0.07f}, glm::mat4(1.0f), Surface::Shadow);
    const auto point = [&root, &lean, height](float t)
    {
        return root + glm::vec3(lean.x * t * t, height * t, lean.z * t * t);
    };

    // Four slightly overlapping segments bend the trunk without leaving seams.
    constexpr int segments = 4;
    for (int segment = 0; segment < segments; ++segment)
    {
        const float t0 = static_cast<float>(segment) / segments;
        const float t1 = static_cast<float>(segment + 1) / segments;
        const glm::vec3 start = point(t0), end = point(t1);
        const float radius = (0.14f - (t0 + t1) * 0.025f) * height / 7.0f;
        const glm::mat4 frame = LandscapeFrame((start + end) * 0.5f, end - start);
        AddPart(cylinder, {0.0f, 0.0f, 0.0f},
            {radius, glm::length(end - start) + 0.045f, radius}, bark, frame, Surface::Wood);
    }
    for (int ring = 1; ring <= 6; ++ring)
    {
        const float t = static_cast<float>(ring) / 7.0f;
        const float radius = (0.14f - t * 0.05f) * height / 7.0f * 1.08f;
        const glm::vec3 tangent(2.0f * lean.x * t, height, 2.0f * lean.z * t);
        AddPart(cylinder, {0.0f, 0.0f, 0.0f}, {radius, 0.045f, radius},
            bark * 0.77f, LandscapeFrame(point(t), tangent), Surface::Wood);
    }

    const glm::vec3 crown = point(1.0f);
    AddPart(sphere, crown, {0.19f, 0.24f, 0.19f},
        {0.30f, 0.37f, 0.15f}, glm::mat4(1.0f), Surface::Foliage);
    for (int leaf = 0; leaf < 8; ++leaf)
    {
        const float leafYaw = yaw + leaf * 45.0f;
        const float leafLength = 2.08f + (leaf % 3) * 0.16f;
        const glm::mat4 leafFrame = glm::rotate(glm::translate(glm::mat4(1.0f),
            crown + glm::vec3(0.0f, 0.04f + (leaf % 2) * 0.06f, 0.0f)),
            glm::radians(leafYaw), glm::vec3(0.0f, 1.0f, 0.0f));
        AddPart(palmFrond, {0.0f, 0.0f, 0.0f}, {0.54f, 0.85f, leafLength},
            leaf % 3 == 0 ? glm::vec3(0.32f, 0.43f, 0.18f) : glm::vec3(0.23f, 0.38f, 0.15f),
            leafFrame, Surface::Foliage);
    }
    for (float side : {-1.0f, 1.0f})
        AddPart(sphere, crown + glm::vec3(side * 0.13f, -0.15f, 0.06f),
            {0.105f, 0.13f, 0.11f}, {0.34f, 0.27f, 0.14f});
    // The trees are scenery beyond the existing perimeter, so they need no gameplay collider.
}

void SceneNuketown::BuildEnvironment()
{
    const glm::mat4 identity(1.0f);
    const glm::vec3 sand(0.72f, 0.63f, 0.47f);

    // The world-space basin stays outside R=92. It is never rotated around a translated origin.
    AddBox({0.0f, -0.22f, 0.0f}, {1400.0f, 0.36f, 1400.0f}, sand, identity, Surface::Sand);
    AddPart(desertBasin, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f},
        {0.63f, 0.56f, 0.44f}, identity, Surface::Mountains);
    AddPart(riverRibbon, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f},
        {0.08f, 0.35f, 0.39f}, identity, Surface::Water);

    // Low, narrow banks stay outside the maximum 2.25 m half-width of the water.
    constexpr int bankSegments = 24;
    constexpr float riverStart = -28.0f;
    constexpr float riverEnd = 26.0f;
    for (int segment = 0; segment < bankSegments; ++segment)
    {
        const float z0 = glm::mix(riverStart, riverEnd, static_cast<float>(segment) / bankSegments);
        const float z1 = glm::mix(riverStart, riverEnd, static_cast<float>(segment + 1) / bankSegments);
        for (float side : {-1.0f, 1.0f})
            AddPath({RiverCenter(z0) + side * 2.65f, z0},
                {RiverCenter(z1) + side * 2.65f, z1}, 0.68f, 0.004f,
                {0.62f, 0.55f, 0.39f}, Surface::Sand);
    }

    BuildPalm({54.2f, -13.0f}, 6.1f, -22.0f);
    BuildPalm({46.1f, -1.0f}, 7.3f, 133.0f);
    BuildPalm({60.0f, 12.0f}, 6.8f, 25.0f);
    BuildPalm({50.7f, 21.0f}, 5.9f, -74.0f);

    const glm::vec2 rocks[] = {
        {44.0f, -24.0f}, {44.6f, -7.0f}, {61.0f, 15.0f}, {65.0f, -16.0f},
        {69.0f, 8.0f}, {-48.0f, 21.0f}, {-53.0f, -14.0f}, {8.0f, -52.0f}
    };
    for (int index = 0; index < 8; ++index)
    {
        const float size = 0.62f + (index % 3) * 0.19f;
        AddPart(cylinder, {rocks[index].x, -0.032f, rocks[index].y},
            {size * 1.05f, 0.004f, size * 0.80f}, {0.12f, 0.10f, 0.07f}, identity, Surface::Shadow);
        const glm::mat4 rock = glm::rotate(glm::translate(identity,
            {rocks[index].x, 0.16f + (index % 2) * 0.08f, rocks[index].y}),
            glm::radians(index * 37.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        AddPart(sphere, {0.0f, 0.0f, 0.0f}, {size, 0.30f + (index % 2) * 0.08f, size * 0.72f},
            {0.60f, 0.55f, 0.45f}, rock, Surface::Stone);
    }

    const glm::vec2 dryBrush[] = {
        {43.4f, -19.0f}, {44.5f, 0.0f}, {47.5f, 20.0f}, {62.8f, 6.0f},
        {64.0f, 24.0f}, {67.0f, -5.0f}, {60.0f, -25.0f}, {72.0f, 17.0f},
        {-45.0f, -7.0f}, {-54.0f, 24.0f}, {-11.0f, -49.0f}, {18.0f, 47.0f}
    };
    const glm::vec3 dryWood(0.43f, 0.37f, 0.23f);
    for (int index = 0; index < 12; ++index)
    {
        const glm::vec3 root(dryBrush[index].x, -0.015f, dryBrush[index].y);
        const float height = 0.32f + (index % 4) * 0.06f;
        AddBeam(root, root + glm::vec3(0.0f, height, 0.0f), 0.035f, dryWood);
        AddBeam(root + glm::vec3(0.0f, height * 0.43f, 0.0f),
            root + glm::vec3(0.23f, height * 0.87f, 0.10f), 0.025f, dryWood);
        AddBeam(root + glm::vec3(0.0f, height * 0.58f, 0.0f),
            root + glm::vec3(-0.18f, height * 0.94f, 0.14f), 0.024f, dryWood);
        AddBeam(root + glm::vec3(0.0f, height * 0.50f, 0.0f),
            root + glm::vec3(0.05f, height * 0.81f, -0.21f), 0.022f, dryWood);
    }
}
