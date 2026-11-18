#include "SceneNuketown.h"

#include <cmath>

namespace
{
    glm::mat4 PlaceVehicle(glm::vec3 position, float yaw)
    {
        return glm::rotate(glm::translate(glm::mat4(1.0f), position),
            glm::radians(yaw), glm::vec3(0.0f, 1.0f, 0.0f));
    }
}

void SceneNuketown::BuildTruck()
{
    const glm::mat4 vehicle = PlaceVehicle({2.54f, 0.0f, -2.85f}, 13.0f);
    const glm::vec3 collisionCenter = glm::vec3(vehicle * glm::vec4(0.0f, 0.0f, -0.44f, 1.0f));
    traversal.AddObstacle({collisionCenter.x, collisionCenter.z}, {1.39f, 3.77f}, 13.0f);
    const glm::vec3 red(0.57f, 0.125f, 0.085f);
    const glm::vec3 ivory(0.85f, 0.83f, 0.71f);
    const glm::vec3 aluminium(0.63f, 0.68f, 0.69f);
    const glm::vec3 chrome(0.68f, 0.72f, 0.75f);
    const glm::vec3 rubber(0.032f, 0.038f, 0.042f);
    const glm::vec3 glass(0.20f, 0.32f, 0.39f);
    const auto world = [&vehicle](glm::vec3 p) { return glm::vec3(vehicle * glm::vec4(p, 1.0f)); };

    AddPart(cylinder, {0.0f, 0.055f, -0.15f}, {1.72f, 0.006f, 4.26f}, rubber, vehicle, Surface::Shadow);
    AddPart(roundedBox, {0.0f, 0.74f, -0.46f}, {2.29f, 0.23f, 7.14f}, rubber, vehicle, Surface::Tire);

    // A separate rounded bonnet, mudguards and a tapered cab replace the old stacked cubes.
    AddPart(truckHood, {0.0f, 1.34f, 2.11f}, {1.75f, 0.98f, 2.18f}, red, vehicle, Surface::VehiclePaint);
    AddPart(roundedBox, {0.0f, 1.58f, 1.12f}, {2.13f, 1.10f, 1.85f}, red, vehicle, Surface::VehiclePaint);
    AddPart(carCabin, {0.0f, 2.14f, 1.20f}, {2.16f, 1.38f, 1.92f}, glass, vehicle, Surface::VehicleGlass);
    AddPart(roundedBox, {0.0f, 2.87f, 1.14f}, {1.80f, 0.15f, 1.20f}, ivory, vehicle, Surface::VehiclePaint);
    AddPart(roundedBox, {0.0f, 1.91f, 1.97f}, {2.03f, 0.43f, 0.29f}, red, vehicle, Surface::VehiclePaint);
    AddPart(roundedBox, {0.0f, 0.68f, 3.235f}, {2.61f, 0.17f, 0.24f}, chrome, vehicle, Surface::Chrome);
    AddPart(roundedBox, {0.0f, 1.31f, 3.225f}, {1.09f, 1.01f, 0.065f}, chrome, vehicle, Surface::Chrome);
    AddPart(roundedBox, {0.0f, 1.31f, 3.267f}, {0.95f, 0.87f, 0.027f}, rubber, vehicle, Surface::Tire);
    for (int slat = 0; slat < 11; ++slat)
        AddPart(roundedBox, {-0.40f + slat * 0.08f, 1.31f, 3.291f},
            {0.028f, 0.78f, 0.018f}, chrome, vehicle, Surface::Chrome);
    AddPart(roundedBox, {0.0f, 0.81f, 3.376f}, {0.41f, 0.18f, 0.018f}, ivory, vehicle, Surface::Paint);
    AddBeam(world({0.0f, 1.86f, 1.21f}), world({0.0f, 1.80f, 3.00f}), 0.017f, chrome, Surface::Chrome);

    for (float side : {-1.0f, 1.0f})
    {
        AddPart(truckHood, {side * 1.01f, 1.08f, 2.08f}, {0.52f, 0.57f, 2.10f},
            red, vehicle, Surface::VehiclePaint);
        AddPart(roundedBox, {side * 1.04f, 1.62f, 1.12f}, {0.10f, 1.04f, 1.77f}, red, vehicle, Surface::VehiclePaint);
        AddBeam(world({side * 0.98f, 2.12f, 1.95f}), world({side * 0.88f, 2.79f, 1.72f}),
            0.068f, ivory, Surface::VehiclePaint);
        AddBeam(world({side * 1.00f, 2.12f, 0.48f}), world({side * 0.88f, 2.79f, 0.61f}),
            0.075f, red, Surface::VehiclePaint);
        AddBeam(world({side * 1.00f, 2.12f, 0.49f}), world({side * 1.00f, 2.12f, 1.92f}),
            0.027f, chrome, Surface::Chrome);
        AddPart(roundedBox, {side * 1.10f, 1.74f, 0.59f}, {0.038f, 0.034f, 0.20f}, chrome, vehicle, Surface::Chrome);
        AddPart(roundedBox, {side * 1.12f, 0.72f, 0.83f}, {0.40f, 0.12f, 1.01f}, aluminium * 0.65f, vehicle, Surface::Chrome);
        for (float z : {0.48f, 0.68f, 0.88f, 1.08f})
            AddPart(roundedBox, {side * 1.12f, 0.786f, z}, {0.31f, 0.013f, 0.045f}, rubber, vehicle, Surface::Tire);
        AddBeam(world({side * 0.93f, 2.23f, 1.80f}), world({side * 1.30f, 2.32f, 1.75f}),
            0.028f, chrome, Surface::Chrome);
        AddPart(roundedBox, {side * 1.32f, 2.35f, 1.73f}, {0.11f, 0.24f, 0.20f}, chrome, vehicle, Surface::Chrome);
        AddPart(roundedBox, {side * 1.385f, 2.35f, 1.73f}, {0.011f, 0.18f, 0.145f}, glass, vehicle, Surface::VehicleGlass);

        const glm::mat4 lamp = glm::rotate(glm::translate(vehicle, {side * 1.02f, 1.13f, 3.16f}),
            glm::radians(90.0f), glm::vec3(1, 0, 0));
        AddPart(cylinder, {0, 0, 0}, {0.170f, 0.065f, 0.170f}, chrome, lamp, Surface::Chrome);
        AddPart(cylinder, {0, 0.041f, 0}, {0.137f, 0.015f, 0.137f}, {0.91f, 0.89f, 0.74f}, lamp);
        AddPart(roundedBox, {side * 1.12f, 0.84f, 3.13f}, {0.115f, 0.070f, 0.022f},
            {0.83f, 0.37f, 0.055f}, vehicle, Surface::VehiclePaint);
        for (float z : {1.42f, 1.57f, 1.72f, 1.87f})
            AddPart(roundedBox, {side * 0.877f, 1.43f, z}, {0.014f, 0.20f, 0.025f}, rubber, vehicle, Surface::Tire);
    }
    AddBeam(world({-0.98f, 2.12f, 1.95f}), world({0.98f, 2.12f, 1.95f}), 0.040f, chrome, Surface::Chrome);
    AddBeam(world({-0.87f, 2.79f, 1.72f}), world({0.87f, 2.79f, 1.72f}), 0.032f, chrome, Surface::Chrome);
    AddBeam(world({0.0f, 2.12f, 1.96f}), world({0.0f, 2.79f, 1.73f}), 0.035f, ivory, Surface::VehiclePaint);
    for (float side : {-1.0f, 1.0f})
        AddBeam(world({side * 0.42f, 2.16f, 1.955f}), world({side * 0.16f, 2.31f, 1.902f}), 0.017f, rubber, Surface::Tire);

    const auto addTruckWheel = [this, &vehicle, &chrome, &rubber, &aluminium](float side, float z)
    {
        constexpr float radius = 0.56f;
        const glm::mat4 axle = glm::rotate(glm::translate(vehicle, {side * 1.11f, 0.60f, z}),
            glm::radians(side * 90.0f), glm::vec3(0, 1, 0));
        AddPart(tire, {0, 0, 0}, {radius, radius, 0.66f}, rubber, axle, Surface::Tire);
        const glm::mat4 hub = glm::rotate(axle, glm::radians(90.0f), glm::vec3(1, 0, 0));
        AddPart(cylinder, {0, 0.14f, 0}, {0.355f, 0.065f, 0.355f}, aluminium, hub, Surface::Chrome);
        AddPart(cylinder, {0, 0.179f, 0}, {0.293f, 0.018f, 0.293f}, chrome, hub, Surface::Chrome);
        for (int vent = 0; vent < 6; ++vent)
        {
            const float angle = glm::radians(vent * 60.0f);
            AddPart(cylinder, {std::cos(angle) * 0.225f, 0.195f, std::sin(angle) * 0.225f},
                {0.043f, 0.008f, 0.056f}, rubber, hub, Surface::Tire);
        }
        AddPart(sphere, {0, 0.205f, 0}, {0.115f, 0.039f, 0.115f}, chrome, hub, Surface::Chrome);
        for (int bolt = 0; bolt < 6; ++bolt)
        {
            const float angle = glm::radians(bolt * 60.0f);
            AddPart(cylinder, {std::cos(angle) * 0.080f, 0.238f, std::sin(angle) * 0.080f},
                {0.015f, 0.012f, 0.015f}, aluminium, hub, Surface::Chrome);
        }
    };
    for (float side : {-1.0f, 1.0f})
        for (float z : {2.14f, -1.65f, -2.80f})
            addTruckWheel(side, z);

    // The load body remains a practical metal box, with rolled corners and a separate frame.
    AddPart(roundedBox, {0.0f, 2.14f, -1.82f}, {2.68f, 2.67f, 4.69f},
        {0.66f, 0.69f, 0.65f}, vehicle, Surface::VehiclePaint);
    AddPart(roundedBox, {0.0f, 3.53f, -1.82f}, {2.76f, 0.14f, 4.78f}, aluminium, vehicle, Surface::Chrome);
    for (float side : {-1.0f, 1.0f})
    {
        AddPart(roundedBox, {side * 1.351f, 1.19f, -1.82f}, {0.031f, 0.61f, 4.44f}, red, vehicle, Surface::VehiclePaint);
        AddPart(roundedBox, {side * 1.373f, 1.53f, -1.82f}, {0.020f, 0.035f, 4.43f}, chrome, vehicle, Surface::Chrome);
        AddPart(roundedBox, {side * 1.351f, 2.43f, -1.82f}, {0.023f, 1.89f, 3.91f}, aluminium, vehicle, Surface::Chrome);
        AddBox({side * 1.369f, 2.43f, -1.82f}, {0.013f, 1.79f, 3.81f}, ivory, vehicle, Surface::Delivery);
        for (float z : {-3.98f, 0.35f})
            AddPart(roundedBox, {side * 1.35f, 2.16f, z}, {0.045f, 2.45f, 0.050f}, aluminium, vehicle, Surface::Chrome);
        for (float y : {0.99f, 1.48f, 3.31f})
            AddPart(roundedBox, {side * 1.36f, y, -1.82f}, {0.023f, 0.028f, 4.41f}, aluminium, vehicle, Surface::Chrome);
        for (float z : {-3.70f, -2.96f, -2.22f, -1.48f, -0.74f, 0.0f})
        {
            const glm::mat4 rivet = glm::rotate(glm::translate(vehicle, {side * 1.385f, 3.34f, z}),
                glm::radians(-side * 90.0f), glm::vec3(0, 0, 1));
            AddPart(cylinder, {0, 0, 0}, {0.017f, 0.012f, 0.017f}, chrome, rivet, Surface::Chrome);
        }
    }

    // Closed twin rear doors hide the former loading recess and boxes.
    for (float side : {-1.0f, 1.0f})
    {
        AddPart(roundedBox, {side * 0.59f, 2.15f, -4.182f}, {1.15f, 2.20f, 0.065f},
            {0.66f, 0.69f, 0.65f}, vehicle, Surface::VehiclePaint);
        AddPart(roundedBox, {side * 1.20f, 2.15f, -4.19f}, {0.10f, 2.31f, 0.065f},
            aluminium, vehicle, Surface::Chrome);
        AddPart(roundedBox, {side * 0.43f, 2.14f, -4.23f}, {0.026f, 1.91f, 0.030f},
            chrome, vehicle, Surface::Chrome);
        AddPart(roundedBox, {side * 0.43f, 1.63f, -4.26f}, {0.19f, 0.035f, 0.035f},
            chrome, vehicle, Surface::Chrome);
        for (float y : {1.39f, 2.10f, 2.87f})
            AddPart(roundedBox, {side * 1.17f, y, -4.24f}, {0.18f, 0.055f, 0.035f},
                chrome, vehicle, Surface::Chrome);
        for (float y = 1.18f; y < 3.15f; y += 0.24f)
            AddPart(roundedBox, {side * 0.59f, y, -4.222f}, {1.06f, 0.018f, 0.010f},
                aluminium * 0.76f, vehicle, Surface::Paint);
        AddPart(roundedBox, {side * 0.98f, 0.88f, -4.28f}, {0.18f, 0.092f, 0.024f},
            {0.59f, 0.055f, 0.029f}, vehicle, Surface::VehiclePaint);
    }
    AddPart(roundedBox, {0.0f, 2.15f, -4.231f}, {0.045f, 2.22f, 0.030f}, aluminium, vehicle, Surface::Chrome);
    AddPart(roundedBox, {0.0f, 0.97f, -4.20f}, {2.44f, 0.09f, 0.21f}, chrome, vehicle, Surface::Chrome);
}
