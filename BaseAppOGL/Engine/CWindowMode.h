#pragma once

#include <GLFW/glfw3.h>
#include <algorithm>

// Switching the monitor of the existing window preserves its OpenGL context.
class CWindowMode
{
public:
    bool Toggle(GLFWwindow* window)
    {
        if (!window)
            return false;

        if (glfwGetWindowMonitor(window))
        {
            glfwSetWindowMonitor(window, nullptr, windowX, windowY,
                windowWidth, windowHeight, GLFW_DONT_CARE);
            if (glfwGetWindowMonitor(window))
                return false;
            if (wasMaximized)
                glfwMaximizeWindow(window);
            return true;
        }

        GLFWmonitor* monitor = SelectMonitor(window);
        const GLFWvidmode* mode = monitor ? glfwGetVideoMode(monitor) : nullptr;
        if (!mode || mode->width <= 0 || mode->height <= 0)
            return false;

        wasMaximized = glfwGetWindowAttrib(window, GLFW_MAXIMIZED) == GLFW_TRUE;
        // A maximized window reports the monitor-sized rectangle. Restore it
        // briefly to save its actual normal placement before entering fullscreen.
        if (wasMaximized)
            glfwRestoreWindow(window);
        glfwGetWindowPos(window, &windowX, &windowY);
        glfwGetWindowSize(window, &windowWidth, &windowHeight);
        windowWidth = std::max(windowWidth, 1);
        windowHeight = std::max(windowHeight, 1);

        glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height,
            mode->refreshRate);
        if (glfwGetWindowMonitor(window) == monitor)
            return true;

        // Leave the previous window usable if a display-mode change fails.
        glfwSetWindowMonitor(window, nullptr, windowX, windowY,
            windowWidth, windowHeight, GLFW_DONT_CARE);
        if (wasMaximized)
            glfwMaximizeWindow(window);
        return false;
    }

    bool IsFullscreen(GLFWwindow* window) const
    {
        return window && glfwGetWindowMonitor(window) != nullptr;
    }

private:
    static GLFWmonitor* SelectMonitor(GLFWwindow* window)
    {
        int x = 0, y = 0, width = 0, height = 0;
        glfwGetWindowPos(window, &x, &y);
        glfwGetWindowSize(window, &width, &height);

        GLFWmonitor* primary = glfwGetPrimaryMonitor();
        GLFWmonitor* selected = nullptr;
        long long largestOverlap = -1;
        int count = 0;
        GLFWmonitor** monitors = glfwGetMonitors(&count);
        for (int index = 0; monitors && index < count; ++index)
        {
            GLFWmonitor* candidate = monitors[index];
            const GLFWvidmode* mode = candidate ? glfwGetVideoMode(candidate) : nullptr;
            if (!mode || mode->width <= 0 || mode->height <= 0)
                continue;

            int monitorX = 0, monitorY = 0;
            glfwGetMonitorPos(candidate, &monitorX, &monitorY);
            const long long overlapWidth = std::max(0LL,
                std::min(static_cast<long long>(x) + width,
                    static_cast<long long>(monitorX) + mode->width)
                - std::max(static_cast<long long>(x), static_cast<long long>(monitorX)));
            const long long overlapHeight = std::max(0LL,
                std::min(static_cast<long long>(y) + height,
                    static_cast<long long>(monitorY) + mode->height)
                - std::max(static_cast<long long>(y), static_cast<long long>(monitorY)));
            const long long overlap = overlapWidth * overlapHeight;
            if (overlap > largestOverlap || (overlap == largestOverlap && candidate == primary))
            {
                selected = candidate;
                largestOverlap = overlap;
            }
        }
        return selected ? selected : primary;
    }

    int windowX = 100;
    int windowY = 100;
    int windowWidth = 1280;
    int windowHeight = 720;
    bool wasMaximized = false;
};
