#pragma once

#include <cmath>
#include <vector>

#include <glm/glm.hpp>

class NuketownTraversal
{
public:
    // The two coordinates correspond to world X and Z.
    void AddObstacle(glm::vec2 center, glm::vec2 halfExtents, float yawDegrees = 0.0f)
    {
        const float angle = glm::radians(yawDegrees);
        const float cosine = std::cos(angle);
        const float sine = std::sin(angle);
        obstacles.push_back({center, halfExtents, {cosine, -sine}, {sine, cosine}});
    }

    bool IsWalkable(glm::vec2 position) const
    {
        for (const Obstacle& obstacle : obstacles)
        {
            const glm::vec2 offset = position - obstacle.center;
            const glm::vec2 local(glm::dot(offset, obstacle.right), glm::dot(offset, obstacle.forward));
            const glm::vec2 closest = glm::clamp(local, -obstacle.halfExtents, obstacle.halfExtents);
            const glm::vec2 separation = local - closest;
            if (glm::dot(separation, separation) < playerRadius * playerRadius)
                return false;
        }
        return true;
    }

    glm::vec3 ConstrainCamera(glm::vec3 target, glm::vec3 desired) const
    {
        const glm::vec3 span = desired - target;
        const float distance = glm::length(span);
        if (distance < 0.00001f || !IsWalkable({target.x, target.z}))
            return target;

        constexpr float maxStep = 0.08f;
        constexpr float boundaryMargin = 0.06f;
        const int steps = 1 + static_cast<int>(distance / maxStep);
        const glm::vec3 direction = span / distance;
        float safeDistance = 0.0f;

        // Check the straight camera ray without the player's sliding movement.
        for (int index = 1; index <= steps; ++index)
        {
            const float sampleDistance = distance * static_cast<float>(index) / steps;
            const glm::vec3 sample = target + direction * sampleDistance;
            if (!IsWalkable({sample.x, sample.z}))
                return target + direction * glm::max(0.0f, safeDistance - boundaryMargin);
            safeDistance = sampleDistance;
        }
        return desired;
    }

    glm::vec2 Move(glm::vec2 position, glm::vec2 delta) const
    {
        // Subdivide movement so even thin fences cannot be crossed in one update.
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

            // Keep the unblocked component to let the player slide along obstacles.
            if (IsWalkable(position + glm::vec2(step.x, 0.0f)))
                position.x += step.x;
            if (IsWalkable(position + glm::vec2(0.0f, step.y)))
                position.y += step.y;
        }
        return position;
    }

private:
    struct Obstacle
    {
        glm::vec2 center;
        glm::vec2 halfExtents;
        glm::vec2 right;
        glm::vec2 forward;
    };

    static constexpr float playerRadius = 0.32f;
    std::vector<Obstacle> obstacles;
};
