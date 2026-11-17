#include "CSceneManager.h"

void CSceneManager::DrawScene(float deltaTime)
{
	scene.DrawScene(deltaTime);
}

void CSceneManager::ProcessSceneInput(GLFWwindow* window, float deltaTime)
{
	const InputManager& input = InputManager::GetInstance();

	if (input.IsKeyJustPressed(GLFW_KEY_ESCAPE))
		glfwSetWindowShouldClose(window, true);

	if (input.IsKeyJustPressed(GLFW_KEY_V))
	{
		bVsyncEnabled = !bVsyncEnabled;
		glfwSwapInterval(bVsyncEnabled);
		std::cout << "Vsync: '" << bVsyncEnabled << "'" << std::endl;
	}

	if (input.IsKeyJustPressed(GLFW_KEY_CAPS_LOCK))
	{
		bWireframeMode = !bWireframeMode;
		std::cout << "Wireframe: '" << bWireframeMode << "'" << std::endl;
	}

	glPolygonMode(GL_FRONT_AND_BACK, bWireframeMode ? GL_LINE : GL_FILL);
	scene.ProcessSceneInput(window, deltaTime);
}
