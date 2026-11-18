#include "SceneNuketown.h"

void SceneNuketown::BuildEnvironment()
{
    const glm::mat4 identity(1.0f);
    const glm::vec3 sand(0.72f, 0.63f, 0.47f);

    // Keep the closed terrain and distant mountain ring as the city's backdrop.
    AddBox({0.0f, -0.22f, 0.0f}, {1400.0f, 0.36f, 1400.0f}, sand, identity, Surface::Sand);
    AddPart(desertBasin, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f},
        {0.63f, 0.56f, 0.44f}, identity, Surface::Mountains);
}
