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
	//m_Camera->SetPosition(0.0f, 0.1f, -0.35f); //BUDDA
	//m_Camera->SetPosition(0.0f, 5.0f, -15.0f); //SHADOWMAPPING
	m_Camera->SetPosition(0.0f, 0.0f, -2.0f); //SINGLE SPHERE
	
	// Create the model object.
	m_Model = new ModelClass;
	if(!m_Model)
		return false;

	m_skyboxModel = new ModelClass;
	if (!m_skyboxModel)
		return false;

	m_cubeModel = new ModelClass;
	if (!m_cubeModel)
		return false;

	//Initialize the model object.
	result = m_skyboxModel->Initialize(m_D3D->GetDevice(), "sphere.obj");
	if (!result)
		return false;

	result = m_Model->Initialize(m_D3D->GetDevice(), "sphere.obj");
	if(!result)
		return false;

	result = m_cubeModel->Initialize(m_D3D->GetDevice(), "cube.obj");
	if (!result)
		return false;

	//Create mouse container
	m_mouse = new MouseClassContainer;

	if (!(m_pbrShader = new ShaderPBRClass))
		return false;

	//INITIALIZE ImGUI
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	ImGui_ImplWin32_Init(hwnd);
	ImGui_ImplDX11_Init(m_D3D->GetDevice(), m_D3D->GetDeviceContext());
	ImGui::StyleColorsDark();

	//SCENE LIGHTING
	m_directionalLight = new LightClass;
	m_directionalLight->SetLookAt(0, 0, 0);
	m_directionalLight->SetPosition(0, 10, -5);
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

	if (!m_pbrShader->Initialize(m_D3D->GetDevice(), hwnd, L"pbr_used.vs", L"pbr_used.ps", input))
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

	//m_pbrShader->SetRoughness(0.32f);
	//m_pbrShader->SetMetalness(1.0f);

	//Direction + strength (w)
	m_pbrShader->AddDirectionalLight(XMFLOAT4(0.0f, 10.0f, -5.0f, 1.0f), 1.0f, 1.0f, 1.0f);

	//m_pbrShader->AddDirectionalLight(XMFLOAT4(0.0f, 1.0f, 5.0f, 10.0f), 1.0f, 0.0f, 0.0f);
	//m_pbrShader->AddDirectionalLight(XMFLOAT4(-11.0f, -1.0f, 0.0f, 2.5f), 0.0f, 1.0f, 0.0f);
	//m_pbrShader->AddDirectionalLight(XMFLOAT4(-2.76f, 4.5f, 5.0f, 5.0f), 0.0f, 0.0f, 1.0f);
	//m_pbrShader->AddDirectionalLight(XMFLOAT4(1.0f, 5.0f, 0.0f, 0.5f), 1.0f, 1.0f, 1.0f);

	//m_pbrShader->AddPointLight(XMFLOAT3{ 0.0f, 0.0f, -5.0f }, 10.0f, 1.0f, 0.0f, 0.0f, 0.7f);
	//m_pbrShader->AddPointLight(XMFLOAT3{ 0.0f, 15.0f, 5.0f }, 10.0f, 0.0f, 1.0f, 0.0f, 0.4f);
	//m_pbrShader->AddPointLight(XMFLOAT3{ 0.0f, 5.0f, -10.0f}, 10.0f, 0.0f, 0.0f, 1.0f, 1.5f);
	//m_pbrShader->AddPointLight(XMFLOAT3{ 5.0f , 0.0f, 12.0f}, 10.0f, 1.0f, 1.0f, 1.0f, 0.4f);
	
	m_pbrShader->AddDirectionalLight(XMFLOAT3{ 0.0f, 0.0f, -1.0f }, 0.4f, 1.0f, 0.0f, 0.0f);
	m_pbrShader->AddDirectionalLight(XMFLOAT3{ 0.0f, 2.0f, 6.0f }, 0.7f, 0.0f, 1.0f, 0.0f);
	m_pbrShader->AddDirectionalLight(XMFLOAT3{ 0.0f, 10.f, 3.0f}, 1.25f, 0.0f, 0.0f, 1.0f);
	m_pbrShader->AddDirectionalLight(XMFLOAT3{ 5.0f , -3.0f, 0.0f}, 1.0f, 1.0f, 1.0f, 0.4f);
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

	ID3D11Resource* tmp = nullptr;
	for (int i = 0; i < MAX_TEXTURE_INPUT; i++)
	{
		m_emptyTexView[i] = nullptr;
		UITexturePreview::LoadTexture(m_D3D->GetDevice(), EMPTY_TEX, tmp, m_emptyTexView[i]);
	}
	UITexturePreview::LoadTexture(m_D3D->GetDevice(), EMPTY_TEX, tmp, m_emptyTexViewEditor);

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
	if (!(m_renderTextureMainScene = new RenderTextureClass))
		return false;
	if (!(m_renderTextureMainScene->Initialize(m_D3D->GetDevice(), screenWidth, screenHeight)))
		return false;

	//Blur shader - vertical and horizontal
	m_blurShaderHorizontal = new BlurShaderClass;
	if (!m_blurShaderHorizontal->Initialize(m_D3D->GetDevice(), *m_D3D->GetHWND(), L"blurHorizontal.vs", L"blurHorizontal.ps", input))
		return false;

	m_blurShaderVertical = new BlurShaderClass;
	if (!m_blurShaderVertical->Initialize(m_D3D->GetDevice(), *m_D3D->GetHWND(), L"blurVertical.vs", L"blurVertical.ps", input))
		return false;

	m_convoluteQuadModel = new ModelClass;
	m_convoluteQuadModel->Initialize(m_D3D->GetDevice(), ModelClass::ShapeSize::RECTANGLE, -1.0f, 1.0f, 1.0f, -1.0f, true);

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
		system("texassemble cube -w 512 -h 512 -f R8G8B8A8_UNORM -o Skyboxes/cubemap.dds Skyboxes/posx.bmp Skyboxes/negx.bmp Skyboxes/posy.bmp Skyboxes/negy.bmp Skyboxes/posz.bmp Skyboxes/negz.bmp");
		skyboxFile.open("Skyboxes/cubemap.dds");
		if (skyboxFile.fail())
		{
			system("texassemble cube -w 512 -h 512 -f R8G8B8A8_UNORM -o Skyboxes/cubemap.dds Skyboxes/posx.jpg Skyboxes/negx.jpg Skyboxes/posy.jpg Skyboxes/negy.jpg Skyboxes/posz.jpg Skyboxes/negz.jpg");
			skyboxFile.open("Skyboxes/cubemap.dds");
			if (skyboxFile.fail())
				return false;
		}
	}
	//LOAD SKYBOX
	if (!(m_skyboxShader = new SkyboxShaderClass))
		return false;
	if (!m_skyboxShader->Initialize(m_D3D->GetDevice(), *m_D3D->GetHWND(), L"skybox.vs", L"skybox.ps", input))
		return false;

	skyboxFile.close();
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
	
	//Calculate specular IBL environment prefiltered map
	if (!(m_environmentTextureMap = new RenderTextureClass))
		return false;
	if (!(m_environmentTextureMap->Initialize(m_D3D->GetDevice(), ENVIRONMENT_SPECULAR_SIZE, ENVIRONMENT_SPECULAR_SIZE, RenderTextureClass::Scaling::NONE)))
		return false;

	if (!(m_brdfLUT = new RenderTextureClass))
		return false;
	if (!(m_brdfLUT->Initialize(m_D3D->GetDevice(), 512, 512, RenderTextureClass::Scaling::NONE)))
		return false;
	
	if (!(m_specularIBLShader = new SkyboxShaderClass))
		return false;
	if (!(m_specularIBLShader->Initialize(m_D3D->GetDevice(), *m_D3D->GetHWND(), L"environmentPrefilteredMap.vs", L"environmentPrefilteredMap.ps", input)))
		return false;

	m_specularIBLShader->LoadTexture(m_D3D->GetDevice(), L"Skyboxes/cubemap.dds", m_specularIBLShader->m_skyboxTexture, m_specularIBLShader->m_skyboxTextureView);
	m_specularIBLShader->SetType(SkyboxShaderClass::SkyboxType::ENVIRO);
	m_specularIBLShader->SetUpVector(XMFLOAT3{ 0, 1, 0 });

	if (!(m_brdfLutShader = new SkyboxShaderClass))
		return false;
	if (!(m_brdfLutShader->Initialize(m_D3D->GetDevice(), *m_D3D->GetHWND(), L"SpecularLookupIBL.vs", L"SpecularLookupIBL.ps", input)))
		return false;

	//PrepareLutBrdf(m_brdfLUT);
	PrepareEnvironmentPrefilteredMap(m_specularIBLShader->m_skyboxTextureView, m_environmentTextureMap);
	CreateSingleEnvironmentMap();

	//Skybox texture previews
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
	m_skyboxPreviewForward->Initialize(m_D3D, -0.25f, .0f, .33f, L"Skyboxes/conv_posz.dds");
	m_skyboxPreviewBack->Initialize(m_D3D, 0.411f, .0, .33f, L"Skyboxes/conv_negz.dds");

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
	//if (!m_pbrShader->LoadIrradianceMap(m_environmentTextureMap->GetShaderResourceView()))
	//if (!m_pbrShader->LoadIrradianceMap(m_D3D->GetDevice(), L"Skyboxes/enviro_cubemap_0.dds"))
		return false;
	//if (!m_pbrShader->LoadEnvironmentMap(m_D3D->GetDevice(), L"Skyboxes/enviro_cubemap_0.dds"))
	//if (!m_pbrShader->LoadEnvironmentMap(m_environmentTextureMap->GetShaderResourceView()))
		//return false;
	m_pbrShader->AddEnvironmentMapLevel(m_D3D->GetDevice(), L"Skyboxes/enviro_cubemap_0.dds");
	m_pbrShader->AddEnvironmentMapLevel(m_D3D->GetDevice(), L"Skyboxes/enviro_cubemap_1.dds");
	m_pbrShader->AddEnvironmentMapLevel(m_D3D->GetDevice(), L"Skyboxes/enviro_cubemap_2.dds");
	m_pbrShader->AddEnvironmentMapLevel(m_D3D->GetDevice(), L"Skyboxes/enviro_cubemap_3.dds");
	m_pbrShader->AddEnvironmentMapLevel(m_D3D->GetDevice(), L"Skyboxes/enviro_cubemap_4.dds");
	if (!m_pbrShader->LoadBrdfLut(m_D3D->GetDevice(), L"Skyboxes/LutBRDF.dds"))
		return false;

	CreateShadowMap(m_shadowMapTexture);

#pragma region SSAO
	//Create textures that will hold buffers
	m_positionBuffer = new RenderTextureClass;
	m_positionBuffer->Initialize(m_D3D->GetDevice(), 1280, 720);
	//m_positionBuffer->GetShaderResourceView() = nullptr;
	
	m_normalBuffer = new RenderTextureClass;
	m_normalBuffer->Initialize(m_D3D->GetDevice(), 1280, 720);
	//m_normalBuffer->GetShaderResourceView() = nullptr;

	m_albedoBuffer = new RenderTextureClass;
	m_albedoBuffer->Initialize(m_D3D->GetDevice(), 1280, 720);
	//m_albedoBuffer->GetShaderResourceView() = nullptr;

	//Create material shaders for buffers
	m_GBufferShaderPosition = new GBufferShader;	
	if (!m_GBufferShaderPosition->Initialize(m_D3D->GetDevice(), *m_D3D->GetHWND(), L"positionGBuffer.vs", L"positionGBuffer.ps", input, GBufferShader::BufferType::POSITION))
		return false;

	m_GBufferShaderNormal = new GBufferShader;
	m_GBufferShaderNormal->ChangeTextureType(GBufferShader::BufferType::NORMAL);
	if (!m_GBufferShaderNormal->Initialize(m_D3D->GetDevice(), *m_D3D->GetHWND(), L"normalGBuffer.vs", L"normalGBuffer.ps", input, GBufferShader::BufferType::NORMAL))
		return false;

	m_GBufferShaderSSAO = new GBufferShader;
	m_GBufferShaderSSAO->LoadPositionTexture(m_positionBuffer->GetShaderResourceView());
	m_GBufferShaderSSAO->LoadNormalTexture(m_normalBuffer->GetShaderResourceView());

	m_GBufferShaderSSAO->ChangeTextureType(GBufferShader::BufferType::SSAO);
	if (!m_GBufferShaderSSAO->Initialize(m_D3D->GetDevice(), *m_D3D->GetHWND(), L"ssaoShader.vs", L"ssaoShader.ps", input, GBufferShader::BufferType::SSAO))
		return false;

	//Create kernel for SSAO
	std::uniform_real_distribution<float> randomFloats(0.0f, 1.0f);
	std::default_random_engine generator;
	XMFLOAT3 tmpSample;
	for (int i = 0; i < SSAO_KERNEL_SIZE; i++)
	{
		//Generate random vector3 ([-1, 1], [-1, 1], [0, 1])
		tmpSample.x = randomFloats(generator) * 2.0f - 1.0f;
		tmpSample.y = randomFloats(generator) * 2.0f - 1.0f;
		tmpSample.z = randomFloats(generator);

		//Normalize vector3
		XMVECTOR tmpVector;
		tmpVector.m128_f32[0] = tmpSample.x;
		tmpVector.m128_f32[1] = tmpSample.y;
		tmpVector.m128_f32[2] = tmpSample.z;

		tmpVector = XMVector3Normalize(tmpVector);
		
		tmpSample.x = tmpVector.m128_f32[0];
		tmpSample.y = tmpVector.m128_f32[1];
		tmpSample.z = tmpVector.m128_f32[2];

		//Multiply by random value all coordinates of vector3
		//float randomMultiply = randomFloats(generator);
		//tmpSample.x *= randomMultiply;
		//tmpSample.y *= randomMultiply;
		//tmpSample.z *= randomMultiply;

		//Scale samples so they are more aligned to middle of hemisphere
		float scale = float(i) / 64.0f;
		scale = lerp(0.1f, 1.0f, scale * scale);
		tmpSample.x *= scale;
		tmpSample.y *= scale;
		tmpSample.z *= scale;

		//Pass value to array
		m_ssaoKernel[i] = tmpSample;
	}

	XMFLOAT2 tmpNoise;
	for (int i = 0; i < 16; i++)
	{
		tmpNoise.x = randomFloats(generator);
		tmpNoise.y = randomFloats(generator);
		m_ssaoNoise[i] = tmpNoise;
	}

	//Create SSAO noise texture
	//m_ssaoNoiseTexture = new RenderTextureClass;
	//m_ssaoNoiseTexture->Initialize(m_D3D->GetDevice(), SSAO_NOISE_SIZE, SSAO_NOISE_SIZE);
	//m_GBufferShader->ChangeTextureType(GBufferShader::BufferType::SSAO_NOISE);
	//if (!m_GBufferShader->Initialize(m_D3D->GetDevice(), *m_D3D->GetHWND(), L"ssaoNoise.vs", L"ssaoNoise.ps", input))
	//	return false;
	//if (!RenderSSAONoiseTexture(m_ssaoNoiseTexture))
	//	return false;

	//Create SSAO texture
	m_ssaoTexture = new RenderTextureClass;
	if (!m_ssaoTexture->Initialize(m_D3D->GetDevice(), screenWidth, screenHeight))
		return false;
	//m_ssaoTexture->GetShaderResourceView() = nullptr;

	m_postSSAOTexture = new RenderTextureClass;
	if (!m_postSSAOTexture->Initialize(m_D3D->GetDevice(), screenWidth, screenHeight))
		return false;

	m_ssaoNoiseTexture = new RenderTextureClass;
	m_ssaoNoiseTexture->Initialize(m_D3D->GetDevice(), SSAO_NOISE_SIZE, SSAO_NOISE_SIZE);
	if (!m_ssaoNoiseTexture->LoadTexture(m_D3D->GetDevice(), L"ssaoNoise.dds", m_ssaoNoiseTexture->GetShaderResource(), m_ssaoNoiseTexture->GetShaderResourceView()))
		return false;
	m_GBufferShaderSSAO->SetKernelValues(m_ssaoKernel);
	m_GBufferShaderSSAO->SetNoiseValues(m_ssaoNoise);
	//m_GBufferShader->LoadPositionTexture(m_positionBuffer->GetShaderResourceView());
	//m_GBufferShader->LoadNormalTexture(m_normalBuffer->GetShaderResourceView());
	//m_GBufferShader->LoadNoiseTexture(m_ssaoNoiseTexture->GetShaderResourceView());

	//m_GBufferShader->ChangeTextureType(GBufferShader::BufferType::SSAO);
	//if (!m_GBufferShader->Initialize(m_D3D->GetDevice(), *m_D3D->GetHWND(), L"ssaoShader.vs", L"ssaoShader.ps", input))
	//	return false;
	//if (!RenderSSAOTexture(m_ssaoTexture))
	//	return false;
#pragma endregion

	m_postProcessShader = new PostProcessShader;
	if (!m_postProcessShader)
		return false;
	if (!m_postProcessShader->Initialize(m_D3D->GetDevice(), *m_D3D->GetHWND(), L"postprocess.vs", L"postprocess.ps", input))
		return false;

#pragma region BLOOM
	m_bloomShader = new BloomShaderClass;
	if (!m_bloomShader->Initialize(m_D3D->GetDevice(), *m_D3D->GetHWND(), L"bloomRange.vs", L"bloomRange.ps", input))
		return false;

	m_bloomHelperTexture = new RenderTextureClass;
	if (!m_bloomHelperTexture->Initialize(m_D3D->GetDevice(), m_screenWidth, m_screenHeight))
		return false;

	m_bloomHorizontalBlur = new RenderTextureClass;
	if (!m_bloomHorizontalBlur->Initialize(m_D3D->GetDevice(), m_screenWidth, m_screenHeight))
		return false;

	m_bloomVerticalBlur = new RenderTextureClass;
	if (!m_bloomVerticalBlur->Initialize(m_D3D->GetDevice(), m_screenWidth, m_screenHeight))
		return false;
#pragma endregion

#pragma region VIGNETTE
	m_vignetteShader = new VignetteShader;
	if (!m_vignetteShader->Initialize(m_D3D->GetDevice(), *m_D3D->GetHWND(), L"textureShader.vs", L"textureShader.ps", input))
		return false;

	if (!BaseShaderClass::LoadTexture(m_D3D->GetDevice(), L"Vignette.png", m_vignetteShader->m_vignetteResource, m_vignetteShader->m_vignetteResourceView, false))
		return false;
#pragma endregion

#pragma region LUT
	m_lutShader = new LUTShader;
	if (!m_lutShader->Initialize(m_D3D->GetDevice(), *m_D3D->GetHWND(), L"lutShader.vs", L"lutShader.ps", input))
		return false;
	m_lutShader->SetLUT(m_D3D->GetDevice(), L"lut_sepia.png", false);
#pragma endregion
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

	//if (m_directionalLight->GetPosition().x > 5.0f)
	//	m_directionalLight->SetPosition(0.0f, m_directionalLight->GetPosition().y, m_directionalLight->GetPosition().z);
	//m_directionalLight->SetPosition(m_directionalLight->GetPosition().x + 0.01f, m_directionalLight->GetPosition().y , m_directionalLight->GetPosition().z);

	result = Render();
	if(!result)
	{
		return false;
	}

	return true;
}

void GraphicsClass::MoveCameraForward(float val)
{
	m_Camera->AddPosition(0, val, 0);
}

void GraphicsClass::MoveCameraBackward(float val)
{
	m_Camera->AddPosition(0, -val, 0);
}

void GraphicsClass::MoveCameraLeft(float val)
{
	m_Camera->AddPosition(-val, 0, 0);
}

void GraphicsClass::MoveCameraRight(float val)
{
	m_Camera->AddPosition(val, 0, 0);
}

void GraphicsClass::MoveCameraUp(float val)
{
	m_Camera->AddPosition(0, 0, val);
}

void GraphicsClass::MoveCameraDown(float val)
{
	m_Camera->AddPosition(0, 0, -val);
}

void GraphicsClass::RotateCamera(XMVECTOR rotation)
{
	m_Camera->SetRotation(m_Camera->GetRotation().x + rotation.m128_f32[0], m_Camera->GetRotation().y + rotation.m128_f32[1], m_Camera->GetRotation().z + rotation.m128_f32[2]);
}

void GraphicsClass::UpdateUI()
{
	XMMATRIX worldMatrix, viewMatrix, projectionMatrix;
	m_Camera->Render();
	m_Camera->GetViewMatrix(viewMatrix);
	m_D3D->GetWorldMatrix(worldMatrix);
	m_D3D->GetProjectionMatrix(projectionMatrix);

	m_D3D->EnableAlphaBlending();
	if (m_shaderEditorManager)
	{
		m_shaderEditorManager->UpdateBlocks();
	}
	m_D3D->DisableAlphaBlending();

	return;

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

void GraphicsClass::UpdateShaderEditorMouseOnly()
{
	if (m_shaderEditorManager)
	{
		m_shaderEditorManager->UpdateBlocks(true);
	}
}

void GraphicsClass::SetMouseRef(MouseClass * mouse)
{
	ShowCursor(true);
	m_mouse->SetMouse(mouse);
	if (m_mouse->GetModel() == nullptr)
		m_mouse->InitializeMouse();

	if (!CreateShaderEditor())
		PostQuitMessage(0);
}

MouseClass* GraphicsClass::GetMouse()
{
	if (m_mouse == nullptr)
		return nullptr;

	return m_mouse->GetMouse();
}

D3DClass * GraphicsClass::GetD3D()
{
	return m_D3D;
}

TextEngine::FontData * GraphicsClass::AddText(float && posX, float && posY, std::string&& text, float && scale, TextEngine::Align && align, XMVECTOR && color)
{
	return m_textEngine->WriteText(m_D3D->GetDeviceContext(), m_screenWidth, m_screenHeight, posX, posY, text, scale, align, color);
}

void GraphicsClass::ChangeRenderWindow()
{
	RENDER_MATERIAL_EDITOR = !RENDER_MATERIAL_EDITOR;
}

void GraphicsClass::DeleteCurrentShaderBlock()
{
	if (m_shaderEditorManager)
		m_shaderEditorManager->DeleteCurrentShaderBlock();
}

bool GraphicsClass::IsChoosingShaderWindowActive()
{
	if (m_shaderEditorManager)
		return m_shaderEditorManager->WillRenderChoosingWindow();
	else
		return false;
}

void GraphicsClass::ChangeChoosingWindowShaderFocus(ShaderWindowDirection direction)
{
	if (m_shaderEditorManager)
	{
		if (direction == GraphicsClass::ShaderWindowDirection::Down)
		{
			(*m_shaderEditorManager->GetChoosingWindowHandler())++;
			if (*m_shaderEditorManager->GetChoosingWindowHandler() >= m_shaderEditorManager->ChoosingWindowItems.size())
			{
				(*m_shaderEditorManager->GetChoosingWindowHandler()) = 0;
			}
		}
		else if (direction == GraphicsClass::ShaderWindowDirection::Up)
		{
			(*m_shaderEditorManager->GetChoosingWindowHandler())--;
			if (*m_shaderEditorManager->GetChoosingWindowHandler() < 0)
			{
				(*m_shaderEditorManager->GetChoosingWindowHandler()) = m_shaderEditorManager->ChoosingWindowItems.size() - 1;
			}
		}
	}
}

void GraphicsClass::FocusOnChoosingWindowsShader()
{
	m_focusOnChoosingWindowsShader = true;
}

void GraphicsClass::AcceptCurrentChoosingWindowShader()
{
	if (m_shaderEditorManager)
		m_shaderEditorManager->CreateBlock(m_shaderEditorManager->ChoosingWindowItems[*m_shaderEditorManager->GetChoosingWindowHandler()]);
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
		if (RENDER_MATERIAL_EDITOR == false)
		{
			if (m_postprocessSSAO)
			{
				if (!RenderGBufferPosition(m_positionBuffer, m_GBufferShaderPosition))
					return false;

				if (!RenderGBufferNormal(m_normalBuffer, m_GBufferShaderNormal))
					return false;

				m_GBufferShaderSSAO->LoadPositionTexture(m_positionBuffer->GetShaderResourceView());
				m_GBufferShaderSSAO->LoadNormalTexture(m_normalBuffer->GetShaderResourceView());

				if (!RenderSSAOTexture(m_ssaoTexture, m_GBufferShaderSSAO))
					return false;
			}
			m_D3D->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

			//STANDARD SCENE RENDERING		
			result = RenderScene();
			if (!result)
				return false;
		}
		else
		{
			UpdateUI();
		}

	//PREVIEW SKYBOX IN 6 FACES FORM
		//m_skyboxPreviewRight->Render(m_D3D->GetDeviceContext(), m_groundQuadModel->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix);
		//m_skyboxPreviewLeft->Render(m_D3D->GetDeviceContext(), m_groundQuadModel->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix);
		//m_skyboxPreviewUp->Render(m_D3D->GetDeviceContext(), m_groundQuadModel->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix);
		//m_skyboxPreviewDown->Render(m_D3D->GetDeviceContext(), m_groundQuadModel->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix);
		//m_skyboxPreviewForward->Render(m_D3D->GetDeviceContext(), m_groundQuadModel->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix);
		//m_skyboxPreviewBack->Render(m_D3D->GetDeviceContext(), m_groundQuadModel->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix);
	}

	if (ENABLE_DEBUG)
	{
		RenderDebugSettings();
	}

	if (ENABLE_GUI)
	{
		RenderGUI();
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
	//m_Camera->SetPosition(0.0f, 0.0f, -2.0f);
	m_Camera->Render();

	//RENDER MAIN SCENE MODEL
	m_D3D->ChangeRasterizerCulling(D3D11_CULL_BACK);
	m_D3D->ChangeDepthStencilComparison(D3D11_COMPARISON_LESS_EQUAL);

	m_Camera->GetViewMatrix(viewMatrix);
	m_D3D->GetWorldMatrix(worldMatrix);
	m_D3D->GetProjectionMatrix(projectionMatrix);

	//worldMatrix = DirectX::XMMatrixMultiply(worldMatrix, XMMatrixRotationX(45.4f));
	//worldMatrix = DirectX::XMMatrixMultiply(worldMatrix, XMMatrixTranslation(0, -0.6f, 0));
	//m_groundQuadModel->Render(m_D3D->GetDeviceContext());
	//m_colorShader->m_shadowMapResourceView = m_shadowMapTexture->GetShaderResourceView();
	//m_directionalLight->GenerateViewMatrix();
	//m_directionalLight->GenerateProjectionMatrix(SCREEN_NEAR, SCREEN_DEPTH);
	//m_directionalLight->GetViewMatrix(lightViewMatrix);
	//m_directionalLight->GetProjectionMatrix(lightProjectionMatrix);
	//m_colorShader->SetLightViewProjection(lightViewMatrix, lightProjectionMatrix);
	//result = m_colorShader->Render(m_D3D->GetDeviceContext(), m_groundQuadModel->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix);
	//if (!result)
	//	return false;

	m_Model->Render(m_D3D->GetDeviceContext());
	m_pbrShader->m_cameraPosition = m_Camera->GetPosition();
	////XMVECTOR light_ = XMVector3Rotate(XMVECTOR{ -1.0f, 0.0, -1.0f, 0.0f }, XMVECTOR{ m_Camera->GetRotation().x / 3.14f, m_Camera->GetRotation().y / 3.14f, m_Camera->GetRotation().z / 3.14f, 0.0f });
	//m_pbrShader->m_lightDirection = XMFLOAT4(light_.m128_f32[0], 0.0, light_.m128_f32[2], 1.2f);

	m_Camera->GetViewMatrix(viewMatrix);
	m_D3D->GetWorldMatrix(worldMatrix);
	m_D3D->GetProjectionMatrix(projectionMatrix);
	//m_Camera->SetRotation(-0.5f, 91.0f, 0);
	worldMatrix = DirectX::XMMatrixMultiply(worldMatrix, XMMatrixTranslation(0.0f, -0.15f, 0.0f));
	worldMatrix = DirectX::XMMatrixMultiply(worldMatrix, DirectX::XMMatrixRotationY(m_Camera->GetRotation().y / 3.14f));
	worldMatrix = DirectX::XMMatrixMultiply(worldMatrix, DirectX::XMMatrixRotationX(m_Camera->GetRotation().x / 3.14f));

	//METALNESS / ROUGHNESS presentation
	//result = m_colorShader->Render(m_D3D->GetDeviceContext(), m_Model->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix);
	//if (!result)
	//	return false;
	//for (float x = 0; x < 6; x++)
	//{
	//	for (float y = 0; y < 6; y++)
	//	{
	//		worldMatrix = XMMatrixIdentity();
	//		worldMatrix = XMMatrixTranslation(x * 1.25f, y * 1.25f, 0.0f);
	//		m_pbrShader->SetRoughness(x / 6.0f);
	//		m_pbrShader->SetMetalness(y / 6.0f);
	//		result = m_pbrShader->Render(m_D3D->GetDeviceContext(), m_Model->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix);
	//		if (!result)
	//			return false;
	//	}
	//}

	m_renderTextureMainScene->SetRenderTarget(m_D3D->GetDeviceContext(), m_D3D->GetDepthStencilView());
	m_renderTextureMainScene->ClearRenderTarget(m_D3D->GetDeviceContext(), m_D3D->GetDepthStencilView(), 0.0f, 0.0f, 0.0f, 1.0f);

	if (DRAW_SKYBOX)
	{
		if (RenderSkybox() == false)
			return false;
	}
	m_Model->Render(m_D3D->GetDeviceContext());
	result = m_pbrShader->Render(m_D3D->GetDeviceContext(), m_Model->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix);
	if (!result)
		return false;

	m_D3D->SetBackBufferRenderTarget();

	if (!m_postprocessSSAO)
		m_postProcessShader->ResetSSAO();
	if (!m_postprocessBloom)
		m_postProcessShader->ResetBloom();
	if (!m_postprocessLUT)
		m_postProcessShader->ResetLUT();
	if (!m_postprocessChromaticAberration)
		m_postProcessShader->UseChromaticAberration(false);
	if (!m_postprocessGrain)
		m_postProcessShader->UseGrain(false);

	if (m_postprocessSSAO)
	{
		if (m_postprocessBloom)
		{
			m_postSSAOTexture->SetRenderTarget(m_D3D->GetDeviceContext(), m_D3D->GetDepthStencilView());
			m_postSSAOTexture->ClearRenderTarget(m_D3D->GetDeviceContext(), m_D3D->GetDepthStencilView(), 0.0f, 0.0f, 0.0f, 1.0f);
		}

		ApplySSAO(m_ssaoTexture->GetShaderResourceView(), m_renderTextureMainScene->GetShaderResourceView());
		
		m_D3D->SetBackBufferRenderTarget();
	}

	if (m_postprocessBloom)
	{
		m_convoluteQuadModel->Initialize(m_D3D->GetDevice(), ModelClass::ShapeSize::RECTANGLE, -1.0f, 1.0f, 1.0f, -1.0f, true);
		m_convoluteQuadModel->Render(m_D3D->GetDeviceContext());

		m_Camera->Render();
		m_Camera->GetViewMatrix(viewMatrix);
		m_D3D->GetWorldMatrix(worldMatrix);
		m_D3D->GetProjectionMatrix(projectionMatrix);

		m_bloomShader->SetBloomIntensity(XMFLOAT3{ m_bloomSettings.intensity });
		if (m_postprocessSSAO)
			m_bloomShader->m_bloomTextureView = m_postSSAOTexture->GetShaderResourceView();
		else
			m_bloomShader->m_bloomTextureView = m_renderTextureMainScene->GetShaderResourceView();

		{
			m_bloomHelperTexture->SetRenderTarget(m_D3D->GetDeviceContext(), m_D3D->GetDepthStencilView());
			m_bloomHelperTexture->ClearRenderTarget(m_D3D->GetDeviceContext(), m_D3D->GetDepthStencilView(), 0, 0, 0, 1);

			m_bloomShader->Render(m_D3D->GetDeviceContext(), m_convoluteQuadModel->GetIndexCount(), worldMatrix * 0, viewMatrix, projectionMatrix);

			m_D3D->SetBackBufferRenderTarget(); 
		}

		m_blurShaderHorizontal->SetWeights(m_bloomSettings.weights);
		m_blurShaderVertical->SetWeights(m_bloomSettings.weights);

		BlurFilterScreenSpace(false, m_bloomHelperTexture, m_bloomHorizontalBlur, m_screenWidth / 2); //Blur horizontal
		BlurFilterScreenSpace(true, m_bloomHorizontalBlur, m_bloomVerticalBlur, m_screenHeight / 2); //Blur vertical
		for (int i = 0; i < 10; i++)
		{
			BlurFilterScreenSpace(false, m_bloomVerticalBlur, m_bloomHorizontalBlur, m_screenWidth / 2); //Blur horizontal
			BlurFilterScreenSpace(true, m_bloomHorizontalBlur, m_bloomVerticalBlur, m_screenHeight / 2); //Blur vertical
		}

		ApplyBloom(m_bloomVerticalBlur->GetShaderResourceView(), m_renderTextureMainScene->GetShaderResourceView());
	}

	if (!m_postprocessSSAO && !m_postprocessBloom && !m_postprocessLUT)
	{
		//m_convoluteQuadModel->Initialize(m_D3D->GetDevice(), ModelClass::ShapeSize::RECTANGLE, -1.0f, 1.0f, 1.0f, -1.0f, true);
		m_convoluteQuadModel->Render(m_D3D->GetDeviceContext());

		m_Camera->Render();
		m_Camera->GetViewMatrix(viewMatrix);
		m_D3D->GetWorldMatrix(worldMatrix);
		m_D3D->GetProjectionMatrix(projectionMatrix);

		m_renderTexturePreview->BindTexture(m_renderTextureMainScene->GetShaderResourceView());
		result = m_renderTexturePreview->Render(m_D3D->GetDeviceContext(), 0, worldMatrix, viewMatrix, projectionMatrix);
		if (!result)
			return false;
	}
	
	if (m_postprocessLUT)
	{
		ApplyLUT(m_lutShader->GetLUT(), m_renderTextureMainScene->GetShaderResourceView());
	}

	if (m_postprocessChromaticAberration)
	{		
		ApplyChromaticAberration(nullptr, m_renderTextureMainScene->GetShaderResourceView());
		m_postProcessShader->SetChromaticAberrationOffsets(m_chromaticOffset.red, m_chromaticOffset.green, m_chromaticOffset.blue);
		m_postProcessShader->SetChromaticAberrationIntensity(m_chromaticIntensity);
	}
	
	if (m_postprocessGrain)
	{
		ApplyGrain(nullptr, m_renderTextureMainScene->GetShaderResourceView());
		m_postProcessShader->SetGrainSettings(m_grainSettings.intensity, m_grainSettings.size, (int)m_grainSettings.type, m_grainSettings.hasColor);
	}

	m_bloomHorizontalBlur->ClearRenderTarget(m_D3D->GetDeviceContext(), m_D3D->GetDepthStencilView(), 0.0f, 0.0f, 0.0f, 1.0f);
	m_bloomVerticalBlur->ClearRenderTarget(m_D3D->GetDeviceContext(), m_D3D->GetDepthStencilView(), 0.0f, 0.0f, 0.0f, 1.0f);

	if (m_postprocessVignette)
	{
		m_ssaoTexture->ClearRenderTarget(m_D3D->GetDeviceContext(), m_D3D->GetDepthStencilView(), 0.0f, 0.0f, 0.0f, 1.0f);

		m_Camera->Render();
		m_Camera->GetViewMatrix(viewMatrix);
		m_D3D->GetWorldMatrix(worldMatrix);
		m_D3D->GetProjectionMatrix(projectionMatrix);

		m_convoluteQuadModel->Render(m_D3D->GetDeviceContext());

		m_D3D->EnableAlphaBlending();

		m_renderTexturePreview->BindTexture(m_vignetteShader->m_vignetteResourceView);
		result = m_renderTexturePreview->Render(m_D3D->GetDeviceContext(), 0, worldMatrix, viewMatrix, projectionMatrix);
		if (!result)
			return false;

		m_D3D->DisableAlphaBlending();
	}

	//worldMatrix = XMMatrixTranslation(0.5f, 0.0f, -1.0f);
	//worldMatrix = XMMatrixMultiply(worldMatrix, XMMatrixScaling(4.0f, 6.0f, 1.0f));
	//m_cubeModel->Render(m_D3D->GetDeviceContext());
	//m_colorShader->Render(m_D3D->GetDeviceContext(), m_cubeModel->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix);

	//SHADOWMAPPING presentation
	//for (int i = 0; i < 5; i++)
	//{
	//	m_Camera->GetViewMatrix(viewMatrix);
	//	m_D3D->GetWorldMatrix(worldMatrix);
	//	m_D3D->GetProjectionMatrix(projectionMatrix);

	//	worldMatrix = XMMatrixTranslation(i * 2.0f, 2.0f, 1.0f);
	//	m_Model->Render(m_D3D->GetDeviceContext());
	//	//m_colorShader->m_shadowMapResourceView = m_shadowMapTexture->GetShaderResourceView();
	//	result = m_pbrShader->Render(m_D3D->GetDeviceContext(), m_Model->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix);
	//	if (!result)
	//		return false;
	//}

	if (m_postprocessBloom)
	{
		//Clear to make sure there is no pointer to local variable which outlives scope
		m_bloomShader->m_bloomTexture = nullptr;
		m_bloomShader->m_bloomTextureView = nullptr;
	}

	return true;
}

bool GraphicsClass::RenderGUI()
{
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	PassMouseInfo(m_mouse->GetMouse()->GetLMBPressed(), m_mouse->GetMouse()->GetRMBPressed());

	if (RENDER_MATERIAL_EDITOR && m_shaderEditorManager)
	{
		//Base window of editor - flow control/variables
		{
			ImGui::Begin("Shader editor");
			if (ImGui::Button("Generate shader"))
			{
				m_shaderEditorManager->GenerateCodeToFile();
			}
			//Show scalar options
			if (m_shaderEditorManager->m_focusedBlock && m_shaderEditorManager->m_focusedBlock->GetInputCount() == 0)
			{
				if (m_shaderEditorManager->m_focusedBlock->m_outputNodes.size() > 0 && &m_shaderEditorManager->m_focusedBlock->m_outputNodes[0])
				{
					if (m_shaderEditorManager->m_focusedBlock->GetFunctionName() == "texture")
					{
						RenderTextureViewImGuiEditor(m_shaderEditorManager->m_focusedBlock->m_outputNodes[0]->m_connectedTexture,
							m_shaderEditorManager->m_focusedBlock->m_outputNodes[0]->m_connectedTextureView, "Texture", 
							m_shaderEditorManager->m_focusedBlock->m_outputNodes[0]->m_texturePath);
					}
					else if (m_shaderEditorManager->m_focusedBlock->m_outputNodes[0]->m_returnType == "float")
					{
						ImGui::InputFloat("Value", &m_shaderEditorManager->m_focusedBlock->m_outputNodes[0]->m_value);
						}
					else if (m_shaderEditorManager->m_focusedBlock->m_outputNodes[0]->m_returnType == "float2")
					{
						ImGui::InputFloat("Value_1", &m_shaderEditorManager->m_focusedBlock->m_outputNodes[0]->m_valueTwo[0]);
						ImGui::InputFloat("Value_2", &m_shaderEditorManager->m_focusedBlock->m_outputNodes[0]->m_valueTwo[1]);
					}
					else if (m_shaderEditorManager->m_focusedBlock->m_outputNodes[0]->m_returnType == "float3")
					{
						ImGui::InputFloat("Value_1", &m_shaderEditorManager->m_focusedBlock->m_outputNodes[0]->m_valueThree[0]);
						ImGui::InputFloat("Value_2", &m_shaderEditorManager->m_focusedBlock->m_outputNodes[0]->m_valueThree[1]);
						ImGui::InputFloat("Value_3", &m_shaderEditorManager->m_focusedBlock->m_outputNodes[0]->m_valueThree[2]);

						ImGui::ColorPicker3("Color value", m_shaderEditorManager->m_focusedBlock->m_outputNodes[0]->m_valueThree);
					}
					else if (m_shaderEditorManager->m_focusedBlock->m_outputNodes[0]->m_returnType == "float4")
					{
						ImGui::InputFloat("Value_1", &m_shaderEditorManager->m_focusedBlock->m_outputNodes[0]->m_valueFour[0]);
						ImGui::InputFloat("Value_2", &m_shaderEditorManager->m_focusedBlock->m_outputNodes[0]->m_valueFour[1]);
						ImGui::InputFloat("Value_3", &m_shaderEditorManager->m_focusedBlock->m_outputNodes[0]->m_valueFour[2]);
						ImGui::InputFloat("Value_4", &m_shaderEditorManager->m_focusedBlock->m_outputNodes[0]->m_valueFour[3]);

						ImGui::ColorPicker4("Color value", m_shaderEditorManager->m_focusedBlock->m_outputNodes[0]->m_valueFour);
					}					
				}
			}
			m_shaderEditorManager->UpdateMouseHoveredOnImGui(ImGui::IsWindowHovered() || ImGui::IsWindowFocused());
			ImGui::End();
		}
		//Adding new blocks
		if (m_shaderEditorManager->WillRenderChoosingWindow())
		{			
			ImGui::Begin("Add block", (bool*)0, ImGuiWindowFlags_::ImGuiWindowFlags_NoMove | ImGuiWindowFlags_::ImGuiWindowFlags_NoResize | ImGuiWindowFlags_::ImGuiWindowFlags_NoSavedSettings
					| ImGuiWindowFlags_::ImGuiWindowFlags_AlwaysAutoResize);
			
			ImGui::SetWindowPos({ m_shaderEditorManager->GetWindowPositionX(), m_shaderEditorManager->GetWindowPositionY() });
			if (ImGui::InputText("SEARCH", const_cast<char*>(m_shaderEditorManager->m_choosingWindowSearch.data()), m_shaderEditorManager->k_choosingWindowSearchSize))
			{
			}

			if (*m_shaderEditorManager->GetChoosingWindowHandler() >= m_shaderEditorManager->ChoosingWindowItems.size())
			{
				*m_shaderEditorManager->GetChoosingWindowHandler() = 0;
			}

			if (m_focusOnChoosingWindowsShader)
			{
				m_focusOnChoosingWindowsShader = false;
				ImGui::SetKeyboardFocusHere(0);
			}
			m_shaderEditorManager->SearchThroughChoosingWindow();

			if (ImGui::ListBox("", m_shaderEditorManager->GetChoosingWindowHandler(), m_shaderEditorManager->ChoosingWindowItems.data(), m_shaderEditorManager->ChoosingWindowItems.size()))
			{
				m_hideShaderWindowOnNextTry = false;
				m_shaderEditorManager->CreateBlock(m_shaderEditorManager->ChoosingWindowItems[*m_shaderEditorManager->GetChoosingWindowHandler()]);
			}
			else if (!ImGui::IsWindowFocused() && !ImGui::IsWindowHovered() && !m_mouse->GetMouse()->GetLMBPressed())
			{
				if (m_hideShaderWindowOnNextTry)
				{
					m_hideShaderWindowOnNextTry = false;
					m_shaderEditorManager->CreateBlock("");
				}
				else
					m_hideShaderWindowOnNextTry = true;
			}

			ImGui::End();
		}

		goto Finished_drawing;
	}

	ImGui::Begin("BaseWindow");
	
	//Sliders - roughness/metalness in case of empty texture maps
	ImGui::SliderFloat("Roughness", &m_pbrShader->m_roughness, 0.0f, 1.0f, "%.2f");
	ImGui::SliderFloat("Metalness", &m_pbrShader->m_metalness, 0.0f, 1.0f, "%.2f");
	//Color picker - in case of empty albedo
	ImGui::ColorPicker3("Tint", m_pbrShader->m_tint);
	//SSAO radius/bias slider
		//ImGui::SliderFloat("SSAO Radius", &m_GBufferShaderSSAO->m_radiusSize, 0.0f, 5.0f, "%.2f");
		//ImGui::SliderFloat("SSAO Bias", &m_GBufferShaderSSAO->m_bias, 0.0f, 0.1f, "%.3f");

	//BLOOM settings
	ImGui::SliderFloat("Bloom weight 0", &m_bloomSettings.weights[0], 0.0f, 1.0f, "%.5f");
	ImGui::SliderFloat("Bloom weight 1", &m_bloomSettings.weights[1], 0.0f, 1.0f, "%.5f");
	ImGui::SliderFloat("Bloom weight 2", &m_bloomSettings.weights[2], 0.0f, 1.0f, "%.5f");
	ImGui::SliderFloat("Bloom weight 3", &m_bloomSettings.weights[3], 0.0f, 1.0f, "%.5f");
	ImGui::SliderFloat("Bloom weight 4", &m_bloomSettings.weights[4], 0.0f, 1.0f, "%.5f");
	ImGui::ColorPicker3("Bloom intensity", m_bloomSettings.intensity);

	ImGui::Spacing();

	//CHROMATIC ABERRATION settings
	ImGui::SliderFloat("Chromatic red offset", &m_chromaticOffset.red, -0.01f, 0.01f, "%.5f");
	ImGui::SliderFloat("Chromatic green offset", &m_chromaticOffset.green, -0.01f, 0.01f, "%.5f");
	ImGui::SliderFloat("Chromatic blue offset", &m_chromaticOffset.blue, -0.01f, 0.01f, "%.5f");
	ImGui::SliderFloat("Chromatic intensity", &m_chromaticIntensity, 0.001f, 2.0f, "%.3f");

	ImGui::Spacing();

	//GRAIN settings
	ImGui::SliderFloat("Grain intensity", &m_grainSettings.intensity, 0.0f, 1.0f, "%.2f");
	ImGui::SliderFloat("Grain size", &m_grainSettings.size, 0.01f, 10.0f, "%.2f");

	if (ImGui::BeginCombo("Grain type", CURRENT_GRAIN_TYPE))
	{
		constexpr int count = (int)GrainType::Count;
		for (int i = 0; i < count; ++i)
		{
			bool isSelected = (i == (int)m_grainSettings.type);
			if (ImGui::Selectable(GrainTypeArray[i], isSelected))
			{
				CURRENT_GRAIN_TYPE = GrainTypeArray[i];
				m_grainSettings.type = (GrainType)i;
			}
			if (isSelected)
			{
				ImGui::SetItemDefaultFocus();
			}
		}

		ImGui::EndCombo();
	}
	ImGui::Checkbox("Grain has color", &m_grainSettings.hasColor);

	ImGui::Spacing();
	ImGui::Spacing();
	ImGui::Spacing();

	m_internalTextureViewIndex = 0;

	//Post-process stack bools
	ImGui::Checkbox("Use SSAO", &m_postprocessSSAO);
	ImGui::Checkbox("Use Bloom", &m_postprocessBloom);
	ImGui::Checkbox("Use Vignette", &m_postprocessVignette);
	ImGui::Checkbox("Use LUT", &m_postprocessLUT);
	ImGui::Checkbox("Use Chromatic Aberration", &m_postprocessChromaticAberration);
	ImGui::Checkbox("Use Grain", &m_postprocessGrain);
	
	//Roughness map input
	RenderTextureViewImGui(m_pbrShader->m_roughnessTexture, m_pbrShader->m_roughnessTextureView, "Roughness map:");
	RenderTextureViewImGui(m_pbrShader->m_metalnessTexture, m_pbrShader->m_metalnessTextureView, "Metalness map:");
	RenderTextureViewImGui(m_pbrShader->m_normalTexture, m_pbrShader->m_normalTextureView, "Normal map:");
	RenderTextureViewImGui(m_pbrShader->m_diffuseTexture, m_pbrShader->m_diffuseTextureView, "Albedo map:");

	//Finish ImGui and render
	ImGui::End();

Finished_drawing:
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

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
	XMFLOAT3 tmp = m_Camera->GetPosition();
	m_Camera->SetPosition(0.0f, 0.0f, 0.01f);
	m_Camera->Render();
	m_Camera->SetPosition(tmp.x, tmp.y, tmp.z);
	m_Camera->GetViewMatrix(viewMatrix);
	m_D3D->GetWorldMatrix(worldMatrix);
	m_D3D->GetProjectionMatrix(projectionMatrix);
	//worldMatrix = DirectX::XMMatrixMultiply(worldMatrix, DirectX::XMMatrixRotationY(m_Camera->GetRotation().y / 3.14f));
	//worldMatrix = DirectX::XMMatrixMultiply(worldMatrix, DirectX::XMMatrixRotationX(m_Camera->GetRotation().x / 3.14f));

	m_D3D->ChangeRasterizerCulling(D3D11_CULL_BACK);
	m_D3D->ChangeDepthStencilComparison(D3D11_COMPARISON_LESS_EQUAL);

	m_skyboxModel->Render(m_D3D->GetDeviceContext());
	m_skyboxShader->Render(m_D3D->GetDeviceContext(), m_skyboxModel->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix);

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

bool GraphicsClass::BlurFilterScreenSpace(bool vertical, const RenderTextureClass* textureToBlur, RenderTextureClass* textureToReturn, int screenWidth)
{
	ID3D11ShaderResourceView* resourceView = textureToBlur->GetShaderResourceViewCopy();
	return BlurFilterScreenSpace(vertical, resourceView, textureToReturn, screenWidth);
}

bool GraphicsClass::BlurFilterScreenSpace(bool vertical, const ID3D11ShaderResourceView * textureToBlur, RenderTextureClass * textureToReturn, int screenWidth)
{
	XMMATRIX worldMatrix, viewMatrix, orthoMatrix;
	bool result;

	if (vertical == false) //HORIZONTAL - 1st
	{
		textureToReturn->SetRenderTarget(m_D3D->GetDeviceContext(), m_D3D->GetDepthStencilView());
		textureToReturn->ClearRenderTarget(m_D3D->GetDeviceContext(), m_D3D->GetDepthStencilView(), 0.0f, 0.0f, 0.0f, 1.0f);

		m_Camera->Render();

		m_Camera->GetViewMatrix(viewMatrix);
		m_D3D->GetWorldMatrix(worldMatrix);
		textureToReturn->GetOrthoMatrix(orthoMatrix);

		m_D3D->TurnZBufferOff();

		m_blurShaderHorizontal->SetTextureSize(screenWidth);
		m_blurShaderHorizontal->SetTextureResourceView(const_cast<ID3D11ShaderResourceView*>(textureToBlur));
		//m_renderTexturePreview->GetModel()->Render(m_D3D->GetDeviceContext());
		result = m_blurShaderHorizontal->Render(m_D3D->GetDeviceContext(), m_Model->GetIndexCount(), worldMatrix, viewMatrix, orthoMatrix);
		if (!result)
			return false;

		m_D3D->TurnZBufferOn();

		m_D3D->SetBackBufferRenderTarget();
		//m_D3D->ResetViewport();

		//m_renderTexturePreview->BindTexture(textureToBlur->GetShaderResourceView());
	}
	else //VERTICAL - 2nd
	{
		textureToReturn->SetRenderTarget(m_D3D->GetDeviceContext(), m_D3D->GetDepthStencilView());
		textureToReturn->ClearRenderTarget(m_D3D->GetDeviceContext(), m_D3D->GetDepthStencilView(), 0.0f, 0.0f, 0.0f, 1.0f);

		m_Camera->Render();

		m_Camera->GetViewMatrix(viewMatrix);
		m_D3D->GetWorldMatrix(worldMatrix);
		textureToReturn->GetOrthoMatrix(orthoMatrix);

		m_D3D->TurnZBufferOff();

		m_blurShaderVertical->SetTextureSize(screenWidth);
		m_blurShaderVertical->SetTextureResourceView(const_cast<ID3D11ShaderResourceView*>(textureToBlur));
		//m_renderTexturePreview->GetModel()->Render(m_D3D->GetDeviceContext());
		result = m_blurShaderVertical->Render(m_D3D->GetDeviceContext(), m_Model->GetIndexCount(), worldMatrix, viewMatrix, orthoMatrix);
		if (!result)
			return false;

		m_D3D->TurnZBufferOn();

		m_D3D->SetBackBufferRenderTarget();
		//m_D3D->ResetViewport();

		//m_renderTexturePreview->BindTexture(textureToBlur->GetShaderResourceView());
	}

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
			return false;
	}
	skyboxFile.close();

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
	{
		skyboxFile.close();
		return true;
	}

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

		m_D3D->ChangeRasterizerCulling(D3D11_CULL_BACK);
		m_D3D->ChangeDepthStencilComparison(D3D11_COMPARISON_LESS_EQUAL);

		m_convoluteQuadModel->Render(m_D3D->GetDeviceContext());
		m_convoluteShader->Render(m_D3D->GetDeviceContext(), m_convoluteQuadModel->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix);

		m_D3D->ChangeRasterizerCulling(D3D11_CULL_BACK);
		m_D3D->ChangeDepthStencilComparison(D3D11_COMPARISON_LESS);

		m_D3D->SetBackBufferRenderTarget();
		m_D3D->ResetViewport();

		//m_renderTexturePreview->BindTexture(dstTex->GetShaderResourceView());

		SaveDDSTextureToFile(m_D3D->GetDeviceContext(), dstTex->GetShaderResource(), filenames[i]);
	}

	system("texassemble cube -w 256 -h 256 -f R8G8B8A8_UNORM -o Skyboxes/conv_cubemap.dds Skyboxes/conv_posx.dds Skyboxes/conv_negx.dds Skyboxes/conv_posy.dds Skyboxes/conv_negy.dds Skyboxes/conv_posz.dds Skyboxes/conv_negz.dds");

	return true;
}

bool GraphicsClass::CreateShadowMap(RenderTextureClass*& targetTex)
{
	targetTex->SetRenderTarget(m_D3D->GetDeviceContext(), m_D3D->GetDepthStencilView());
	targetTex->ClearRenderTarget(m_D3D->GetDeviceContext(), m_D3D->GetDepthStencilView(), 1.0f, 1.0f, 1.0f, 1.0f);

	if (RenderDepthScene() == false)
		return false;

	//m_renderTexturePreview->BindTexture(targetTex->GetShaderResourceView());

	m_D3D->SetBackBufferRenderTarget();
	m_D3D->ResetViewport();
	m_D3D->BeginScene(0, 0, 0, 1);

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
	//m_directionalLight->SetPosition(5, 10, -5);
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

	//worldMatrix = XMMatrixTranslation(0.5f, 0.0f, -1.0f);
	//worldMatrix = XMMatrixMultiply(worldMatrix, XMMatrixScaling(4.0f, 6.0f, 1.0f));
	//m_cubeModel->Render(m_D3D->GetDeviceContext());
	//m_shadowMapShader->Render(m_D3D->GetDeviceContext(), m_cubeModel->GetIndexCount(), worldMatrix, lightViewMatrix, lightProjectionMatrix);

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

bool GraphicsClass::PrepareEnvironmentPrefilteredMap(ID3D11ShaderResourceView * srcTex, RenderTextureClass * dstTex)
{
	XMMATRIX worldMatrix, viewMatrix, projectionMatrix;
	bool result;

	XMFLOAT3 position = XMFLOAT3(0, 0, 0);
	XMVECTOR tar[] = { XMVectorSet(1, 0, 0, 0), XMVectorSet(-1, 0, 0, 0), XMVectorSet(0, 1, 0, 0), XMVectorSet(0, -1, 0, 0), XMVectorSet(0, 0, 1, 0), XMVectorSet(0, 0, -1, 0) };
	XMFLOAT3 up[] = { XMFLOAT3{ 0, 1, 0 }, XMFLOAT3{ 0, 0, 1 }, XMFLOAT3{ 1, 0, 0 }, XMFLOAT3{ 0, -1, 0 }, XMFLOAT3{ 0, 0, -1 }, XMFLOAT3{ -1, 0, 0 } };
	wchar_t* filenames[] = { L"Skyboxes/enviro_negy.dds", L"Skyboxes/enviro_negz.dds" , L"Skyboxes/enviro_negx.dds", L"Skyboxes/enviro_posy.dds", L"Skyboxes/enviro_posz.dds", L"Skyboxes/enviro_posx.dds" };
	string filenamesString[] = { "Skyboxes/enviro_negy.dds", "Skyboxes/enviro_negz.dds" , "Skyboxes/enviro_negx.dds", "Skyboxes/enviro_posy.dds", "Skyboxes/enviro_posz.dds", "Skyboxes/enviro_posx.dds" };
	string filenamesCubemap[] = { "Skyboxes/enviro_cubemap_0.dds", "Skyboxes/enviro_cubemap_1.dds" , "Skyboxes/enviro_cubemap_2.dds", "Skyboxes/enviro_cubemap_3.dds", "Skyboxes/enviro_cubemap_4.dds" };

	D3D11_TEXTURE2D_DESC textureDesc;
	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;

	ID3D11Texture2D *texture2D;
	ID3D11RenderTargetView* renderTargetView;
	ID3D11ShaderResourceView* shaderResourceView;

	ZeroMemory(&textureDesc, sizeof(textureDesc));
	renderTargetViewDesc.Format = textureDesc.Format;
	renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	renderTargetViewDesc.Texture2D.MipSlice = 0;

	ZeroMemory(&shaderResourceViewDesc, sizeof(shaderResourceViewDesc));
	shaderResourceViewDesc.Format = textureDesc.Format;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;

	ifstream skyboxFile;
	int size = sizeof(filenamesCubemap) / sizeof(filenamesCubemap[0]);
	for (int i = 0; i < size; i++)
	{
		skyboxFile.open(filenamesCubemap[i].c_str());
		if (skyboxFile.fail() == false)
		{
			skyboxFile.close();
			return true;
		}
	}

	float color[4]{ 1.0f, 0.0f, 0.0f, 1.0f };
	////////////////////////////////////////////////////
	///////////////////////////////////////////////////
	//////////////////////////////////////////////////
	for (int mip = 0; mip < 5; mip++)
	{
		unsigned mipWidth = 256 * pow(0.5, mip);
		unsigned mipHeight = 256 * pow(0.5, mip);

		ZeroMemory(&textureDesc, sizeof(textureDesc));
		textureDesc.Width = mipWidth;
		textureDesc.Height = mipHeight;
		textureDesc.MipLevels = 1;
		textureDesc.ArraySize = 1;
		textureDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.SampleDesc.Quality = 0;
		textureDesc.Usage = D3D11_USAGE_DEFAULT;
		textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		textureDesc.CPUAccessFlags = 0;
		textureDesc.MiscFlags = 0;
		result = m_D3D->GetDevice()->CreateTexture2D(&textureDesc, NULL, &texture2D);
		if (FAILED(result))
			return false;

		result = m_D3D->GetDevice()->CreateRenderTargetView(texture2D, &renderTargetViewDesc, &renderTargetView);
		if (FAILED(result))
			return false;

		result = m_D3D->GetDevice()->CreateShaderResourceView(texture2D, &shaderResourceViewDesc, &shaderResourceView);
		if (FAILED(result))
			return false;

		D3D11_VIEWPORT envMapviewport;
		envMapviewport.Width = mipWidth;
		envMapviewport.Height = mipHeight;
		envMapviewport.MinDepth = 0.0f;
		envMapviewport.MaxDepth = 1.0f;
		envMapviewport.TopLeftX = 0.0f;
		envMapviewport.TopLeftY = 0.0f;

		float roughness = (float)mip / 5.0f;

		m_D3D->GetDeviceContext()->OMSetRenderTargets(1, &renderTargetView, m_D3D->GetDepthStencilView());
		m_D3D->GetDeviceContext()->ClearDepthStencilView(m_D3D->GetDepthStencilView(), D3D11_CLEAR_DEPTH, 1.0f, 0);
		m_D3D->GetDeviceContext()->ClearRenderTargetView(renderTargetView, color);

		m_D3D->GetDeviceContext()->RSSetViewports(1, &envMapviewport);

		for (int i = 0; i < 6; i++)
		{
			m_Camera->Render();
			m_Camera->GetViewMatrix(viewMatrix);
			m_D3D->GetWorldMatrix(worldMatrix);
			m_D3D->GetProjectionMatrix(projectionMatrix);

			m_specularIBLShader->SetUpVector(up[i]);
			m_specularIBLShader->SetRoughness(roughness);

			m_D3D->ChangeRasterizerCulling(D3D11_CULL_BACK);
			m_D3D->ChangeDepthStencilComparison(D3D11_COMPARISON_LESS_EQUAL);

			m_convoluteQuadModel->Render(m_D3D->GetDeviceContext());
			m_specularIBLShader->Render(m_D3D->GetDeviceContext(), m_convoluteQuadModel->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix);

			m_D3D->ChangeRasterizerCulling(D3D11_CULL_BACK);
			m_D3D->ChangeDepthStencilComparison(D3D11_COMPARISON_LESS);

			SaveDDSTextureToFile(m_D3D->GetDeviceContext(), texture2D, filenames[i]);
		}
		string tmp = "texassemble cube -w ";
		tmp += std::to_string(mipWidth);
		tmp += " -h ";
		tmp += std::to_string(mipWidth);
		tmp += " -f R8G8B8A8_UNORM -o ";
		tmp += filenamesCubemap[mip];
		tmp += " Skyboxes/enviro_posx.dds Skyboxes/enviro_negx.dds Skyboxes/enviro_posy.dds Skyboxes/enviro_negy.dds Skyboxes/enviro_posz.dds Skyboxes/enviro_negz.dds";
		
		system(tmp.c_str());
	}
	m_D3D->SetBackBufferRenderTarget();
	m_D3D->ResetViewport();

	return true;
}

bool GraphicsClass::PrepareLutBrdf(RenderTextureClass * dstTex)
{
	XMMATRIX worldMatrix, viewMatrix, projectionMatrix;
	bool result;

	dstTex->SetRenderTarget(m_D3D->GetDeviceContext(), m_D3D->GetDepthStencilView(), true);
	dstTex->ClearRenderTarget(m_D3D->GetDeviceContext(), m_D3D->GetDepthStencilView(), 1.0f, 0.0f, 0.0f, 1.0f);

	m_Camera->SetPosition(0,0,0.f);
	m_Camera->Render();
	m_Camera->GetViewMatrix(viewMatrix);
	m_D3D->GetWorldMatrix(worldMatrix);
	m_D3D->GetProjectionMatrix(projectionMatrix);

	//m_D3D->TurnZBufferOff();
		m_convoluteQuadModel->Render(m_D3D->GetDeviceContext());
		m_brdfLutShader->Render(m_D3D->GetDeviceContext(), m_convoluteQuadModel->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix);
	//m_D3D->TurnZBufferOn();

	m_D3D->SetBackBufferRenderTarget();
	m_D3D->ResetViewport();

	m_renderTexturePreview->BindTexture(dstTex->GetShaderResourceView());

	SaveDDSTextureToFile(m_D3D->GetDeviceContext(), dstTex->GetShaderResource(), L"Skyboxes/LutBRDF.dds");

	return true;
}

bool GraphicsClass::CreateSingleEnvironmentMap()
{
	XMMATRIX worldMatrix, viewMatrix, projectionMatrix;
	bool result;

	XMFLOAT3 position = XMFLOAT3(0, 0, 0);
	XMVECTOR tar[] = { XMVectorSet(1, 0, 0, 0), XMVectorSet(-1, 0, 0, 0), XMVectorSet(0, 1, 0, 0), XMVectorSet(0, -1, 0, 0), XMVectorSet(0, 0, 1, 0), XMVectorSet(0, 0, -1, 0) };
	XMFLOAT3 up[] = { XMFLOAT3{ 0, 1, 0 }, XMFLOAT3{ 0, 0, 1 }, XMFLOAT3{ 1, 0, 0 }, XMFLOAT3{ 0, -1, 0 }, XMFLOAT3{ 0, 0, -1 }, XMFLOAT3{ -1, 0, 0 } };
	wchar_t* filenames[] = { L"Skyboxes/enviro_negy.dds", L"Skyboxes/enviro_negz.dds" , L"Skyboxes/enviro_negx.dds", L"Skyboxes/enviro_posy.dds", L"Skyboxes/enviro_posz.dds", L"Skyboxes/enviro_posx.dds" };
	string filenamesString[] = { "Skyboxes/enviro_negy.dds", "Skyboxes/enviro_negz.dds" , "Skyboxes/enviro_negx.dds", "Skyboxes/enviro_posy.dds", "Skyboxes/enviro_posz.dds", "Skyboxes/enviro_posx.dds" };
	string filenamesCubemap[] = { "Skyboxes/enviro_cubemap_0.dds", "Skyboxes/enviro_cubemap_1.dds" , "Skyboxes/enviro_cubemap_2.dds", "Skyboxes/enviro_cubemap_3.dds", "Skyboxes/enviro_cubemap_4.dds" };

	D3D11_TEXTURE2D_DESC textureDesc;
	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;

	ID3D11Texture2D *texture2D;
	ID3D11RenderTargetView* renderTargetView[6];

	ifstream skyboxFile;
	int size = sizeof(filenamesCubemap) / sizeof(filenamesCubemap[0]);
	for (int i = 0; i < size; i++)
	{
		skyboxFile.open(filenamesCubemap[i].c_str());
		if (skyboxFile.fail() == false)
		{
			skyboxFile.close();
			return true;
		}
	}

	float color[4]{ 1.0f, 0.0f, 0.0f, 1.0f };

	ZeroMemory(&textureDesc, sizeof(textureDesc));
	textureDesc.Width = 256;
	textureDesc.Height = 256;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 6;
	textureDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE | D3D11_RESOURCE_MISC_GENERATE_MIPS;
	result = m_D3D->GetDevice()->CreateTexture2D(&textureDesc, NULL, &texture2D);
	if (FAILED(result))
		return false;

	ZeroMemory(&shaderResourceViewDesc, sizeof(shaderResourceViewDesc));
	shaderResourceViewDesc.Format = textureDesc.Format;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	shaderResourceViewDesc.TextureCube.MostDetailedMip = 0;
	shaderResourceViewDesc.TextureCube.MipLevels = 5;

	result = m_D3D->GetDevice()->CreateShaderResourceView(texture2D, &shaderResourceViewDesc, &m_environmentTextureMap->GetShaderResourceView());
	if (FAILED(result))
		return false;
	////////////////////////////////////////////////////
	///////////////////////////////////////////////////
	//////////////////////////////////////////////////
	for (int mip = 0; mip < 5; mip++)
	{
		ZeroMemory(&textureDesc, sizeof(textureDesc));
		renderTargetViewDesc.Format = textureDesc.Format;
		renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
		renderTargetViewDesc.Texture2DArray.ArraySize = 1;
		renderTargetViewDesc.Texture2DArray.MipSlice = mip;

		unsigned mipWidth = 256 * pow(0.5, mip);
		unsigned mipHeight = 256 * pow(0.5, mip);

		D3D11_VIEWPORT envMapviewport;
		envMapviewport.Width = mipWidth;
		envMapviewport.Height = mipHeight;
		envMapviewport.MinDepth = 0.0f;
		envMapviewport.MaxDepth = 1.0f;
		envMapviewport.TopLeftX = 0.0f;
		envMapviewport.TopLeftY = 0.0f;

		float roughness = (float)mip / 5.0f;

		for (int i = 0; i < 6; i++)
		{		
			renderTargetViewDesc.Texture2DArray.FirstArraySlice = i;
			result = m_D3D->GetDevice()->CreateRenderTargetView(texture2D, &renderTargetViewDesc, &renderTargetView[i]);
			if (FAILED(result))
				return false;

			m_D3D->GetDeviceContext()->OMSetRenderTargets(1, &renderTargetView[i], 0);
			//m_D3D->GetDeviceContext()->ClearDepthStencilView(m_D3D->GetDepthStencilView(), D3D11_CLEAR_DEPTH, 1.0f, 0);
			m_D3D->GetDeviceContext()->RSSetViewports(1, &envMapviewport);
			m_D3D->GetDeviceContext()->ClearRenderTargetView(renderTargetView[i], color);

			m_Camera->Render();
			m_Camera->GetViewMatrix(viewMatrix);
			m_D3D->GetWorldMatrix(worldMatrix);
			m_D3D->GetProjectionMatrix(projectionMatrix);

			m_specularIBLShader->SetUpVector(up[i]);
			m_specularIBLShader->SetRoughness(roughness);

			m_D3D->ChangeRasterizerCulling(D3D11_CULL_BACK);
			m_D3D->ChangeDepthStencilComparison(D3D11_COMPARISON_LESS_EQUAL);

			m_convoluteQuadModel->Render(m_D3D->GetDeviceContext());
			m_specularIBLShader->Render(m_D3D->GetDeviceContext(), m_convoluteQuadModel->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix);

			m_D3D->ChangeRasterizerCulling(D3D11_CULL_BACK);
			m_D3D->ChangeDepthStencilComparison(D3D11_COMPARISON_LESS);

			//SaveDDSTextureToFile(m_D3D->GetDeviceContext(), texture2D, filenames[i]);
		}
		//string tmp = "texassemble cube -w ";
		//tmp += std::to_string(mipWidth);
		//tmp += " -h ";
		//tmp += std::to_string(mipWidth);
		//tmp += " -f R8G8B8A8_UNORM -o ";
		//tmp += filenamesCubemap[mip];
		//tmp += " Skyboxes/enviro_posx.dds Skyboxes/enviro_negx.dds Skyboxes/enviro_posy.dds Skyboxes/enviro_negy.dds Skyboxes/enviro_posz.dds Skyboxes/enviro_negz.dds";
		//
		//system(tmp.c_str());
	}
	m_D3D->SetBackBufferRenderTarget();
	m_D3D->ResetViewport();

	return true;
}

bool GraphicsClass::RenderGBufferPosition(RenderTextureClass *targetTex, GBufferShader* shaderToExecute)
{
	targetTex->SetRenderTarget(m_D3D->GetDeviceContext(), m_D3D->GetDepthStencilView());
	targetTex->ClearRenderTarget(m_D3D->GetDeviceContext(), m_D3D->GetDepthStencilView(), 0.0f, 0.0f, 0.0f, 1.0f);

	/////// RENDER SCENE TO BUFFER ///////
	XMMATRIX worldMatrix, viewMatrix, projectionMatrix;
	bool result;

	//Create matrices based on light position
	m_Camera->Render();
	m_D3D->GetWorldMatrix(worldMatrix);
	m_Camera->GetViewMatrix(viewMatrix);
	m_D3D->GetProjectionMatrix(projectionMatrix);

	//Render test buddha
	m_Model->Render(m_D3D->GetDeviceContext());

	worldMatrix = DirectX::XMMatrixMultiply(worldMatrix, XMMatrixTranslation(0.0f, -0.15f, 0.0f));
	worldMatrix = DirectX::XMMatrixMultiply(worldMatrix, DirectX::XMMatrixRotationY(m_Camera->GetRotation().y / 3.14f));
	worldMatrix = DirectX::XMMatrixMultiply(worldMatrix, DirectX::XMMatrixRotationX(m_Camera->GetRotation().x / 3.14f));
	result = shaderToExecute->Render(m_D3D->GetDeviceContext(), m_Model->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix);
	if (!result)
		return false;
	////////////////////////////////////
	//m_renderTexturePreview->BindTexture(targetTex->GetShaderResourceView());

	m_D3D->SetBackBufferRenderTarget();
	m_D3D->ResetViewport();

	return true;
}

bool GraphicsClass::RenderGBufferNormal(RenderTextureClass * targetTex, GBufferShader* shaderToExecute)
{
	//TODO Can use perfect forwarding
	return RenderGBufferPosition(targetTex, shaderToExecute);
}

bool GraphicsClass::RenderGBufferAlbedo(RenderTextureClass * targetTex, GBufferShader* shaderToExecute)
{
	return false;
}

//TODO Redundant methods - rewrite to avoid copying code
bool GraphicsClass::RenderSSAONoiseTexture(RenderTextureClass * targetTex, GBufferShader* shaderToExecute)
{
	targetTex->SetRenderTarget(m_D3D->GetDeviceContext(), m_D3D->GetDepthStencilView());
	targetTex->ClearRenderTarget(m_D3D->GetDeviceContext(), m_D3D->GetDepthStencilView(), 0.0f, 0.0f, 0.0f, 1.0f);

	/////// RENDER SCENE TO BUFFER ///////
	XMMATRIX worldMatrix, viewMatrix, projectionMatrix;
	bool result;

	//Create matrices based on light position
	m_Camera->Render();
	m_D3D->GetWorldMatrix(worldMatrix);
	m_Camera->GetViewMatrix(viewMatrix);
	m_D3D->GetProjectionMatrix(projectionMatrix);

	//Render test buddha
	m_convoluteQuadModel->Render(m_D3D->GetDeviceContext());

	result = shaderToExecute->Render(m_D3D->GetDeviceContext(), m_convoluteQuadModel->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix);
	if (!result)
		return false;
	////////////////////////////////////
	m_renderTexturePreview->BindTexture(targetTex->GetShaderResourceView());

	m_D3D->SetBackBufferRenderTarget();
	m_D3D->ResetViewport();

	return true;
}

bool GraphicsClass::RenderSSAOTexture(RenderTextureClass * targetTex, GBufferShader* shaderToExecute)
{
	targetTex->SetRenderTarget(m_D3D->GetDeviceContext(), m_D3D->GetDepthStencilView());
	targetTex->ClearRenderTarget(m_D3D->GetDeviceContext(), m_D3D->GetDepthStencilView(), 0.0f, 0.0f, 0.0f, 1.0f);

	/////// RENDER SCENE TO BUFFER ///////
	XMMATRIX worldMatrix, viewMatrix, projectionMatrix;
	bool result;

	//Create matrices based on light position
	m_Camera->Render();
	m_D3D->GetWorldMatrix(worldMatrix);
	m_Camera->GetViewMatrix(viewMatrix);
	m_D3D->GetProjectionMatrix(projectionMatrix);

	//Render test buddha
	m_convoluteQuadModel->Render(m_D3D->GetDeviceContext());

	result = shaderToExecute->Render(m_D3D->GetDeviceContext(), m_convoluteQuadModel->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix);
	if (!result)
		return false;
	////////////////////////////////////
	//m_renderTexturePreview->BindTexture(targetTex->GetShaderResourceView());

	m_D3D->SetBackBufferRenderTarget();
	m_D3D->ResetViewport();

	return true;
}

void GraphicsClass::RenderTextureViewImGui(ID3D11Resource *& resource, ID3D11ShaderResourceView *& resourceView, const char* label)
{
	ImGui::Text(label);
	if (ImGui::ImageButton(resourceView == nullptr ? m_emptyTexView[m_internalTextureViewIndex] : resourceView, ImVec2{ 64, 64 }))
	{
		UITexturePreview::TextureChooseWindow(m_D3D, resource, resourceView);
	}
	ImGui::SameLine();
	std::string buttonText = "X";
	for (int i = 0; i < m_internalTextureViewIndex; i++)
	{
		buttonText += ' ';
	}
	if (ImGui::Button(buttonText.c_str(), ImVec2{ 16, 16 }))
	{
		UITexturePreview::DeletePassedTexture(m_D3D, resource, resourceView);
	}

	m_internalTextureViewIndex++;
}

inline void GraphicsClass::RenderTextureViewImGuiEditor(ID3D11Resource *& resource, ID3D11ShaderResourceView *& resourceView, const char * label, std::string& path)
{
	ImGui::Text(label);
	if (ImGui::ImageButton(resourceView == nullptr ? m_emptyTexViewEditor : resourceView, ImVec2{ 64, 64 }))
	{
		path = UITexturePreview::TextureChooseWindow(m_D3D, resource, resourceView);
		std::string curr{};
		for (int i = path.length() - 1; i > -1; --i)
		{
			curr = path[i] + curr;
			if (curr == "LEngine")
			{
				int index = i + 8;
				path.erase(path.begin(), path.begin() + index);
				break;
			}
			else if (path[i] == '\\')
			{
				curr = "";
			}
		}
	}
	ImGui::SameLine();
	std::string buttonText = "X";
	if (ImGui::Button(buttonText.c_str(), ImVec2{ 16, 16 }))
	{
		UITexturePreview::DeletePassedTexture(m_D3D, resource, resourceView);
		path = "";
	}
}

bool GraphicsClass::ApplySSAO(ID3D11ShaderResourceView*& ssaoMap, ID3D11ShaderResourceView*& mainFrameBuffer)
{
	XMMATRIX worldMatrix, viewMatrix, projectionMatrix;
	m_Camera->Render();
	m_Camera->GetViewMatrix(viewMatrix);
	m_D3D->GetWorldMatrix(worldMatrix);
	m_D3D->GetProjectionMatrix(projectionMatrix);

	if (mainFrameBuffer != nullptr)
		m_postProcessShader->SetScreenBuffer(mainFrameBuffer);
	if (ssaoMap != nullptr)
		m_postProcessShader->SetSSAOBuffer(ssaoMap);

	m_convoluteQuadModel->Initialize(m_D3D->GetDevice(), ModelClass::ShapeSize::RECTANGLE, -1.0f, 1.0f, 1.0f, -1.0f, true);
	m_convoluteQuadModel->Render(m_D3D->GetDeviceContext());
	worldMatrix = worldMatrix * 0;
	return m_postProcessShader->Render(m_D3D->GetDeviceContext(), m_convoluteQuadModel->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix);
}

bool GraphicsClass::ApplyBloom(ID3D11ShaderResourceView * bloomTexture, ID3D11ShaderResourceView * mainFrameBuffer)
{
	XMMATRIX worldMatrix, viewMatrix, projectionMatrix;
	m_Camera->Render();
	m_Camera->GetViewMatrix(viewMatrix);
	m_D3D->GetWorldMatrix(worldMatrix);
	m_D3D->GetProjectionMatrix(projectionMatrix);

	if (mainFrameBuffer != nullptr)
		m_postProcessShader->SetScreenBuffer(mainFrameBuffer);
	if (bloomTexture != nullptr)
		m_postProcessShader->SetBloomBuffer(bloomTexture);

	m_convoluteQuadModel->Initialize(m_D3D->GetDevice(), ModelClass::ShapeSize::RECTANGLE, -1.0f, 1.0f, 1.0f, -1.0f, true);
	m_convoluteQuadModel->Render(m_D3D->GetDeviceContext());
	worldMatrix = worldMatrix * 0;
	return m_postProcessShader->Render(m_D3D->GetDeviceContext(), m_convoluteQuadModel->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix);
}

bool GraphicsClass::ApplyLUT(ID3D11ShaderResourceView * lutTexture, ID3D11ShaderResourceView * mainFrameBuffer)
{
	XMMATRIX worldMatrix, viewMatrix, projectionMatrix;
	m_Camera->Render();
	m_Camera->GetViewMatrix(viewMatrix);
	m_D3D->GetWorldMatrix(worldMatrix);
	m_D3D->GetProjectionMatrix(projectionMatrix);

	if (mainFrameBuffer != nullptr)
		m_postProcessShader->SetScreenBuffer(mainFrameBuffer);
	if (lutTexture != nullptr)
		m_postProcessShader->SetLUTBuffer(lutTexture);

	m_convoluteQuadModel->Initialize(m_D3D->GetDevice(), ModelClass::ShapeSize::RECTANGLE, -1.0f, 1.0f, 1.0f, -1.0f, true);
	m_convoluteQuadModel->Render(m_D3D->GetDeviceContext());
	worldMatrix = worldMatrix * 0;
	return m_postProcessShader->Render(m_D3D->GetDeviceContext(), m_convoluteQuadModel->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix);
}

bool GraphicsClass::ApplyChromaticAberration(ID3D11ShaderResourceView * chromaticAberrationTexture, ID3D11ShaderResourceView * mainFrameBuffer)
{
	m_postProcessShader->UseChromaticAberration(true);
	return true;
}

bool GraphicsClass::ApplyGrain(ID3D11ShaderResourceView * grainTexture, ID3D11ShaderResourceView * mainFrameBuffer)
{
	m_postProcessShader->UseGrain(true);
	return true;
}

bool GraphicsClass::CreateShaderEditor()
{
	m_shaderEditorManager = new ShaderEditorManager(m_D3D, m_mouse->GetMouse());
	m_shaderEditorManager->SetRefToClickedOutside(&m_focusOnChoosingWindowsShader);

	//m_shaderEditorManager->AddShaderBlock(new UIShaderEditorBlock({ -0.2f, 0.2f }), 0, 1);
	//m_shaderEditorManager->AddShaderBlock(new UIShaderEditorBlock({ -0.2f, -0.2f }), 0, 1);
	//m_shaderEditorManager->AddShaderBlock(new UIShaderEditorBlock({ 0.2f, 0.0f }), 2, 1);

	return true;
}

float GraphicsClass::lerp(float a, float b, float val)
{
	return a * (1.0f - val) + b * val;
}