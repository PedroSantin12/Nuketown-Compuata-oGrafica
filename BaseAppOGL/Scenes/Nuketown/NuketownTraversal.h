#pragma once

#include <algorithm>
#include <cmath>
#include <vector>

#include <glm/glm.hpp>

class NuketownTraversal
{
public:
    // The two coordinates correspond to world X and Z. Existing obstacles stay solid
    // from the ground upward; optional heights also allow walking under a balcony.
    void AddObstacle(glm::vec2 center, glm::vec2 halfExtents, float yawDegrees = 0.0f,
        float bottom = 0.0f, float top = 1000.0f)
    {
        const float angle = glm::radians(yawDegrees);
        const float cosine = std::cos(angle);
        const float sine = std::sin(angle);
        if (bottom > top)
            std::swap(bottom, top);
        obstacles.push_back({center, glm::abs(halfExtents), {cosine, -sine},
            {sine, cosine}, bottom, top});
    }

    void AddPlatform(glm::vec2 center, glm::vec2 halfExtents, float yawDegrees, float height)
    {
        const float angle = glm::radians(yawDegrees);
        const float cosine = std::cos(angle);
        const float sine = std::sin(angle);
        platforms.push_back({center, glm::abs(halfExtents), {cosine, -sine},
            {sine, cosine}, height});
    }

    // A ramp is a rectangular walkable surface, measured from start to end.
    // Its visual steps can be modeled separately without individual step colliders.
    void AddRamp(glm::vec2 start, glm::vec2 end, float width,
        float startHeight, float endHeight)
    {
        const glm::vec2 span = end - start;
        const float length = glm::length(span);
        if (length < epsilon || width <= 0.0f)
            return;
        const glm::vec2 forward = span / length;
        ramps.push_back({start, forward, {forward.y, -forward.x}, length,
            width * 0.5f, startHeight, endHeight});
    }

    bool IsWalkable(glm::vec2 position) const
    {
        return IsWalkableAt(position, 0.0f);
    }

    bool IsWalkableAt(glm::vec2 position, float feetHeight, float bodyHeight = 1.73f) const
    {
        if (!ObstaclesClearAt(position, feetHeight, bodyHeight))
            return false;

        // A surface above the feet can act as a ceiling. A high deck still leaves
        // the ground below it accessible, while a low deck cannot cut through a body.
        for (const Platform& platform : platforms)
        {
            if (Contains(platform, position) &&
                IntersectsSurface(feetHeight, bodyHeight, platform.height))
                return false;
        }
        for (const Ramp& ramp : ramps)
        {
            float height = 0.0f;
            if (RampHeightAt(ramp, position, height) &&
                IntersectsSurface(feetHeight, bodyHeight, height))
                return false;
        }
        return true;
    }

    glm::vec3 ConstrainCamera(glm::vec3 target, glm::vec3 desired) const
    {
        const glm::vec3 span = desired - target;
        const float distance = glm::length(span);
        constexpr float rayBodyHeight = 0.10f;
        const auto clear = [this](glm::vec3 point)
        {
            return IsWalkableAt({point.x, point.z}, point.y - rayBodyHeight * 0.5f,
                rayBodyHeight);
        };
        if (distance < epsilon || !clear(target))
            return target;

        constexpr float maxStep = 0.08f;
        constexpr float boundaryMargin = 0.06f;
        const int steps = 1 + static_cast<int>(distance / maxStep);
        const glm::vec3 direction = span / distance;
        float safeDistance = 0.0f;

        // Test the actual 3D ray so elevated rails do not block a ground camera.
        for (int index = 1; index <= steps; ++index)
        {
            const float sampleDistance = distance * static_cast<float>(index) / steps;
            const glm::vec3 sample = target + direction * sampleDistance;
            if (!clear(sample))
                return target + direction * glm::max(0.0f, safeDistance - boundaryMargin);
            safeDistance = sampleDistance;
        }
        return desired;
    }

    glm::vec2 Move(glm::vec2 position, glm::vec2 delta) const
    {
        // Preserve the original ground-only API for existing callers.
        constexpr float maxStep = 0.12f;
        const int steps = 1 + static_cast<int>(glm::length(delta) / maxStep);
        const glm::vec2 step = delta / static_cast<float>(steps);
        for (int index = 0; index < steps; ++index)
        {
            if (IsWalkable(position + step))
            {
                position += step;
                continue;
            }
            if (IsWalkable(position + glm::vec2(step.x, 0.0f)))
                position.x += step.x;
            if (IsWalkable(position + glm::vec2(0.0f, step.y)))
                position.y += step.y;
        }
        return position;
    }

    glm::vec3 MoveFeet(glm::vec3 feet, glm::vec2 delta) const
    {
        constexpr float maxStep = 0.12f;
        const int steps = 1 + static_cast<int>(glm::length(delta) / maxStep);
        const glm::vec2 step = delta / static_cast<float>(steps);
        for (int index = 0; index < steps; ++index)
        {
            const float stepStartHeight = feet.y;
            if (TryMoveFeet(feet, step, stepStartHeight))
                continue;
            // Slide with the same height limit as the complete step. This avoids
            // gaining two step heights by splitting a blocked diagonal movement.
            TryMoveFeet(feet, {step.x, 0.0f}, stepStartHeight);
            TryMoveFeet(feet, {0.0f, step.y}, stepStartHeight);
        }
        return feet;
    }

private:
    struct Obstacle
    {
        glm::vec2 center;
        glm::vec2 halfExtents;
        glm::vec2 right;
        glm::vec2 forward;
        float bottom;
        float top;
    };

    struct Platform
    {
        glm::vec2 center;
        glm::vec2 halfExtents;
        glm::vec2 right;
        glm::vec2 forward;
        float height;
    };

    struct Ramp
    {
        glm::vec2 start;
        glm::vec2 forward;
        glm::vec2 right;
        float length;
        float halfWidth;
        float startHeight;
        float endHeight;
    };

    bool ObstaclesClearAt(glm::vec2 position, float feetHeight, float bodyHeight) const
    {
        for (const Obstacle& obstacle : obstacles)
        {
            if (feetHeight >= obstacle.top - epsilon ||
                feetHeight + bodyHeight <= obstacle.bottom + epsilon)
                continue;
            const glm::vec2 offset = position - obstacle.center;
            const glm::vec2 local(glm::dot(offset, obstacle.right), glm::dot(offset, obstacle.forward));
            const glm::vec2 closest = glm::clamp(local, -obstacle.halfExtents, obstacle.halfExtents);
            const glm::vec2 separation = local - closest;
            if (glm::dot(separation, separation) < playerRadius * playerRadius)
                return false;
        }
        return true;
    }

    static bool Contains(const Platform& platform, glm::vec2 position)
    {
        const glm::vec2 offset = position - platform.center;
        return std::abs(glm::dot(offset, platform.right)) <= platform.halfExtents.x + epsilon &&
            std::abs(glm::dot(offset, platform.forward)) <= platform.halfExtents.y + epsilon;
    }

    static bool RampHeightAt(const Ramp& ramp, glm::vec2 position, float& height)
    {
        const glm::vec2 offset = position - ramp.start;
        const float along = glm::dot(offset, ramp.forward);
        if (along < -epsilon || along > ramp.length + epsilon ||
            std::abs(glm::dot(offset, ramp.right)) > ramp.halfWidth + epsilon)
            return false;
        const float t = glm::clamp(along / ramp.length, 0.0f, 1.0f);
        height = glm::mix(ramp.startHeight, ramp.endHeight, t);
        return true;
    }

    static bool IntersectsSurface(float feetHeight, float bodyHeight, float surfaceHeight)
    {
        return feetHeight < surfaceHeight - epsilon &&
            feetHeight + bodyHeight > surfaceHeight + epsilon;
    }

    float FloorHeightAt(glm::vec2 position, float maximumHeight) const
    {
        float floor = 0.0f;
        for (const Platform& platform : platforms)
        {
            if (platform.height <= maximumHeight + epsilon && platform.height > floor &&
                Contains(platform, position))
                floor = platform.height;
        }
        for (const Ramp& ramp : ramps)
        {
            float height = 0.0f;
            if (RampHeightAt(ramp, position, height) && height <= maximumHeight + epsilon)
                floor = std::max(floor, height);
        }
        return floor;
    }

    bool TryMoveFeet(glm::vec3& feet, glm::vec2 step, float stepStartHeight) const
    {
        const glm::vec2 candidate = glm::vec2(feet.x, feet.z) + step;
        // Check rails at the departure height before considering a downward snap.
        // Otherwise stepping past a deck edge could teleport below an elevated rail.
        if (!ObstaclesClearAt(candidate, feet.y, playerBodyHeight))
            return false;
        const float heightLimit = std::min(feet.y, stepStartHeight) + maxStepHeight;
        const float height = FloorHeightAt(candidate, heightLimit);
        if (!IsWalkableAt(candidate, height, playerBodyHeight))
            return false;
        feet = {candidate.x, height, candidate.y};
        return true;
    }

    static constexpr float playerRadius = 0.32f;
    static constexpr float playerBodyHeight = 1.73f;
    static constexpr float maxStepHeight = 0.24f;
    static constexpr float epsilon = 0.0001f;
    std::vector<Obstacle> obstacles;
    std::vector<Platform> platforms;
    std::vector<Ramp> ramps;
};
