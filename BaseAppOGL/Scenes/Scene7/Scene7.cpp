#include "Scene7.h"


Scene7::Scene7()
{
	// Cria a câmera
	pCamera = NULL;
	pCamera = new CCamera(glm::vec3(0.0f, 3.0f, 10.0f));
	pCamera->MovementSpeed = 100.0f;

	// Cria o Timer
	pTimer = NULL;
	pTimer = new CTimer();
	pTimer->Init();

	// Cria gerenciador de impressão de texto na tela
	pText = NULL;
	pText = new CText();
	pText->InitTextManager();

	// Cria os shaders
	pShader = NULL;
	pShader = new CShader();
	pShader->LoadShader("Grid", "Scenes/Common/grid.vert", "Scenes/Common/grid.frag");
	pShader->LoadShader("Axis", "Scenes/Common/axis.vert", "Scenes/Common/axis.frag");
	pShader->LoadShader("Text2D", "Scenes/Common/Text2D.vert", "Scenes/Common/Text2D.frag");

	pShader->LoadShader("Assets", 
		"Scenes/Common/Assets.vert", 
		"Scenes/Common/Assets.frag");

	pShader->LoadShader("Light",
		"Scenes/Common/Light.vert",
		"Scenes/Common/Light.frag");

	pShader->LoadShader("Primitives",
		"Scenes/Scene1/Primitives.vert",
		"Scenes/Scene1/Primitives.frag");


	// Cria o gerenciador de grid e axis
	pGridAxis = NULL;
	pGridAxis = new CGridAxis(50.0f);

	// Cria o gerenciador de texturas
	pTextures = NULL;
	pTextures = new CTextures();
	pTextures->LoadTextureAnisotropic(0, "Scenes/Scene7/grass01.png");


	
	// Carregar os objetos
	pScene = NULL;
	pScene = new CAsset("Scenes/Scene7/Scene7.obj");

	pPrimitives = NULL;
	pPrimitives = new CAsset("Scenes/Scene7/Primitivas.obj");

	pHouse = NULL;
	pHouse = new CAsset("Scenes/Scene7/House.obj");


	// Ativa/Desativa a GUI
	bActiveGUI =  false;

	// Posição da fonte de luz
	posX = 0.0f;
	posY = 10.0f;
	posZ = 30.0f;

	// Cor da luz
	lightColor = glm::vec3(1.0f, 1.0f, 1.0f);

	// Velocidade de deslocamento da luz
	speed = 10.0f;

	// Intensidade da reflexão ambiente
	ambientStrenght = 0.5f;

	// Cria a Lâmpada
	pLamp.generateSphere();
	pLamp.setupBuffers();

	// Valor de transparência
	alpha = 1.0f;

	// Cria a grama
	pGrass.generatePlane(2.0f, 2.0f);
	pGrass.setupBuffers();


	// Parâmetros da neblina
	FogDensity = 0.05f;
	FogStart = 0.1f;
	FogEnd = 100.0f;
	FogColor = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
	FogMode = 0;
	FogEnable = true;

}

Scene7::~Scene7()
{
	pGrass.cleanup();


	pLamp.cleanup();

	if (pHouse)
	{
		delete pHouse;
		pHouse = NULL;
	}

	if (pPrimitives)
	{
		delete pPrimitives;
		pPrimitives = NULL;
	}

	if (pScene)
	{
		delete pScene;
		pScene = NULL;
	}

	if (pCamera)
	{
		delete pCamera;
		pCamera = NULL;
	}

	if (pShader)
	{
		delete pShader;
		pShader = NULL;
	}

	if (pGridAxis)
	{
		delete pGridAxis;
		pGridAxis = NULL;
	}

	if (pTextures)
	{
		delete pTextures;
		pTextures = NULL;
	}

	if (pTimer)
	{
		delete pTimer;
		pTimer = NULL;
	}

	if (pText)
	{
		delete pText;
		pText = NULL;
	}


}

void Scene7::DrawScene(float deltaTime)
{
	// Update timer (must be called at start of frame)
	pTimer->Update();

	// Clear the color and depth buffers
	if (FogEnable)
		glClearColor(FogColor.r, FogColor.g, FogColor.b, FogColor.a);
	else
		glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Create transformations 
	// Note: In a real application, you would typically calculate the projection and view matrices 
	// once per frame and pass them to your shaders
	glm::mat4 projection = glm::perspective(glm::radians(pCamera->Zoom), ASPECT_RATIO, Z_NEAR, Z_FAR);
	glm::mat4 view = pCamera->GetViewMatrix();
	glm::mat4 model = glm::mat4(1.0f);

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Draw the 3D grid
	//pShader->Use("Grid");
	//pShader->SetMat4("uProj", projection);
	//pShader->SetMat4("uView", view);
	//pShader->SetFloat("uGridSpacing", 1.0f);
	//pShader->SetFloat("uFadeStart", 18.f);
	//pShader->SetFloat("uFadeEnd", 100.f);
	//pShader->SetVec3("uCamPos", pCamera->Position);
	//pGridAxis->DrawGrid();
	// Draw the 3D axis
	pShader->Use("Axis");
	pShader->SetMat4("uProj", projection);
	pShader->SetMat4("uView", view);
	pGridAxis->DrawAxis();

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//                               DESENHA OS OBJETOS DA CENA (INÍCIO)
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Desenha a Lâmpada
	pShader->Use("Primitives");
	pShader->SetMat4("projection", projection);
	pShader->SetMat4("view", view);
	model = glm::mat4(1.0);
	model = glm::translate(model, glm::vec3(posX, posY, posZ));
	pShader->SetMat4("model", model);
	pLamp.render();


	// Desenha objetos com textura e iluminação
	pShader->Use("Light");
	pShader->SetMat4("projection", projection);
	pShader->SetMat4("view", view);

	// Passa a posição da fonte de luz
	pShader->SetVec3("lightPos", glm::vec3(posX, posY, posZ));

	// Passa a posição da câmera (observador da cena)
	pShader->SetVec3("viewPos", pCamera->Position);

	// Passa a cor da Luz
	pShader->SetVec3("lightColor", lightColor);

	// Passa a intensidade de reflexão da luz ambiente
	pShader->SetFloat("ambientStrenght", ambientStrenght);

	
	// Passa os parâmetros da neblina
	// Passa a cor da neblina
	pShader->SetVec4("FogColor", FogColor);

	// Passa a densidade da neblina
	pShader->SetFloat("FogDensity", FogDensity);

	// Passa o início da neblina
	pShader->SetFloat("FogStart", FogStart);

	// Passa o fim da neblina
	pShader->SetFloat("FogEnd", FogEnd);

	// Passa a posição da câmera
	pShader->SetVec3("CameraPos", pCamera->Position);

	// Passa o modo da neblina
	pShader->SetInt("FogMode", FogMode);

	// Passa se a neblina está habilitada ou não
	pShader->SetBool("FogEnable", FogEnable);
	




	// Cena
	model = glm::mat4(1.0);
	model = glm::translate(model, glm::vec3(0.0, 0.0, 0.0));
	pShader->SetMat4("model", model);
	pShader->SetFloat("alpha", 1.0f);
	pScene->Draw(pShader->GetProgram("Light"));

	// Primitivas
	model = glm::mat4(1.0);
	model = glm::translate(model, glm::vec3(15.0, 0.0, 0.0));
	pShader->SetMat4("model", model);
	pShader->SetFloat("alpha", 1.0f);
	pPrimitives->Draw(pShader->GetProgram("Light"));

	// Grama
	model = glm::mat4(1.0);
	model = glm::translate(model, glm::vec3(0.0, 1.0, 20.0));
	model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	pShader->SetMat4("model", model);

	glActiveTexture(GL_TEXTURE0);
	pTextures->ApplyTexture(0);
	pShader->SetInt("Texture0", 0);

	pGrass.render();


	/// Assets com transparência ///
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	// House
	model = glm::mat4(1.0);
	model = glm::translate(model, glm::vec3(-15.0, 0.0, 0.0));
	pShader->SetMat4("model", model);
	
	// Set alpha value for transparency
	pShader->SetFloat("alpha", alpha); 
	
	// Draw the house with transparency
	pHouse->Draw(pShader->GetProgram("Light"));
	
	glDisable(GL_BLEND);









	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//                               DESENHA OS OBJETOS DA CENA (FIM)
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Draw text on the screen (disable depth test and use orthographic projection)
	glDisable(GL_DEPTH_TEST);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	projection = glm::ortho(0.0f, static_cast<float>(SCR_WIDTH), 0.0f, static_cast<float>(SCR_HEIGHT));

	pShader->Use("Text2D");
	pShader->SetMat4("projection", projection);
	pShader->SetVec3("textColor", 1.0f, 1.0f, 1.0f);

	pText->RenderText(std::format("CamPosition\tx: {0:.2f} \t y: {1:.2f} \t z: {2:.2f}", pCamera->Position.x, pCamera->Position.y, pCamera->Position.z), 5.0f, 100.0f, 0.3f);
	pText->RenderText(std::format("CamFoward\tx: {0:.2f} \t y: {1:.2f} \t z: {2:.2f}", pCamera->Front.x, pCamera->Front.y, pCamera->Front.z), 5.0f, 80.0f, 0.3f);

	pText->RenderText(std::format("FPS: {0:.0f}\tDeltatime: {1:.2f}ms\tTimer: {2:.2f}s", pTimer->GetFPS(), pTimer->GetDeltaTime(), pTimer->GetTime() / 1000), 5.0f, 10.0f, 0.3f);
	glEnable(GL_DEPTH_TEST);
}


void Scene7::ProcessSceneInput(GLFWwindow* window, float deltaTime)
{
	InputManager& input = InputManager::GetInstance();

	if (input.IsKeyJustPressed(GLFW_KEY_ENTER))
	{
		bActiveGUI = !bActiveGUI;
		input.SetCursorMode(bActiveGUI ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);

		std::cout << "GUI mode: '" << bActiveGUI << "'" << std::endl;
	}

	
	if (bActiveGUI == false)
	{
		// Camera movement
		if (input.IsKeyPressed(GLFW_KEY_W))
			pCamera->ProcessKeyboard(FORWARD, deltaTime);
		if (input.IsKeyPressed(GLFW_KEY_S))
			pCamera->ProcessKeyboard(BACKWARD, deltaTime);
		if (input.IsKeyPressed(GLFW_KEY_A))
			pCamera->ProcessKeyboard(LEFT, deltaTime);
		if (input.IsKeyPressed(GLFW_KEY_D))
			pCamera->ProcessKeyboard(RIGHT, deltaTime);
		if (input.IsKeyPressed(GLFW_KEY_E))
			pCamera->ProcessKeyboard(UP, deltaTime);
		if (input.IsKeyPressed(GLFW_KEY_Q))
			pCamera->ProcessKeyboard(DOWN, deltaTime);

		
		if (input.IsKeyPressed(GLFW_KEY_UP))
			posZ -= speed * deltaTime;
		if (input.IsKeyPressed(GLFW_KEY_DOWN))
			posZ += speed * deltaTime;
		if (input.IsKeyPressed(GLFW_KEY_LEFT))
			posX -= speed * deltaTime;
		if (input.IsKeyPressed(GLFW_KEY_RIGHT))
			posX += speed * deltaTime;
		if (input.IsKeyPressed(GLFW_KEY_PAGE_DOWN))
			posY -= speed * deltaTime;
		if (input.IsKeyPressed(GLFW_KEY_PAGE_UP))
			posY += speed * deltaTime;







		// Increase/decrease movement speed
		if (input.IsKeyPressed(GLFW_KEY_LEFT_SHIFT))
			pCamera->MovementSpeed = SPEED_MULTIPLIER;
		else
			pCamera->MovementSpeed = SPEED;

		// Process mouse movement
		glm::vec2 mouseDelta = input.GetMouseDelta();
		if (mouseDelta.x != 0.0f || mouseDelta.y != 0.0f)
		{
			pCamera->ProcessMouseMovement(mouseDelta.x, mouseDelta.y);
		}

		// Process scroll
		float scrollOffset = input.GetScrollOffset();
		if (scrollOffset != 0.0f)
		{
			pCamera->ProcessMouseScroll(scrollOffset);
		}
	}
	else
	{
		ShowGUI(); // Show Graphics User Interface
	}
}

void Scene7::ShowGUI()
{
	ImGuiColorEditFlags flags = ImGuiColorEditFlags_Float;

	ImGui::Begin("Light Options:");
	ImGui::SliderFloat("Ambient Light", &ambientStrenght, 0.0f, 1.0f, "%.2f");
	ImGui::ColorEdit3("Cor da Luz", (float*)&lightColor, flags);
	ImGui::End();

	ImGui::Begin("Blend Options:");
	ImGui::SliderFloat("Alpha", &alpha, 0.0f, 1.0f, "%.2f");
	ImGui::End();

	ImGui::Begin("Fog Options");
	ImGui::Text("Fog Mode:");
	ImGui::RadioButton("Linear", &FogMode, 0); ImGui::SameLine();
	ImGui::RadioButton("Exponential", &FogMode, 1); ImGui::SameLine();
	ImGui::RadioButton("Quadratic", &FogMode, 2); ImGui::NewLine();
	ImGui::ColorEdit4("Fog Color", (float*)&FogColor, flags);
	ImGui::SliderFloat("Fog Density", &FogDensity, 0.0f, 1.0f, "%.3f");
	ImGui::SliderFloat("Fog Start", &FogStart, 0.0f, 10.0f, "%.2f");
	ImGui::SliderFloat("Fog End", &FogEnd, 0.0f, 100.0f, "%.2f");
	ImGui::Checkbox("Fog Enabled", &FogEnable);
	ImGui::End();


}