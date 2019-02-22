////////////////////////////////////////////////////////////////////////////////
// Filename: graphicsclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "graphicsclass.h"


GraphicsClass::GraphicsClass()
{
	m_D3D = 0;
	m_Camera = 0;
	m_Model = 0;
}


GraphicsClass::GraphicsClass(const GraphicsClass& other)
{
}


GraphicsClass::~GraphicsClass()
{
}


bool GraphicsClass::Initialize(int screenWidth, int screenHeight, HWND hwnd)
{
	bool result;

	//Save screen WIDTH x HEIGHT for internal usage
	m_screenWidth = screenWidth;
	m_screenHeight = screenHeight;

	// Create the Direct3D object.
	m_D3D = new D3DClass;
	if(!m_D3D)
		return false;

	// Initialize the Direct3D object.
	result = m_D3D->Initialize(screenWidth, screenHeight, VSYNC_ENABLED, hwnd, FULL_SCREEN, SCREEN_DEPTH, SCREEN_NEAR);
	if(!result)
		return false;

	// Create the camera object.
	m_Camera = new CameraClass;
	if(!m_Camera)
		return false;

	// Set the initial position of the camera.
	m_Camera->SetPosition(0.0f, 0.0f, -2.0f);
	
	// Create the model object.
	m_Model = new ModelClass;
	if(!m_Model)
		return false;

	//Initialize the model object.
	result = m_Model->Initialize(m_D3D->GetDevice(), "sphere.obj");
	if(!result)
		return false;

	//Create mouse container
	m_mouse = new MouseClassContainer;

	if (!(m_pbrShader = new ShaderPBRClass))
		return false;

	//SCENE LIGHTING
	m_directionalLight = new LightClass;
	m_directionalLight->SetLookAt(0, 0, 0);
	m_directionalLight->SetPosition(-5, 10, 0);
	m_directionalLight->GenerateViewMatrix();
	m_directionalLight->GenerateProjectionMatrix(SCREEN_DEPTH, SCREEN_NEAR);

#pragma region PBR Shader loading
	//Create input format for vertex data
	std::vector <LPCSTR> names;
	names.push_back("position");
	names.push_back("texcoord");
	names.push_back("normal");
	names.push_back("tangent");
	names.push_back("binormal");
	std::vector <DXGI_FORMAT> formats;
	formats.push_back(DXGI_FORMAT_R32G32B32_FLOAT);
	formats.push_back(DXGI_FORMAT_R32G32_FLOAT);
	formats.push_back(DXGI_FORMAT_R32G32B32_FLOAT);
	formats.push_back(DXGI_FORMAT_R32G32B32_FLOAT);
	formats.push_back(DXGI_FORMAT_R32G32B32_FLOAT);
	BaseShaderClass::vertexInputType input(names, formats);

	if (!m_pbrShader->Initialize(m_D3D->GetDevice(), hwnd, L"pbr_base.vs", L"pbr_base.ps", input))
		return false;

	//Load textures for PBR shader
	if (!m_pbrShader->LoadTexture(m_D3D->GetDevice(), L"Metal_006_Base_Color.dds", m_pbrShader->m_diffuseTexture, m_pbrShader->m_diffuseTextureView))
		return false;
	if (!m_pbrShader->LoadTexture(m_D3D->GetDevice(), L"Metal_006_Normal.dds", m_pbrShader->m_normalTexture, m_pbrShader->m_normalTextureView))
		return false;
	if (!m_pbrShader->LoadTexture(m_D3D->GetDevice(), L"Metal_006_Roughness.dds", m_pbrShader->m_roughnessTexture, m_pbrShader->m_roughnessTextureView))
		return false;
	if (!m_pbrShader->LoadTexture(m_D3D->GetDevice(), L"Metal_006_Metallic.dds", m_pbrShader->m_metalnessTexture, m_pbrShader->m_metalnessTextureView))
		return false;

	//Direction + strength (w)
	m_pbrShader->m_lightDirection = XMFLOAT4(-1.0f, 0.0, -1.0f, 1.2f);
#pragma endregion

#pragma region Creating UI
	//Debug window background
	m_debugBackground = new UIBackground;
	if (!m_debugBackground->Initialize(m_D3D, ModelClass::ShapeSize::RECTANGLE, -0.95f, -0.5f, 0.9f, -0.8f))
		return false;
	m_debugBackground->ChangeColor(102.0f/255.0f, 163.0f/255.0f, 1.0f, 0.4f);

	//Create TextEngine which handles and render all text at once
	m_textEngine = new TextEngine;
	m_textEngine->Initialize(m_D3D->GetDevice(), L"Fonts/font.spritefont");
	m_textEngine->WriteText(m_D3D->GetDeviceContext(), m_screenWidth, m_screenHeight, -0.75f, 0.85f, "Debug Menu", 0.5f, TextEngine::Align::CENTER);

	//Roughness slider
	m_roughnessSlider = new UISlider;
	if (!m_roughnessSlider->Initialize(m_D3D, -0.73f, -0.52f, 0.6f, 0.05f))
		return false;

	AddText(-0.94f, 0.625f, "Roughness:", 0.35f);

	m_roughnessSlider->ChangeColor(226.0f/255.0f, 211.0f/255.0f, 90.0f/255.0f, 1.0f);
	m_roughnessSlider->CreateTextArea(AddText(-0.79f, 0.625f, "0.00", 0.35f, TextEngine::Align::LEFT));
	m_roughnessSlider->EventOnChangeValue = [=](float roughness) { m_pbrShader->SetRoughness(roughness); };
	
	//Metalness slider
	m_metalnessSlider = new UISlider;
	if (!m_metalnessSlider->Initialize(m_D3D, -0.73f, -0.52f, 0.45f, 0.05f))
		return false;
	
	AddText(-0.94f, 0.475f, "Metalness:", 0.35f);

	m_metalnessSlider->ChangeColor(226.0f / 255.0f, 211.0f / 255.0f, 90.0f / 255.0f, 1.0f);
	m_metalnessSlider->CreateTextArea(AddText(-0.79f, 0.475f, "0.00", 0.35f, TextEngine::Align::LEFT));
	m_metalnessSlider->EventOnChangeValue = [=](float metalness) { m_pbrShader->SetMetalness(metalness); };

	//Texture preview - roughness
	m_texturePreviewRoughness = new UITexturePreview;
	m_texturePreviewRoughness->Initialize(m_D3D, -0.58f, 0.255f, 0.1f, m_pbrShader->m_roughnessTexture, m_pbrShader->m_roughnessTextureView);
	AddText(-0.94f, 0.325f, "Roughness:", 0.35f);

	//Texture preview - metalness
	m_texturePreviewMetalness = new UITexturePreview;
	m_texturePreviewMetalness->Initialize(m_D3D, -0.58f, 0.000f, 0.1f, m_pbrShader->m_metalnessTexture, m_pbrShader->m_metalnessTextureView);
	AddText(-0.94f, 0.1f, "Metalness:", 0.35f);

	//Texture preview - normal map
	m_texturePreviewNormal = new UITexturePreview;
	m_texturePreviewNormal->Initialize(m_D3D, -0.58f, -0.255f, 0.1f, m_pbrShader->m_normalTexture, m_pbrShader->m_normalTextureView);
	AddText(-0.94f, -0.125f, "Normal:", 0.35f);

	//Texture preview - albedo
	m_texturePreviewAlbedo = new UITexturePreview;
	m_texturePreviewAlbedo->Initialize(m_D3D, -0.58f, -0.510f, 0.1f, m_pbrShader->m_diffuseTexture, m_pbrShader->m_diffuseTextureView);
	AddText(-0.94f, -0.350f, "Albedo:", 0.35f);

#pragma endregion

	if (BLUR_BILINEAR)
	{
#pragma region Blur bilinear screenspace
		float textureWidth = screenHeight;
		float textureHeight = screenHeight;
		//RENDER SCENE TO TEXTURE
		if (!(m_renderTexture = new RenderTextureClass))
			return false;
		if (!(m_renderTexture->Initialize(m_D3D->GetDevice(), screenWidth, screenHeight)))
			return false;

		//m_renderTexture->LoadTexture(m_D3D->GetDevice(), L"seafloor.dds", m_renderTexture->GetShaderResource(), m_renderTexture->GetShaderResourceView());

		if (!(m_renderTexturePreview = new UITexture))
			return false;
		if (!(m_renderTexturePreview->Initialize(m_D3D, ModelClass::ShapeSize::RECTANGLE, -1.0f, 1.0f, 1.0f, -1.0f)))
			return false;

		m_renderTexturePreview->BindTexture(m_renderTexture->GetShaderResourceView());

		//Texture downsampled
		if (!(m_renderTextureDownsampled = new RenderTextureClass))
			return false;
		if (!(m_renderTextureDownsampled->Initialize(m_D3D->GetDevice(), screenWidth / 2, screenHeight / 2)))
			return false;

		//Texture upscaled
		if (!(m_renderTextureUpscaled = new RenderTextureClass))
			return false;
		if (!(m_renderTextureUpscaled->Initialize(m_D3D->GetDevice(), screenWidth, screenHeight)))
			return false;

		//Texture horizontal blur
		if (!(m_renderTextureHorizontalBlur = new RenderTextureClass))
			return false;
		if (!(m_renderTextureHorizontalBlur->Initialize(m_D3D->GetDevice(), screenWidth / 2, screenHeight / 2)))
			return false;

		//Texture vertical blur
		if (!(m_renderTextureVerticalBlur = new RenderTextureClass))
			return false;
		if (!(m_renderTextureVerticalBlur->Initialize(m_D3D->GetDevice(), screenWidth / 2, screenHeight / 2)))
			return false;
#pragma endregion
	}
	//Blur shader - vertical and horizontal
	m_blurShaderHorizontal = new BlurShaderClass;
	if (!m_blurShaderHorizontal->Initialize(m_D3D->GetDevice(), *m_D3D->GetHWND(), L"blurHorizontal.vs", L"blurHorizontal.ps", input))
		return false;

	m_blurShaderVertical = new BlurShaderClass;
	if (!m_blurShaderVertical->Initialize(m_D3D->GetDevice(), *m_D3D->GetHWND(), L"blurVertical.vs", L"blurVertical.ps", input))
		return false;

	m_convoluteQuadModel = new ModelClass;
	m_convoluteQuadModel->Initialize(m_D3D->GetDevice(), ModelClass::ShapeSize::RECTANGLE, -1.0f, 1.0f, 1.0f, -1.0f);

	m_groundQuadModel = new ModelClass;
	m_groundQuadModel->Initialize(m_D3D->GetDevice(), ModelClass::ShapeSize::RECTANGLE, -50.0f, 50.0f, 50.0f, -50.0f);

	m_shadowQuadModel = new ModelClass;
	m_shadowQuadModel->Initialize(m_D3D->GetDevice(), ModelClass::ShapeSize::RECTANGLE, -1.0f, 1.0f, 1.0f, -1.0f);
	
	m_colorShader = new SingleColorClass;
	if (!m_colorShader->Initialize(m_D3D->GetDevice(), *m_D3D->GetHWND(), L"singleColor.vs", L"singleColor.ps", input))
		return false;

	XMMATRIX tempMatrixView, tempMatrixProj;
	m_directionalLight->GetViewMatrix(tempMatrixView);
	m_directionalLight->GetProjectionMatrix(tempMatrixProj);
	m_colorShader->SetLightPosition(m_directionalLight->GetPosition());
	m_colorShader->SetLightViewProjection(tempMatrixView, tempMatrixProj);

	m_singleColorShader = new SingleColorClass;
	if (!m_singleColorShader->Initialize(m_D3D->GetDevice(), *m_D3D->GetHWND(), L"singleColorNoShadow.vs", L"singleColorNoShadow.ps", input))
		return false;

	m_singleColorShader->SetLightPosition(m_directionalLight->GetPosition());
	m_singleColorShader->SetLightViewProjection(tempMatrixView, tempMatrixProj);


	ifstream skyboxFile;
	skyboxFile.open("Skyboxes/cubemap.dds");
	if (skyboxFile.fail())
	{
		system("texassemble cube -w 2048 -h 2048 -f R8G8B8A8_UNORM -o Skyboxes/cubemap.dds Skyboxes/posx.bmp Skyboxes/negx.bmp Skyboxes/posy.bmp Skyboxes/negy.bmp Skyboxes/posz.bmp Skyboxes/negz.bmp");
		skyboxFile.open("Skyboxes/cubemap.dds");
		if (skyboxFile.fail())
			return true;
	}
	//LOAD SKYBOX
	if (!(m_skyboxShader = new SkyboxShaderClass))
		return false;
	if (!m_skyboxShader->Initialize(m_D3D->GetDevice(), *m_D3D->GetHWND(), L"skybox.vs", L"skybox.ps", input))
		return false;

	m_skyboxShader->LoadTexture(m_D3D->GetDevice(), L"Skyboxes/cubemap.dds", m_skyboxShader->m_skyboxTexture, m_skyboxShader->m_skyboxTextureView);
	
	//RENDER SCENE TO TEXTURE
	if (!(m_renderTexture = new RenderTextureClass))
		return false;
	if (!(m_renderTexture->Initialize(m_D3D->GetDevice(), CONVOLUTION_DIFFUSE_SIZE, CONVOLUTION_DIFFUSE_SIZE)))
		return false;

	if (!(m_renderTexturePreview = new UITexture))
		return false;
	if (!(m_renderTexturePreview->Initialize(m_D3D, ModelClass::ShapeSize::RECTANGLE, -1.0f, 1.0f, 1.0f, -1.0f)))
		return false;

	//Texture downsampled
	if (!(m_skyboxDownsampled = new RenderTextureClass))
		return false;
	if (!(m_skyboxDownsampled->Initialize(m_D3D->GetDevice(), CONVOLUTION_DIFFUSE_SIZE, CONVOLUTION_DIFFUSE_SIZE, RenderTextureClass::Scaling::NONE)))
		return false;

	if (!(m_convoluteShader = new SkyboxShaderClass))
		return false;
	if (!(m_convoluteShader->Initialize(m_D3D->GetDevice(), *m_D3D->GetHWND(), L"convolutionSkybox.vs", L"convolutionSkybox.ps", input)))
		return false;

	m_convoluteShader->LoadTexture(m_D3D->GetDevice(), L"Skyboxes/cubemap.dds", m_convoluteShader->m_skyboxTexture, m_convoluteShader->m_skyboxTextureView);
	m_convoluteShader->SetType(SkyboxShaderClass::SkyboxType::CONV_DIFFUSE);
	m_convoluteShader->SetUpVector(XMFLOAT3{ 0, 1, 0 });
	//DownsampleSkybox();
	ConvoluteShader(m_convoluteShader->m_skyboxTextureView, m_skyboxDownsampled);

	m_skyboxPreviewLeft = new UITexture;
	m_skyboxPreviewRight = new UITexture;
	m_skyboxPreviewUp = new UITexture;
	m_skyboxPreviewDown = new UITexture;
	m_skyboxPreviewForward = new UITexture;
	m_skyboxPreviewBack = new UITexture;

	m_skyboxPreviewLeft->Initialize(m_D3D, -.58125f, .0f, .33f, L"Skyboxes/conv_negx.dds");
	m_skyboxPreviewRight->Initialize(m_D3D, 0.081f, .0f, .33f, L"Skyboxes/conv_posx.dds");
	m_skyboxPreviewUp->Initialize(m_D3D, -.25f, .6625f, .33f, L"Skyboxes/conv_posy.dds");
	m_skyboxPreviewDown->Initialize(m_D3D, -.25f, -.6625f, .33f, L"Skyboxes/conv_negy.dds");
	m_skyboxPreviewForward->Initialize(m_D3D, 0.411f, .0f, .33f, L"Skyboxes/conv_posz.dds");
	m_skyboxPreviewBack->Initialize(m_D3D, -.25f, .0, .33f, L"Skyboxes/conv_negz.dds");

	m_skyboxTextureLeft = new RenderTextureClass;
	m_skyboxTextureRight = new RenderTextureClass;
	m_skyboxTextureUp = new RenderTextureClass;
	m_skyboxTextureDown = new RenderTextureClass;
	m_skyboxTextureForward = new RenderTextureClass;
	m_skyboxTextureBack = new RenderTextureClass;

	m_skyboxTextureLeft->LoadTexture(m_D3D->GetDevice(), L"Skyboxes/negx.bmp", m_skyboxTextureLeft->GetShaderResource(), m_skyboxTextureLeft->GetShaderResourceView(), false);
	m_skyboxTextureRight->LoadTexture(m_D3D->GetDevice(), L"Skyboxes/posx.bmp", m_skyboxTextureRight->GetShaderResource(), m_skyboxTextureRight->GetShaderResourceView(), false);
	m_skyboxTextureUp->LoadTexture(m_D3D->GetDevice(), L"Skyboxes/posy.bmp", m_skyboxTextureUp->GetShaderResource(), m_skyboxTextureUp->GetShaderResourceView(), false);
	m_skyboxTextureDown->LoadTexture(m_D3D->GetDevice(), L"Skyboxes/negy.bmp", m_skyboxTextureDown->GetShaderResource(), m_skyboxTextureDown->GetShaderResourceView(), false);
	m_skyboxTextureForward->LoadTexture(m_D3D->GetDevice(), L"Skyboxes/posz.bmp", m_skyboxTextureForward->GetShaderResource(), m_skyboxTextureForward->GetShaderResourceView(), false);
	m_skyboxTextureBack->LoadTexture(m_D3D->GetDevice(), L"Skyboxes/negz.bmp", m_skyboxTextureBack->GetShaderResource(), m_skyboxTextureBack->GetShaderResourceView(), false);

	//m_skyboxPreviewLeft->BindTexture(m_skyboxTextureLeft->GetShaderResourceView());
	//m_skyboxPreviewRight->BindTexture(m_skyboxTextureRight->GetShaderResourceView());
	//m_skyboxPreviewUp->BindTexture(m_skyboxTextureUp->GetShaderResourceView());
	//m_skyboxPreviewDown->BindTexture(m_skyboxTextureDown->GetShaderResourceView());
	//m_skyboxPreviewForward->BindTexture(m_skyboxTextureForward->GetShaderResourceView());
	//m_skyboxPreviewBack->BindTexture(m_skyboxTextureBack->GetShaderResourceView());

#pragma region SHADOW MAPPING

	//std::vector <LPCSTR> shadowMapNames;
	//names.push_back("position");
	//std::vector <DXGI_FORMAT> shadowMapFormats;
	//formats.push_back(DXGI_FORMAT_R32G32B32_FLOAT);
	//BaseShaderClass::vertexInputType shadowMapInput(shadowMapNames, shadowMapFormats);
	m_shadowMapShader = new ShadowMapClass;
	if (!m_shadowMapShader->Initialize(m_D3D->GetDevice(), *m_D3D->GetHWND(), L"shadowmap.vs", L"shadowmap.ps", input))
		return false;


	m_shadowMapTexture = new RenderTextureClass;
	m_shadowMapTexture->InitializeShadowMap(m_D3D->GetDevice(), 1280, 720);

#pragma endregion

	if (!m_pbrShader->LoadIrradianceMap(m_D3D->GetDevice(), L"Skyboxes/conv_cubemap.dds"))
		return false;

	//CreateShadowMap(m_shadowMapTexture);

	return true;
}


void GraphicsClass::Shutdown()
{

	// Release the model object.
	if(m_Model)
	{
		m_Model->Shutdown();
		delete m_Model;
		m_Model = 0;
	}

	// Release the camera object.
	if(m_Camera)
	{
		delete m_Camera;
		m_Camera = 0;
	}

	// Release the D3D object.
	if(m_D3D)
	{
		m_D3D->Shutdown();
		delete m_D3D;
		m_D3D = 0;
	}

	return;
}


bool GraphicsClass::Frame()
{
	bool result;


	// Render the graphics scene.
	//m_rotationY += 0.01f;
	if (m_rotationY >= 360.0f)
		m_rotationY = 0.0f;
	result = Render();
	if(!result)
	{
		return false;
	}

	return true;
}

void GraphicsClass::MoveCameraForward()
{
	m_Camera->AddPosition(0, 0.5f, 0);
}

void GraphicsClass::MoveCameraBackward()
{
	m_Camera->AddPosition(0, -0.5f, 0);
}

void GraphicsClass::MoveCameraLeft()
{
	m_Camera->AddPosition(-0.5f, 0, 0);
}

void GraphicsClass::MoveCameraRight()
{
	m_Camera->AddPosition(0.5f, 0, 0);
}

void GraphicsClass::MoveCameraUp()
{
	m_Camera->AddPosition(0, 0, 0.5f);
}

void GraphicsClass::MoveCameraDown()
{
	m_Camera->AddPosition(0, 0, -0.5f);
}

void GraphicsClass::RotateCamera(XMVECTOR rotation)
{
	m_Camera->SetRotation(m_Camera->GetRotation().x + rotation.m128_f32[0], m_Camera->GetRotation().y + rotation.m128_f32[1], m_Camera->GetRotation().z + rotation.m128_f32[2]);
}

void GraphicsClass::UpdateUI()
{
	if (m_mouse == nullptr)
		return;

	if (m_mouse->GetMouse()->GetLMBPressed())
	{
		if (m_roughnessSlider->IsChanging())
		{
			m_roughnessSlider->ChangeSliderValue(m_mouse->GetMouse());
		}
		else if (m_metalnessSlider->IsChanging())
		{
			m_metalnessSlider->ChangeSliderValue(m_mouse->GetMouse());
		}

		if (m_mouse->GetMouse()->isInputConsumed == true)
			return;

		if (m_roughnessSlider->MouseOnArea(m_mouse->GetMouse()))
		{
			m_mouse->GetMouse()->isInputConsumed = true;
			m_roughnessSlider->StartUsing();
			m_roughnessSlider->ChangeSliderValue(m_mouse->GetMouse());
		}
		else if (m_metalnessSlider->MouseOnArea(m_mouse->GetMouse()))
		{
			m_mouse->GetMouse()->isInputConsumed = true;
			m_metalnessSlider->StartUsing();
			m_metalnessSlider->ChangeSliderValue(m_mouse->GetMouse());
		}
		else if (m_texturePreviewRoughness->MouseOnArea(m_mouse->GetMouse()))
		{
			m_mouse->GetMouse()->isInputConsumed = true;
			m_texturePreviewRoughness->TextureChooseWindow(*m_D3D->GetHWND());
		}
		else if (m_texturePreviewMetalness->MouseOnArea(m_mouse->GetMouse()))
		{
			m_mouse->GetMouse()->isInputConsumed = true;
			m_texturePreviewMetalness->TextureChooseWindow(*m_D3D->GetHWND());
		}
		else if (m_texturePreviewNormal->MouseOnArea(m_mouse->GetMouse()))
		{
			m_mouse->GetMouse()->isInputConsumed = true;
			m_texturePreviewNormal->TextureChooseWindow(*m_D3D->GetHWND());
		}
		else if (m_texturePreviewAlbedo->MouseOnArea(m_mouse->GetMouse()))
		{
			m_mouse->GetMouse()->isInputConsumed = true;
			m_texturePreviewAlbedo->TextureChooseWindow(*m_D3D->GetHWND());
		}
	}
	else if (m_mouse->GetMouse()->GetRMBPressed())
	{
		if (m_mouse->GetMouse()->isInputConsumed == true)
			return;

		if (m_texturePreviewRoughness->MouseOnArea(m_mouse->GetMouse()))
		{
			m_mouse->GetMouse()->isInputConsumed = true;
			m_texturePreviewRoughness->DeleteTexture();
		}
		else if (m_texturePreviewMetalness->MouseOnArea(m_mouse->GetMouse()))
		{
			m_mouse->GetMouse()->isInputConsumed = true;
			m_texturePreviewMetalness->DeleteTexture();
		}
		else if (m_texturePreviewNormal->MouseOnArea(m_mouse->GetMouse()))
		{
			m_mouse->GetMouse()->isInputConsumed = true;
			m_texturePreviewNormal->DeleteTexture();
		}
		else if (m_texturePreviewAlbedo->MouseOnArea(m_mouse->GetMouse()))
		{
			m_mouse->GetMouse()->isInputConsumed = true;
			m_texturePreviewAlbedo->DeleteTexture();
		}
	}
	else
	{
		m_mouse->GetMouse()->isInputConsumed = false;
		if (m_roughnessSlider->IsChanging())
			m_roughnessSlider->EndUsing();
		if (m_metalnessSlider->IsChanging())
			m_metalnessSlider->EndUsing();
	}
}

void GraphicsClass::SetMouseRef(MouseClass * mouse)
{
	m_mouse->SetMouse(mouse);
	if (m_mouse->GetModel() == nullptr)
		m_mouse->InitializeMouse();
}

D3DClass * GraphicsClass::GetD3D()
{
	return m_D3D;
}

TextEngine::FontData * GraphicsClass::AddText(float && posX, float && posY, std::string&& text, float && scale, TextEngine::Align && align, XMVECTOR && color)
{
	return m_textEngine->WriteText(m_D3D->GetDeviceContext(), m_screenWidth, m_screenHeight, posX, posY, text, scale, align, color);
}

bool GraphicsClass::Render()
{
	XMMATRIX worldMatrix, viewMatrix, projectionMatrix;
	bool result;

	// Clear the buffers to begin the scene.
	m_D3D->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

	// Generate the view matrix based on the camera's position.
	m_Camera->Render();
	m_Camera->GetViewMatrix(viewMatrix);
	m_D3D->GetWorldMatrix(worldMatrix);
	m_D3D->GetProjectionMatrix(projectionMatrix);

	if (BLUR_BILINEAR)
	{
		if (!RenderSceneToTexture())
			return false;

		DownsampleTexture();
		BlurFilterScreenSpace(false);
		BlurFilterScreenSpace(true);
		UpscaleTexture();

		result = m_renderTexturePreview->Render(m_D3D->GetDeviceContext(), 0, worldMatrix, viewMatrix, projectionMatrix);
		if (!result)
			return false;
	}
	else
	{
		//CreateShadowMap(m_shadowMapTexture);

		//result = m_renderTexturePreview->Render(m_D3D->GetDeviceContext(), 0, worldMatrix, viewMatrix, projectionMatrix);
		//if (!result)
		//	return false;

	//STANDARD SCENE RENDERING
		//result = RenderScene();
		//if (!result)
		//	return false;

	//PREVIEW SKYBOX IN 6 FACES FORM
		m_skyboxPreviewRight->Render(m_D3D->GetDeviceContext(), m_groundQuadModel->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix);
		m_skyboxPreviewLeft->Render(m_D3D->GetDeviceContext(), m_groundQuadModel->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix);
		m_skyboxPreviewUp->Render(m_D3D->GetDeviceContext(), m_groundQuadModel->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix);
		m_skyboxPreviewDown->Render(m_D3D->GetDeviceContext(), m_groundQuadModel->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix);
		m_skyboxPreviewForward->Render(m_D3D->GetDeviceContext(), m_groundQuadModel->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix);
		m_skyboxPreviewBack->Render(m_D3D->GetDeviceContext(), m_groundQuadModel->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix);
	}

	if (ENABLE_DEBUG)
	{
		RenderDebugSettings();
	}
	// Present the rendered scene to the screen.
	m_D3D->EndScene();

	return true;
}

bool GraphicsClass::RenderSceneToTexture()
{
	m_renderTexture->SetRenderTarget(m_D3D->GetDeviceContext(), m_D3D->GetDepthStencilView());
	m_renderTexture->ClearRenderTarget(m_D3D->GetDeviceContext(), m_D3D->GetDepthStencilView(), 0.0f, 0.0f, 0.0f, 1.0f);

	m_renderTexturePreview->BindTexture(m_renderTexture->GetShaderResourceView());

	if (RenderScene() == false)
		return false;

	m_D3D->SetBackBufferRenderTarget();

	return true;
}

bool GraphicsClass::RenderScene()
{
	XMMATRIX worldMatrix, viewMatrix, projectionMatrix, lightViewMatrix, lightProjectionMatrix;
	bool result;

	//m_Camera->SetPosition(0.0f, 5.0f, -15.0f);
	m_Camera->SetPosition(0.0f, 0.0f, -2.0f);
	m_Camera->Render();

	//RENDER MAIN SCENE MODEL
	m_D3D->ChangeRasterizerCulling(D3D11_CULL_BACK);
	m_D3D->ChangeDepthStencilComparison(D3D11_COMPARISON_LESS_EQUAL);

	m_Camera->GetViewMatrix(viewMatrix);
	m_D3D->GetWorldMatrix(worldMatrix);
	m_D3D->GetProjectionMatrix(projectionMatrix);

	worldMatrix = DirectX::XMMatrixMultiply(worldMatrix, XMMatrixRotationX(45.4f));
	worldMatrix = DirectX::XMMatrixMultiply(worldMatrix, XMMatrixTranslation(0, -0.6f, 0));
	m_groundQuadModel->Render(m_D3D->GetDeviceContext());
	m_colorShader->m_shadowMapResourceView = m_shadowMapTexture->GetShaderResourceView();
	m_directionalLight->GetViewMatrix(lightViewMatrix);
	m_directionalLight->GetProjectionMatrix(lightProjectionMatrix);
	m_colorShader->SetLightViewProjection(lightViewMatrix, lightProjectionMatrix);
	//result = m_singleColorShader->Render(m_D3D->GetDeviceContext(), m_groundQuadModel->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix);
	//if (!result)
	//	return false;

	m_Model->Render(m_D3D->GetDeviceContext());
	m_pbrShader->m_cameraPosition = m_Camera->GetPosition();
	//XMVECTOR light_ = XMVector3Rotate(XMVECTOR{ -1.0f, 0.0, -1.0f, 0.0f }, XMVECTOR{ m_Camera->GetRotation().x / 3.14f, m_Camera->GetRotation().y / 3.14f, m_Camera->GetRotation().z / 3.14f, 0.0f });
	//m_pbrShader->m_lightDirection = XMFLOAT4(light_.m128_f32[0], 0.0, light_.m128_f32[2], 1.2f);

	m_Camera->GetViewMatrix(viewMatrix);
	m_D3D->GetWorldMatrix(worldMatrix);
	m_D3D->GetProjectionMatrix(projectionMatrix);
	worldMatrix = DirectX::XMMatrixMultiply(worldMatrix, DirectX::XMMatrixRotationY(m_Camera->GetRotation().y / 3.14f));
	worldMatrix = DirectX::XMMatrixMultiply(worldMatrix, DirectX::XMMatrixRotationX(m_Camera->GetRotation().x / 3.14f));
	//result = m_colorShader->Render(m_D3D->GetDeviceContext(), m_Model->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix);
	//if (!result)
	//	return false;
	result = m_pbrShader->Render(m_D3D->GetDeviceContext(), m_Model->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix);
	if (!result)
		return false;

	//for (int i = 0; i < 5; i++)
	//{
	//	m_Camera->GetViewMatrix(viewMatrix);
	//	m_D3D->GetWorldMatrix(worldMatrix);
	//	m_D3D->GetProjectionMatrix(projectionMatrix);

	//	worldMatrix = XMMatrixTranslation(i * 2.0f, 2.0f, 1.0f);
	//	m_Model->Render(m_D3D->GetDeviceContext());
	//	result = m_colorShader->Render(m_D3D->GetDeviceContext(), m_Model->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix);
	//	if (!result)
	//		return false;
	//}

	if (DRAW_SKYBOX)
	{
		if (RenderSkybox() == false)
			return false;
	}

	return true;
}

bool GraphicsClass::RenderDebugSettings()
{
	XMMATRIX worldMatrix, viewMatrix, projectionMatrix;
	m_D3D->TurnZBufferOff();

	//Get the world, view, and projection matrices from the camera and d3d objects.
	m_Camera->GetViewMatrix(viewMatrix);
	m_D3D->GetWorldMatrix(worldMatrix);
	m_D3D->GetProjectionMatrix(projectionMatrix);

	m_D3D->GetWorldMatrix(worldMatrix);
	//DRAW UI TRANSLUCENT OPTIONS
	m_D3D->EnableAlphaBlending();

	bool result = m_debugBackground->Render(m_D3D->GetDeviceContext(), m_Model->GetIndexCount(), worldMatrix * 0, viewMatrix, projectionMatrix);
	if (!result)
		return false;
	m_D3D->DisableAlphaBlending();

	//DRAW UI OPAQUE OPTIONS
	result = m_roughnessSlider->Render(m_D3D->GetDeviceContext(), m_Model->GetIndexCount(), worldMatrix * 0, viewMatrix, projectionMatrix);
	if (!result)
		return false;

	result = m_metalnessSlider->Render(m_D3D->GetDeviceContext(), m_Model->GetIndexCount(), worldMatrix * 0, viewMatrix, projectionMatrix);
	if (!result)
		return false;
	
	//DRAW UI INPUT
	m_texturePreviewRoughness->Render(m_D3D->GetDeviceContext(), m_Model->GetIndexCount(), worldMatrix * 0, viewMatrix, projectionMatrix);
	m_texturePreviewMetalness->Render(m_D3D->GetDeviceContext(), m_Model->GetIndexCount(), worldMatrix * 0, viewMatrix, projectionMatrix);
	m_texturePreviewNormal->Render(m_D3D->GetDeviceContext(), m_Model->GetIndexCount(), worldMatrix * 0, viewMatrix, projectionMatrix);
	m_texturePreviewAlbedo->Render(m_D3D->GetDeviceContext(), m_Model->GetIndexCount(), worldMatrix * 0, viewMatrix, projectionMatrix);

	m_textEngine->RenderText(m_D3D->GetDeviceContext(), m_screenWidth, m_screenHeight);

	////ALWAYS RENDER MOUSE AT THE VERY END
	m_D3D->TurnZBufferOff();

	result = m_mouse->Render(m_D3D->GetDeviceContext(), m_Model->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix);
	if (!result)
		return false;
	m_D3D->TurnZBufferOn();

	return true;
}

bool GraphicsClass::RenderSkybox()
{
	XMMATRIX worldMatrix, viewMatrix, projectionMatrix;
	//DRAW SKYBOX
	m_Camera->GetViewMatrix(viewMatrix);
	m_D3D->GetWorldMatrix(worldMatrix);
	m_D3D->GetProjectionMatrix(projectionMatrix);
	worldMatrix = DirectX::XMMatrixMultiply(worldMatrix, DirectX::XMMatrixRotationY(m_Camera->GetRotation().y / 3.14f));
	worldMatrix = DirectX::XMMatrixMultiply(worldMatrix, DirectX::XMMatrixRotationX(m_Camera->GetRotation().x / 3.14f));

	m_D3D->ChangeRasterizerCulling(D3D11_CULL_BACK);
	m_D3D->ChangeDepthStencilComparison(D3D11_COMPARISON_LESS_EQUAL);

	m_Model->Render(m_D3D->GetDeviceContext());
	m_skyboxShader->Render(m_D3D->GetDeviceContext(), m_Model->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix);

	m_D3D->ChangeRasterizerCulling(D3D11_CULL_BACK);
	m_D3D->ChangeDepthStencilComparison(D3D11_COMPARISON_LESS);

	return true;
}

bool GraphicsClass::DownsampleTexture()
{
	XMMATRIX worldMatrix, viewMatrix, orthoMatrix;
	bool result;

	m_renderTextureDownsampled->SetRenderTarget(m_D3D->GetDeviceContext(), m_D3D->GetDepthStencilView());
	m_renderTextureDownsampled->ClearRenderTarget(m_D3D->GetDeviceContext(), m_D3D->GetDepthStencilView(), 1.0f, 0.0f, 0.0f, 1.0f);

	m_Camera->Render();

	m_Camera->GetViewMatrix(viewMatrix);
	m_D3D->GetWorldMatrix(worldMatrix);
	m_renderTextureDownsampled->GetOrthoMatrix(orthoMatrix);

	m_D3D->TurnZBufferOff();

	result = m_renderTexturePreview->Render(m_D3D->GetDeviceContext(), m_Model->GetIndexCount(), worldMatrix, viewMatrix, orthoMatrix);
	if (!result)
		return false;

	m_D3D->TurnZBufferOn();

	m_D3D->SetBackBufferRenderTarget();
	m_D3D->ResetViewport();

	m_renderTexturePreview->BindTexture(m_renderTextureDownsampled->GetShaderResourceView());
	return true;
}

bool GraphicsClass::UpscaleTexture()
{
	XMMATRIX worldMatrix, viewMatrix, orthoMatrix;
	bool result;

	m_renderTextureUpscaled->SetRenderTarget(m_D3D->GetDeviceContext(), m_D3D->GetDepthStencilView());
	m_renderTextureUpscaled->ClearRenderTarget(m_D3D->GetDeviceContext(), m_D3D->GetDepthStencilView(), 0.0f, 0.0f, 0.0f, 1.0f);

	m_Camera->Render();

	m_Camera->GetViewMatrix(viewMatrix);
	m_D3D->GetWorldMatrix(worldMatrix);
	m_renderTextureUpscaled->GetOrthoMatrix(orthoMatrix);

	m_D3D->TurnZBufferOff();

	result = m_renderTexturePreview->Render(m_D3D->GetDeviceContext(), m_Model->GetIndexCount(), worldMatrix, viewMatrix, orthoMatrix);
	if (!result)
		return false;

	m_D3D->TurnZBufferOn();

	m_D3D->SetBackBufferRenderTarget();
	m_D3D->ResetViewport();

	m_renderTexturePreview->BindTexture(m_renderTextureUpscaled->GetShaderResourceView());
	return true;
}

bool GraphicsClass::BlurFilterScreenSpace(bool vertical)
{
	XMMATRIX worldMatrix, viewMatrix, orthoMatrix;
	bool result;

	if (vertical == false) //HORIZONTAL - 1st
	{
		m_renderTextureHorizontalBlur->SetRenderTarget(m_D3D->GetDeviceContext(), m_D3D->GetDepthStencilView());
		m_renderTextureHorizontalBlur->ClearRenderTarget(m_D3D->GetDeviceContext(), m_D3D->GetDepthStencilView(), 0.0f, 0.0f, 0.0f, 1.0f);

		m_Camera->Render();

		m_Camera->GetViewMatrix(viewMatrix);
		m_D3D->GetWorldMatrix(worldMatrix);
		m_renderTextureHorizontalBlur->GetOrthoMatrix(orthoMatrix);

		m_D3D->TurnZBufferOff();

		m_blurShaderHorizontal->SetTextureSize(m_screenWidth / 2);
		m_blurShaderHorizontal->SetTextureResourceView(m_renderTextureDownsampled->GetShaderResourceView());
		m_renderTexturePreview->GetModel()->Render(m_D3D->GetDeviceContext());
		result = m_blurShaderHorizontal->Render(m_D3D->GetDeviceContext(), m_Model->GetIndexCount(), worldMatrix, viewMatrix, orthoMatrix);
		if (!result)
			return false;

		m_D3D->TurnZBufferOn();

		m_D3D->SetBackBufferRenderTarget();
		m_D3D->ResetViewport();

		m_renderTexturePreview->BindTexture(m_renderTextureHorizontalBlur->GetShaderResourceView());
	}
	else //VERTICAL - 2nd
	{
		m_renderTextureVerticalBlur->SetRenderTarget(m_D3D->GetDeviceContext(), m_D3D->GetDepthStencilView());
		m_renderTextureVerticalBlur->ClearRenderTarget(m_D3D->GetDeviceContext(), m_D3D->GetDepthStencilView(), 0.0f, 0.0f, 0.0f, 1.0f);

		m_Camera->Render();

		m_Camera->GetViewMatrix(viewMatrix);
		m_D3D->GetWorldMatrix(worldMatrix);
		m_renderTextureVerticalBlur->GetOrthoMatrix(orthoMatrix);

		m_D3D->TurnZBufferOff();

		m_blurShaderVertical->SetTextureSize(m_screenHeight / 2);
		m_blurShaderVertical->SetTextureResourceView(m_renderTexturePreview->GetShaderResourceView());
		m_renderTexturePreview->GetModel()->Render(m_D3D->GetDeviceContext());
		result = m_blurShaderVertical->Render(m_D3D->GetDeviceContext(), m_Model->GetIndexCount(), worldMatrix, viewMatrix, orthoMatrix);
		if (!result)
			return false;

		m_D3D->TurnZBufferOn();

		m_D3D->SetBackBufferRenderTarget();
		m_D3D->ResetViewport();

		m_renderTexturePreview->BindTexture(m_renderTextureVerticalBlur->GetShaderResourceView());
	}

	return true;
}

bool GraphicsClass::DownsampleSkybox()
{
	//Texture horizontal blur
	if (!(m_skyboxBlurHorizontal = new RenderTextureClass))
		return false;
	if (!(m_skyboxBlurHorizontal->Initialize(m_D3D->GetDevice(), CONVOLUTION_DIFFUSE_SIZE, CONVOLUTION_DIFFUSE_SIZE, RenderTextureClass::Scaling::NONE)))
		return false;

	//Texture vertical blur
	if (!(m_skyboxBlurVertical = new RenderTextureClass))
		return false;
	if (!(m_skyboxBlurVertical->Initialize(m_D3D->GetDevice(), CONVOLUTION_DIFFUSE_SIZE, CONVOLUTION_DIFFUSE_SIZE, RenderTextureClass::Scaling::NONE)))
		return false;

	DownsampleSkyboxFace(L"Skyboxes/posx.jpg", L"Skyboxes/conv_posx.dds", false);
	//DownsampleSkyboxFace(L"Skyboxes/posy.jpg", L"Skyboxes/conv_posy.dds", false);
	//DownsampleSkyboxFace(L"Skyboxes/posz.jpg", L"Skyboxes/conv_posz.dds", false);
	//DownsampleSkyboxFace(L"Skyboxes/negx.jpg", L"Skyboxes/conv_negx.dds", false);
	//DownsampleSkyboxFace(L"Skyboxes/negy.jpg", L"Skyboxes/conv_negy.dds", false);
	//DownsampleSkyboxFace(L"Skyboxes/negz.jpg", L"Skyboxes/conv_negz.dds", false);

	ifstream skyboxFile;
	skyboxFile.open("Skyboxes/conv_cubemap.dds");
	if (skyboxFile.fail())
	{
		system("texassemble cube -w 256 -h 256 -f R8G8B8A8_UNORM -o Skyboxes/conv_cubemap.dds Skyboxes/conv_posx.dds Skyboxes/conv_negx.dds Skyboxes/conv_posy.dds Skyboxes/conv_negy.dds Skyboxes/conv_posz.dds Skyboxes/conv_negz.dds");
		skyboxFile.open("Skyboxes/conv_cubemap.dds");
		if (skyboxFile.fail())
			return true;
	}

	return true;
}

bool GraphicsClass::DownsampleSkyboxFace(const wchar_t * inputFilename, const wchar_t * outputFilename, bool isDDS)
{
	XMMATRIX worldMatrix, viewMatrix, orthoMatrix;
	bool result;

	m_renderTexture->LoadTexture(m_D3D->GetDevice(), inputFilename, m_renderTexture->GetShaderResource(), m_renderTexture->GetShaderResourceView(), isDDS);
	m_renderTexturePreview->BindTexture(m_renderTexture->GetShaderResourceView());

	m_skyboxDownsampled->SetRenderTarget(m_D3D->GetDeviceContext(), m_D3D->GetDepthStencilView());
	m_skyboxDownsampled->ClearRenderTarget(m_D3D->GetDeviceContext(), m_D3D->GetDepthStencilView(), 1.0f, 0.0f, 0.0f, 1.0f);

	m_Camera->Render();

	m_Camera->GetViewMatrix(viewMatrix);
	m_D3D->GetWorldMatrix(worldMatrix);
	m_skyboxDownsampled->GetOrthoMatrix(orthoMatrix);

	m_D3D->TurnZBufferOff();

	result = m_renderTexturePreview->Render(m_D3D->GetDeviceContext(), m_Model->GetIndexCount(), worldMatrix, viewMatrix, orthoMatrix);
	if (!result)
		return false;

	m_D3D->TurnZBufferOn();

	m_D3D->SetBackBufferRenderTarget();
	m_D3D->ResetViewport();

	m_renderTexturePreview->BindTexture(m_skyboxDownsampled->GetShaderResourceView());
	ConvoluteShader(m_skyboxDownsampled->GetShaderResourceView(), m_skyboxDownsampled);
	//for (int i = 0; i < 3; i++)
	//{
	//	BlurFilter(false, m_renderTexturePreview, m_skyboxBlurHorizontal, CONVOLUTION_DIFFUSE_SIZE);
	//	BlurFilter(true, m_skyboxBlurHorizontal, m_skyboxBlurVertical, CONVOLUTION_DIFFUSE_SIZE);
	//}
	
	SaveDDSTextureToFile(m_D3D->GetDeviceContext(), m_skyboxDownsampled->GetShaderResource(), outputFilename);

	return false;
}

bool GraphicsClass::BlurFilter(bool vertical, UITexture* srcTex, RenderTextureClass* dstTex, int width)
{
	return BlurFilter(vertical, srcTex->GetShaderResourceView(), dstTex, width);
}

bool GraphicsClass::BlurFilter(bool vertical, RenderTextureClass * srcTex, RenderTextureClass * dstTex, int width)
{
	return BlurFilter(vertical, srcTex->GetShaderResourceView(), dstTex, width);
}

bool GraphicsClass::BlurFilter(bool vertical, ID3D11ShaderResourceView * srcTex, RenderTextureClass * dstTex, int width)
{
	XMMATRIX worldMatrix, viewMatrix, orthoMatrix;
	bool result;

	if (vertical == false) //HORIZONTAL - 1st
	{
		dstTex->SetRenderTarget(m_D3D->GetDeviceContext(), m_D3D->GetDepthStencilView());
		dstTex->ClearRenderTarget(m_D3D->GetDeviceContext(), m_D3D->GetDepthStencilView(), 0.0f, 0.0f, 0.0f, 1.0f);

		m_Camera->Render();

		m_Camera->GetViewMatrix(viewMatrix);
		m_D3D->GetWorldMatrix(worldMatrix);
		dstTex->GetOrthoMatrix(orthoMatrix);

		m_D3D->TurnZBufferOff();

		m_blurShaderHorizontal->SetTextureSize(width);
		m_blurShaderHorizontal->SetTextureResourceView(srcTex);
		m_renderTexturePreview->GetModel()->Render(m_D3D->GetDeviceContext());
		result = m_blurShaderHorizontal->Render(m_D3D->GetDeviceContext(), m_Model->GetIndexCount(), worldMatrix, viewMatrix, orthoMatrix);
		if (!result)
			return false;

		m_D3D->TurnZBufferOn();

		m_D3D->SetBackBufferRenderTarget();
		m_D3D->ResetViewport();

		m_renderTexturePreview->BindTexture(dstTex->GetShaderResourceView());
	}
	else //VERTICAL - 2nd
	{
		dstTex->SetRenderTarget(m_D3D->GetDeviceContext(), m_D3D->GetDepthStencilView());
		dstTex->ClearRenderTarget(m_D3D->GetDeviceContext(), m_D3D->GetDepthStencilView(), 0.0f, 0.0f, 0.0f, 1.0f);

		m_Camera->Render();

		m_Camera->GetViewMatrix(viewMatrix);
		m_D3D->GetWorldMatrix(worldMatrix);
		dstTex->GetOrthoMatrix(orthoMatrix);

		m_D3D->TurnZBufferOff();

		m_blurShaderVertical->SetTextureSize(width);
		m_blurShaderVertical->SetTextureResourceView(srcTex);
		m_renderTexturePreview->GetModel()->Render(m_D3D->GetDeviceContext());
		result = m_blurShaderVertical->Render(m_D3D->GetDeviceContext(), m_Model->GetIndexCount(), worldMatrix, viewMatrix, orthoMatrix);
		if (!result)
			return false;

		m_D3D->TurnZBufferOn();

		m_D3D->SetBackBufferRenderTarget();
		m_D3D->ResetViewport();

		m_renderTexturePreview->BindTexture(dstTex->GetShaderResourceView());
	}

	return true;
}

bool GraphicsClass::ConvoluteShader(ID3D11ShaderResourceView * srcTex, RenderTextureClass * dstTex)
{
	ifstream skyboxFile;
	skyboxFile.open("Skyboxes/conv_cubemap.dds");
	if (skyboxFile.fail() == false)
		return true;

	XMMATRIX worldMatrix, viewMatrix, projectionMatrix;
	bool result;

	XMFLOAT3 position = XMFLOAT3(0, 0, 0);
	XMVECTOR tar[] = { XMVectorSet(1, 0, 0, 0), XMVectorSet(-1, 0, 0, 0), XMVectorSet(0, 1, 0, 0), XMVectorSet(0, -1, 0, 0), XMVectorSet(0, 0, 1, 0), XMVectorSet(0, 0, -1, 0) };
	XMFLOAT3 up[] = { XMFLOAT3{0, 1, 0}, XMFLOAT3{0, 0, 1}, XMFLOAT3{1, 0, 0}, XMFLOAT3{0, -1, 0}, XMFLOAT3{0, 0, -1}, XMFLOAT3{-1, 0, 0} };
	wchar_t* filenames[] = { L"Skyboxes/conv_negy.dds", L"Skyboxes/conv_negz.dds" , L"Skyboxes/conv_negx.dds", L"Skyboxes/conv_posy.dds", L"Skyboxes/conv_posz.dds", L"Skyboxes/conv_posx.dds" };
	for (int i = 0; i < 6; i++)
	{
		dstTex->SetRenderTarget(m_D3D->GetDeviceContext(), m_D3D->GetDepthStencilView());
		dstTex->ClearRenderTarget(m_D3D->GetDeviceContext(), m_D3D->GetDepthStencilView(), 1.0f, 0.0f, 0.0f, 1.0f);

		m_Camera->Render();
		m_Camera->GetViewMatrix(viewMatrix);
		m_D3D->GetWorldMatrix(worldMatrix);
		m_D3D->GetProjectionMatrix(projectionMatrix);

		m_convoluteShader->SetUpVector(up[i]);
		if (i > 3)
			m_convoluteShader->SetRightVector(-1);
		else
			m_convoluteShader->SetRightVector(1);

		//viewMatrix = DirectX::XMMatrixLookToLH(XMVECTOR{ 0.0f,0.0f,0.0f }, tar[i], XMVECTOR{ 0,1,0 });

		m_D3D->ChangeRasterizerCulling(D3D11_CULL_BACK);
		m_D3D->ChangeDepthStencilComparison(D3D11_COMPARISON_LESS_EQUAL);

		m_groundQuadModel->Render(m_D3D->GetDeviceContext());
		m_convoluteShader->Render(m_D3D->GetDeviceContext(), m_groundQuadModel->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix);

		m_D3D->ChangeRasterizerCulling(D3D11_CULL_BACK);
		m_D3D->ChangeDepthStencilComparison(D3D11_COMPARISON_LESS);

		m_D3D->SetBackBufferRenderTarget();
		m_D3D->ResetViewport();

		m_renderTexturePreview->BindTexture(dstTex->GetShaderResourceView());

		SaveDDSTextureToFile(m_D3D->GetDeviceContext(), dstTex->GetShaderResource(), filenames[i]);
	}

	system("texassemble cube -w 256 -h 256 -f R8G8B8A8_UNORM -o Skyboxes/conv_cubemap.dds Skyboxes/conv_posx.dds Skyboxes/conv_negx.dds Skyboxes/conv_posy.dds Skyboxes/conv_negy.dds Skyboxes/conv_posz.dds Skyboxes/conv_negz.dds");

	return true;
}

bool GraphicsClass::CreateShadowMap(RenderTextureClass* targetTex)
{
	targetTex->SetRenderTarget(m_D3D->GetDeviceContext(), m_D3D->GetDepthStencilView());
	targetTex->ClearRenderTarget(m_D3D->GetDeviceContext(), m_D3D->GetDepthStencilView(), 0.0f, 0.0f, 0.0f, 1.0f);

	if (RenderDepthScene() == false)
		return false;

	m_renderTexturePreview->BindTexture(targetTex->GetShaderResourceView());

	m_D3D->SetBackBufferRenderTarget();
	m_D3D->ResetViewport();

	return true;
}

bool GraphicsClass::RenderDepthScene()
{
	XMMATRIX worldMatrix, lightViewMatrix, lightProjectionMatrix, translateMatrix;
	XMMATRIX viewMatrix, projectionMatrix;
	float posX, posY, posZ;
	bool result;

	//Set light position
	m_directionalLight->SetLookAt(0, 0, 0);
	//m_directionalLight->SetPosition(-20, 15, -5);
	m_directionalLight->SetPosition(0, 5, -5);
	m_shadowMapShader->SetLightPosition(m_directionalLight->GetPosition());
	m_Model->Render(m_D3D->GetDeviceContext());

	//Create matrices based on light position
	m_D3D->GetWorldMatrix(worldMatrix);
	m_directionalLight->GenerateViewMatrix();
	m_directionalLight->GetViewMatrix(lightViewMatrix);
	m_directionalLight->GetProjectionMatrix(lightProjectionMatrix);

	//Translate model position and save to worldMatrix
	//XMFLOAT4 position = m_Model->GetPosition();
	//worldMatrix = XMMatrixTranslation(position.x, position.y, position.z);

	//Render sphere model
	//worldMatrix = DirectX::XMMatrixMultiply(worldMatrix, XMMatrixRotationX(45.4f));
	//worldMatrix = DirectX::XMMatrixMultiply(worldMatrix, XMMatrixTranslation(0, -0.6f, 0));
	//m_groundQuadModel->Render(m_D3D->GetDeviceContext());
	//result = m_shadowMapShader->Render(m_D3D->GetDeviceContext(), m_groundQuadModel->GetIndexCount(), worldMatrix, lightViewMatrix, lightProjectionMatrix);
	//if (!result)
	//	return false;

	for (int i = 0; i < 5; i++)
	{
		m_D3D->GetWorldMatrix(worldMatrix);
		m_directionalLight->GenerateViewMatrix();
		m_directionalLight->GetViewMatrix(lightViewMatrix);
		m_directionalLight->GetProjectionMatrix(lightProjectionMatrix);

		worldMatrix = XMMatrixTranslation(i * 2.0f, 2.0f, 1.0f);
		m_Model->Render(m_D3D->GetDeviceContext());
		result = m_shadowMapShader->Render(m_D3D->GetDeviceContext(), m_Model->GetIndexCount(), worldMatrix, lightViewMatrix, lightProjectionMatrix);
		if (!result)
			return false;
	}

	return true;
}


//m_Camera->GetViewMatrix(viewMatrix);
//m_D3D->GetWorldMatrix(worldMatrix);
//m_D3D->GetProjectionMatrix(projectionMatrix);
////RENDER MAIN SCENE MODEL
//m_D3D->ChangeRasterizerCulling(D3D11_CULL_BACK);
//m_D3D->ChangeDepthStencilComparison(D3D11_COMPARISON_LESS_EQUAL);
//
//m_Camera->SetPosition(-1.0f, 5.0f, 0.0f);
//m_shadowMapShader->SetLightPosition(m_Camera->GetPosition());
//m_Camera->Render();
//m_Model->Render(m_D3D->GetDeviceContext());
//m_pbrShader->m_cameraPosition = m_Camera->GetPosition();
//
//m_D3D->GetWorldMatrix(worldMatrix);
//m_Camera->GetViewMatrix(viewMatrix);
//m_D3D->GetProjectionMatrix(projectionMatrix);
////worldMatrix = DirectX::XMMatrixMultiply(worldMatrix, DirectX::XMMatrixRotationY(m_Camera->GetRotation().y / 3.14f));
////worldMatrix = DirectX::XMMatrixMultiply(worldMatrix, DirectX::XMMatrixRotationX(m_Camera->GetRotation().x / 3.14f));
//result = m_shadowMapShader->Render(m_D3D->GetDeviceContext(), m_Model->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix);
//if (!result)
//return false;
//
//m_D3D->GetWorldMatrix(worldMatrix);
//m_directionalLight->GetViewMatrix(viewMatrix);
//m_directionalLight->GetProjectionMatrix(projectionMatrix);
//
//worldMatrix = DirectX::XMMatrixMultiply(worldMatrix, XMMatrixRotationX(45.4f));
//worldMatrix = DirectX::XMMatrixMultiply(worldMatrix, XMMatrixTranslation(0, -0.5f, 0));
////m_groundQuadModel->Render(m_D3D->GetDeviceContext());
////result = m_shadowMapShader->Render(m_D3D->GetDeviceContext(), m_groundQuadModel->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix);
////if (!result)
////	return false;