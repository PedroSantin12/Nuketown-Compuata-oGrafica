#include "SceneNuketown.h"

namespace
{
    glm::mat4 PlaceLandmark(glm::vec3 position, float yaw)
    {
        return glm::rotate(glm::translate(glm::mat4(1.0f), position),
            glm::radians(yaw), glm::vec3(0.0f, 1.0f, 0.0f));
    }
}

void SceneNuketown::BuildParkedCar(glm::vec2 center, float yaw, glm::vec3 color, bool police)
{
    const glm::mat4 car = PlaceLandmark({center.x, 0.0f, center.y}, yaw);
    const glm::vec3 chrome(0.64f, 0.69f, 0.72f);
    const glm::vec3 rubber(0.035f, 0.042f, 0.045f);
    const glm::vec3 glass(0.20f, 0.32f, 0.38f);
    const glm::vec3 roofColor(0.86f, 0.84f, 0.73f);
    const auto world = [&car](glm::vec3 p) { return glm::vec3(car * glm::vec4(p, 1.0f)); };
    traversal.AddObstacle(center, {1.12f, 2.44f}, yaw);

    AddPart(cylinder, {0.0f, 0.055f, 0.0f}, {1.16f, 0.006f, 2.53f}, rubber, car, Surface::Shadow);
    AddPart(roundedBox, {0.0f, 0.39f, 0.0f}, {1.70f, 0.16f, 4.10f}, rubber, car, Surface::Tire);
    AddPart(carBody, {0.0f, 0.70f, 0.0f}, {1.95f, 0.65f, 4.65f}, color, car, Surface::VehiclePaint);

    // One sloping glazed shell forms the windscreen, rear screen and side windows.
    // The roof and slim pillars cover it rather than placing rectangular panes outside a block.
    AddPart(carCabin, {0.0f, 1.28f, -0.25f}, {1.60f, 0.78f, 2.14f}, glass, car, Surface::VehicleGlass);
    AddPart(roundedBox, {0.0f, 1.69f, -0.314f}, {1.36f, 0.10f, 1.28f}, roofColor, car, Surface::VehiclePaint);
    for (float side : {-1.0f, 1.0f})
    {
        AddBeam(world({side * 0.81f, 0.97f, 0.82f}), world({side * 0.65f, 1.69f, 0.307f}),
            0.056f, roofColor, Surface::VehiclePaint);
        AddBeam(world({side * 0.81f, 0.97f, -1.32f}), world({side * 0.65f, 1.69f, -0.935f}),
            0.067f, roofColor, Surface::VehiclePaint);
        AddBeam(world({side * 0.80f, 0.99f, -0.29f}), world({side * 0.65f, 1.67f, -0.29f}),
            0.062f, roofColor, Surface::VehiclePaint);
        AddBeam(world({side * 0.81f, 1.00f, -1.27f}), world({side * 0.81f, 1.00f, 0.77f}),
            0.025f, chrome, Surface::Chrome);
        AddBeam(world({side * 0.65f, 1.68f, -0.93f}), world({side * 0.65f, 1.68f, 0.30f}),
            0.023f, chrome, Surface::Chrome);
        AddPart(roundedBox, {side * 0.963f, 0.75f, -0.13f}, {0.018f, 0.026f, 3.95f},
            chrome, car, Surface::Chrome);
        AddPart(roundedBox, {side * 0.917f, 0.48f, -0.08f}, {0.022f, 0.046f, 3.95f},
            rubber, car, Surface::Tire);
        for (float z : {-0.89f, 0.17f})
        {
            AddPart(roundedBox, {side * 0.981f, 0.91f, z}, {0.035f, 0.026f, 0.18f}, chrome, car, Surface::Chrome);
            AddPart(roundedBox, {side * 0.985f, 0.87f, z - 0.09f}, {0.018f, 0.032f, 0.027f}, chrome, car, Surface::Chrome);
        }
        // Recessed dark wheel wells retain the curved outline behind the tires.
        for (float z : {-1.45f, 1.45f})
        {
            const glm::mat4 well = glm::rotate(glm::translate(car, {side * 0.975f, 0.43f, z}),
                glm::radians(-side * 90.0f), glm::vec3(0, 0, 1));
            AddPart(cylinder, {0, 0, 0}, {0.405f, 0.015f, 0.405f}, rubber, well, Surface::Tire);
        }
        AddBeam(world({side * 0.80f, 1.11f, 0.57f}), world({side * 0.985f, 1.14f, 0.58f}),
            0.022f, chrome, Surface::Chrome);
        AddPart(sphere, {side * 1.01f, 1.17f, 0.58f}, {0.065f, 0.044f, 0.085f}, chrome, car, Surface::Chrome);

        const glm::mat4 headlamp = glm::rotate(glm::translate(car, {side * 0.69f, 0.80f, 2.335f}),
            glm::radians(90.0f), glm::vec3(1, 0, 0));
        AddPart(cylinder, {0, 0, 0}, {0.148f, 0.050f, 0.148f}, chrome, headlamp, Surface::Chrome);
        AddPart(cylinder, {0, 0.031f, 0}, {0.121f, 0.012f, 0.121f}, {0.89f, 0.87f, 0.72f}, headlamp);
        AddPart(roundedBox, {side * 0.825f, 0.59f, 2.337f}, {0.115f, 0.062f, 0.028f},
            {0.74f, 0.34f, 0.055f}, car, Surface::VehiclePaint);
        const glm::mat4 tail = glm::rotate(glm::translate(car, {side * 0.73f, 0.76f, -2.335f}),
            glm::radians(-90.0f), glm::vec3(1, 0, 0));
        AddPart(cylinder, {0, 0, 0}, {0.105f, 0.036f, 0.105f}, chrome, tail, Surface::Chrome);
        AddPart(cylinder, {0, 0.023f, 0}, {0.083f, 0.012f, 0.083f}, {0.57f, 0.045f, 0.025f}, tail, Surface::VehiclePaint);
    }
    AddBeam(world({0, 0.99f, 0.84f}), world({0, 1.67f, 0.313f}), 0.023f, chrome, Surface::Chrome);
    AddBeam(world({-0.78f, 1.00f, 0.84f}), world({0.78f, 1.00f, 0.84f}), 0.025f, chrome, Surface::Chrome);
    AddBeam(world({-0.65f, 1.68f, 0.315f}), world({0.65f, 1.68f, 0.315f}), 0.021f, chrome, Surface::Chrome);
    AddPart(roundedBox, {0.0f, 0.48f, 2.29f}, {1.97f, 0.13f, 0.17f}, chrome, car, Surface::Chrome);
    AddPart(roundedBox, {0.0f, 0.48f, -2.29f}, {1.97f, 0.13f, 0.17f}, chrome, car, Surface::Chrome);
    AddPart(roundedBox, {0.0f, 0.76f, 2.342f}, {0.96f, 0.22f, 0.027f}, rubber, car, Surface::Tire);
    for (float y : {0.70f, 0.75f, 0.80f})
        AddPart(roundedBox, {0.0f, y, 2.359f}, {0.89f, 0.013f, 0.016f}, chrome, car, Surface::Chrome);
    AddPart(roundedBox, {0.0f, 0.61f, -2.328f}, {0.36f, 0.15f, 0.015f}, roofColor, car, Surface::Paint);
    AddBeam(world({0, 1.017f, 0.93f}), world({0, 0.995f, 1.99f}), 0.014f, chrome, Surface::Chrome);
    if (police)
        for (float side : {-1.0f, 1.0f})
            AddBox({side * 0.982f, 0.83f, -0.23f}, {0.012f, 0.20f, 1.62f}, roofColor, car, Surface::PoliceMark);
    AddWheels(car, 0.97f, 0.35f, 1.45f, -1.45f);
}

void SceneNuketown::BuildLandmarks()
{
    const glm::vec3 chrome(0.65f, 0.65f, 0.56f);
    const glm::vec3 cream(0.89f, 0.85f, 0.67f);
    const glm::vec3 teal(0.24f, 0.42f, 0.39f);
    const glm::mat4 sign = PlaceLandmark({5.9f, 0.0f, 14.6f}, -90.0f);

    // Leave a complete walking lane between the car and the closed end fence.
    BuildParkedCar({0.45f, 16.0f}, 75.0f, {0.44f, 0.50f, 0.45f}, true);
    // The driveway car leaves enough room to enter the newly opened green garage.
    BuildParkedCar({-8.0410f, 2.0183f}, 68.0f, {0.62f, 0.68f, 0.56f});

    // The sign faces the road. Its face has its own readable, non-repeating texture.
    for (float side : {-1.0f, 1.0f})
    {
        AddPart(cylinder, {side * 1.60f, 1.48f, -0.02f}, {0.075f, 2.88f, 0.075f}, chrome, sign);
        AddBox({side * 1.60f, 0.065f, -0.02f}, {0.37f, 0.13f, 0.37f}, cream * 0.80f, sign, Surface::Concrete);
        AddPart(sphere, {side * 1.60f, 2.99f, -0.02f}, {0.11f, 0.11f, 0.11f}, chrome, sign);
    }
    AddBox({0.0f, 2.03f, 0.0f}, {4.42f, 1.94f, 0.20f}, teal, sign, Surface::Paint);
    AddBox({0.0f, 2.03f, 0.115f}, {4.27f, 1.80f, 0.045f}, cream, sign, Surface::Paint);
    AddBox({0.0f, 2.03f, 0.147f}, {4.05f, 1.64f, 0.022f}, {1.0f, 1.0f, 1.0f}, sign, Surface::Sign);
    AddBox({0.0f, 1.00f, 0.0f}, {3.60f, 0.075f, 0.16f}, chrome, sign);
    traversal.AddObstacle({5.9f, 14.6f}, {2.25f, 0.22f}, -90.0f);

    // A hydrant and bin occupy the roadside verge, leaving both sidewalks open.
    const glm::mat4 hydrant = PlaceLandmark({-4.97f, 0.0f, 14.0f}, 0.0f);
    const glm::vec3 hydrantRed(0.61f, 0.23f, 0.15f);
    AddPart(cylinder, {0.0f, 0.46f, 0.0f}, {0.13f, 0.79f, 0.13f}, hydrantRed, hydrant, Surface::Paint);
    AddPart(cylinder, {0.0f, 0.095f, 0.0f}, {0.22f, 0.10f, 0.22f}, hydrantRed, hydrant, Surface::Paint);
    AddPart(sphere, {0.0f, 0.88f, 0.0f}, {0.19f, 0.11f, 0.19f}, hydrantRed, hydrant, Surface::Paint);
    const glm::mat4 outlets = glm::rotate(glm::translate(hydrant, {0.0f, 0.55f, 0.0f}),
        glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    AddPart(cylinder, {0.0f, 0.0f, 0.0f}, {0.095f, 0.51f, 0.095f}, hydrantRed, outlets, Surface::Paint);
    for (float side : {-1.0f, 1.0f})
        AddPart(cylinder, {0.0f, side * 0.275f, 0.0f}, {0.12f, 0.07f, 0.12f}, chrome, outlets);
    traversal.AddObstacle({-4.97f, 14.0f}, {0.28f, 0.21f});

    const glm::mat4 bin = PlaceLandmark({7.15f, 0.0f, 11.45f}, 0.0f);
    AddPart(cylinder, {0.0f, 0.46f, 0.0f}, {0.26f, 0.88f, 0.26f}, teal * 0.69f, bin, Surface::Paint);
    AddPart(cylinder, {0.0f, 0.94f, 0.0f}, {0.30f, 0.08f, 0.30f}, teal * 0.52f, bin, Surface::Paint);
    AddBox({0.0f, 1.005f, 0.0f}, {0.16f, 0.075f, 0.065f}, chrome, bin);
    traversal.AddObstacle({7.15f, 11.45f}, {0.30f, 0.30f});
}
