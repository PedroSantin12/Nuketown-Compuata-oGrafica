#pragma once

#include "CSceneBase.h"
#include "Scenes/Nuketown/SceneNuketown.h"

class CSceneManager : public CSceneBase
{
public:
	void DrawScene(float deltaTime) override;
	void ProcessSceneInput(GLFWwindow* window, float deltaTime) override;

private:
	SceneNuketown scene;
	bool bWireframeMode = false;
	bool bVsyncEnabled = true;
};
