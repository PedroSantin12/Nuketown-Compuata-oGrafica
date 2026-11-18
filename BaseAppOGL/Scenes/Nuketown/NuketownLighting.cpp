#include "SceneNuketown.h"

#include <cmath>
#include <algorithm>
#include <vector>

namespace
{
    glm::mat4 PlaceStreetLamp(glm::vec2 position)
    {
        const glm::vec2 towardStreet = glm::normalize(-position);
        const float yaw = std::atan2(towardStreet.x, towardStreet.y);
        return glm::rotate(glm::translate(glm::mat4(1.0f), {position.x, 0.0f, position.y}),
            yaw, glm::vec3(0.0f, 1.0f, 0.0f));
    }
}

void SceneNuketown::BuildStreetLighting()
{
    const glm::vec2 positions[] = {
        {-4.90f, 12.40f}, {5.60f, 8.80f}, {-6.60f, -7.80f}, {6.60f, -10.30f}
    };
    const glm::vec3 darkEnamel(0.22f, 0.29f, 0.26f);
    const glm::vec3 metal(0.49f, 0.53f, 0.48f);
    const glm::vec3 concrete(0.58f, 0.57f, 0.49f);
    const glm::vec3 warmLight(1.0f, 0.85f, 0.58f);

    for (glm::vec2 position : positions)
    {
        const glm::mat4 lamp = PlaceStreetLamp(position);
        const auto world = [&lamp](glm::vec3 point)
        {
            return glm::vec3(lamp * glm::vec4(point, 1.0f));
        };

        // A compact cast base and a slender painted column fit the residential street.
        AddPart(cylinder, {0.0f, 0.075f, 0.0f}, {0.23f, 0.15f, 0.23f},
            concrete, lamp, Surface::Concrete);
        AddPart(cylinder, {0.0f, 0.29f, 0.0f}, {0.145f, 0.35f, 0.145f},
            darkEnamel, lamp, Surface::VehiclePaint);
        AddPart(cylinder, {0.0f, 2.20f, 0.0f}, {0.071f, 3.98f, 0.071f},
            darkEnamel, lamp, Surface::VehiclePaint);
        for (float y : {0.47f, 3.95f})
            AddPart(cylinder, {0.0f, y, 0.0f}, {0.107f, 0.055f, 0.107f},
                metal, lamp, Surface::Chrome);
        AddPart(sphere, {0.0f, 4.17f, 0.0f}, {0.094f, 0.095f, 0.094f},
            darkEnamel, lamp, Surface::VehiclePaint);

        // Two short arm sections give a gentle rise before the downward-facing head.
        AddBeam(world({0.0f, 4.04f, 0.0f}), world({0.0f, 4.36f, 0.25f}),
            0.063f, darkEnamel, Surface::VehiclePaint);
        AddBeam(world({0.0f, 4.36f, 0.25f}), world({0.0f, 4.36f, 0.82f}),
            0.063f, darkEnamel, Surface::VehiclePaint);
        AddPart(roundedBox, {0.0f, 4.29f, 0.83f}, {0.49f, 0.18f, 0.69f},
            darkEnamel, lamp, Surface::VehiclePaint);
        AddPart(roundedBox, {0.0f, 4.205f, 0.83f}, {0.445f, 0.048f, 0.64f},
            metal, lamp, Surface::Chrome);
        AddPart(roundedBox, {0.0f, 4.177f, 0.83f}, {0.385f, 0.022f, 0.565f},
            warmLight, lamp, Surface::Emissive);

        // The light is just below the glass so the housing cannot hide its origin.
        streetLightPositions.push_back(world({0.0f, 4.12f, 0.83f}));
        traversal.AddObstacle(position, {0.20f, 0.20f}, 0.0f, 0.0f, 4.43f);
    }
}


void SceneNuketown::BakeStreetLightMap()
{
    if (streetLightTexture != 0)
        return;

    constexpr int width = 512;
    constexpr int height = 384;
    constexpr float minimumX = -52.0f;
    constexpr float minimumZ = -35.0f;
    constexpr float spanX = 106.0f;
    constexpr float spanZ = 72.0f;
    const size_t lightCount = std::min<size_t>(streetLightPositions.size(), 8);
    std::vector<float> pixels(static_cast<size_t>(width) * height * 3, 0.0f);
    const auto smoothstep = [](float low, float high, float value)
    {
        const float t = std::clamp((value - low) / (high - low), 0.0f, 1.0f);
        return t * t * (3.0f - 2.0f * t);
    };

    // Static light pools on the flat ground can be sampled once instead of
    // evaluating eight cones for every pavement or sand fragment each frame.
    for (int row = 0; row < height; ++row)
        for (int column = 0; column < width; ++column)
        {
            const float worldX = minimumX + (column + 0.5f) * spanX / width;
            const float worldZ = minimumZ + (row + 0.5f) * spanZ / height;
            float energy = 0.0f;
            for (size_t index = 0; index < lightCount; ++index)
            {
                const glm::vec3& position = streetLightPositions[index];
                const float dx = position.x - worldX;
                const float dy = position.y;
                const float dz = position.z - worldZ;
                const float distanceSquared = dx * dx + dy * dy + dz * dz;
                const float radius = index < 4 ? 8.0f : 13.0f;
                if (distanceSquared >= radius * radius || dy <= 0.0f)
                    continue;
                const float inverseDistance = 1.0f / std::sqrt(std::max(distanceSquared, 0.0001f));
                const float diffuse = dy * inverseDistance;
                const float cone = smoothstep(0.422618f, 0.939693f, diffuse);
                if (cone <= 0.0f)
                    continue;
                const float distance = distanceSquared * inverseDistance;
                const float falloff = std::max(0.0f, 1.0f - distanceSquared / (radius * radius));
                const float attenuation = falloff * falloff
                    / (1.0f + 0.15f * distance + 0.06f * distanceSquared);
                const float intensity = index < 4 ? 1.8f : 3.0f;
                energy += diffuse * cone * attenuation * intensity;
            }
            const size_t pixel = (static_cast<size_t>(row) * width + column) * 3;
            pixels[pixel] = energy;
            pixels[pixel + 1] = energy * 0.81f;
            pixels[pixel + 2] = energy * 0.53f;
        }

    glGenTextures(1, &streetLightTexture);
    glActiveTexture(GL_TEXTURE10);
    glBindTexture(GL_TEXTURE_2D, streetLightTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0,
        GL_RGB, GL_FLOAT, pixels.data());
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    const float border[] = {0.0f, 0.0f, 0.0f, 0.0f};
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border);
    glActiveTexture(GL_TEXTURE0);
}
