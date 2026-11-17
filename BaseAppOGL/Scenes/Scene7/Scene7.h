#pragma once
#include "Engine/CSceneBase.h"
#include "Engine/CCamera.h"
#include "Engine/CGridAxis.h"
#include "Engine/CShader.h"
#include "Engine/CTextures.h"
#include "Engine/CTimer.h"
#include "Engine/CText.h"
#include "Engine/CPrimitives.h"
#include "Engine/CAsset.h"


class Scene7 : public CSceneBase
{
public:
		Scene7();
		~Scene7();

		virtual void DrawScene(float deltaTime);
        virtual void ProcessSceneInput(GLFWwindow* window, float deltaTime);

		void ShowGUI();
private:

	// Camera
	CCamera* pCamera;

	// Axis
	CGridAxis* pGridAxis;

	// Shader
	CShader* pShader;

	// Textures
	CTextures* pTextures;

	// Timer
	CTimer* pTimer;			

	// Texto
	CText* pText;			

	// Cena
	CAsset* pScene;

	// Primitivas
	CAsset* pPrimitives;

	// House
	CAsset* pHouse;

	// Ativa/Desativa a GUI
	bool bActiveGUI;

	// Posição da fonte de luz
	float posX, posY, posZ;

	// Velocidade de deslocamento da fonte de luz
	float speed;
	
	// Intensidade da reflexão ambiente
	float ambientStrenght;

	// Lâmpada
	CPrimitives pLamp;

	// Cor da luz
	glm::vec3 lightColor;

	// Valor de transparência
	float alpha;

	// Grama
	CPrimitives pGrass;

	// Parâmetros da neblina
	float FogDensity;
	float FogStart;
	float FogEnd;
	glm::vec4 FogColor;
	glm::vec3 CameraPos;
	int FogMode;
	bool FogEnable;

};

