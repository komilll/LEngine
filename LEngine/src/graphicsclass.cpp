#include "graphicsclass.h"

bool GraphicsClass::Initialize(int screenWidth, int screenHeight, HWND hwnd)
{
	//Save screen WIDTH x HEIGHT for internal usage
	m_screenWidth = screenWidth;
	m_screenHeight = screenHeight;

	// Create the Direct3D object.
	m_D3D = new D3DClass;

	if (std::ifstream in{ "msaa_settings.txt" })
	{
		in >> m_D3D->MSAA_NUMBER_OF_SAMPLES;
	}

	// Initialize the Direct3D object.
	if(!m_D3D->Initialize(screenWidth, screenHeight, VSYNC_ENABLED, hwnd, FULL_SCREEN, SCREEN_DEPTH, SCREEN_NEAR))
		return false;

	// Create the camera object.
	m_Camera = new CameraClass;

	// Set the initial position of the camera.
	//m_Camera->SetPosition(0.0f, 0.1f, -0.35f); //BUDDA
	//m_Camera->SetPosition(0.0f, 5.0f, -15.0f); //SHADOWMAPPING
	//m_Camera->SetPosition(0.0f, 0.0f, -2.0f); //SINGLE SPHERE
	//MoveCameraForward(1.60f);
	
	// Create the model object.
	m_Model = new ModelClass;
	m_skyboxModel = new ModelClass;
	m_cubeModel = new ModelClass;
	m_previewData.model = new ModelClass;

	//Initialize the model object.
	if (!m_skyboxModel->Initialize(m_D3D, "sphere.obj", false))
		return false;

	if (!m_previewData.model->Initialize(m_D3D, "sphere.obj", false))
		return false;
	CreateAABB(m_previewData.model);
	m_previewData.model->SetPosition(0.0f, 0.0f, 2.0f);
	m_previewData.baseSize.x = m_previewData.model->GetBounds().GetSizeX();
	m_previewData.baseSize.y = m_previewData.model->GetBounds().GetSizeY();
	m_previewData.worldPos = m_previewData.model->GetPositionXYZ();
	m_previewData.bounds = m_previewData.model->GetBounds();

	if (!m_cubeModel->Initialize(m_D3D, "cube.obj"))
		return false;

	//Create mouse container
	m_mouse = new MouseClassContainer;

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
	BaseShaderClass::vertexInputType input = m_D3D->GetBaseInputType();

	m_pbrShader = new ShaderPBRGenerated;
	if (!m_pbrShader->Initialize(m_D3D->GetDevice(), hwnd, L"pbr_used.vs", L"pbr_used.ps", input))
		return false;

	m_pbrShader->AddDirectionalLight(XMFLOAT3{ 0.0f, 10.0f, -5.0f }, 15.0f, 1.0f, 1.0f, 1.0f);
	
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

	{
		ID3D11Resource* tmp{ nullptr };
		for (auto& view : m_emptyTexView)
			UITexturePreview::LoadTexture(m_D3D->GetDevice(), EMPTY_TEX, tmp, view);
		UITexturePreview::LoadTexture(m_D3D->GetDevice(), EMPTY_TEX, tmp, m_emptyTexViewEditor);
	}
#pragma endregion

	if (BLUR_BILINEAR)
	{
#pragma region Blur bilinear screenspace
		//RENDER SCENE TO TEXTURE
		m_renderTexture = new RenderTextureClass;
		if (!(m_renderTexture->Initialize(m_D3D->GetDevice(), screenWidth, screenHeight, 1)))
			return false;

		m_renderTexturePreview = new UITexture;
		if (!(m_renderTexturePreview->Initialize(m_D3D, ModelClass::ShapeSize::RECTANGLE, -1.0f, 1.0f, 1.0f, -1.0f)))
			return false;

		m_renderTexturePreview->BindTexture(m_renderTexture->GetShaderResourceView());

		//Texture downsampled
		m_renderTextureDownsampled = new RenderTextureClass;
		if (!(m_renderTextureDownsampled->Initialize(m_D3D->GetDevice(), screenWidth / 2, screenHeight / 2, 1)))
			return false;

		//Texture upscaled
		m_renderTextureUpscaled = new RenderTextureClass;
		if (!(m_renderTextureUpscaled->Initialize(m_D3D->GetDevice(), screenWidth, screenHeight, 1)))
			return false;

		//Texture horizontal blur
		m_renderTextureHorizontalBlur = new RenderTextureClass;
		if (!(m_renderTextureHorizontalBlur->Initialize(m_D3D->GetDevice(), screenWidth / 2, screenHeight / 2, 1)))
			return false;

		//Texture vertical blur
		m_renderTextureVerticalBlur = new RenderTextureClass;
		if (!(m_renderTextureVerticalBlur->Initialize(m_D3D->GetDevice(), screenWidth / 2, screenHeight / 2, 1)))
			return false;
#pragma endregion
	}
	m_renderTextureMainScene = new RenderTextureClass;
	if (!(m_renderTextureMainScene->Initialize(m_D3D->GetDevice(), screenWidth, screenHeight, m_D3D->MSAA_NUMBER_OF_SAMPLES)))
		return false;

	m_shaderPreview = new RenderTextureClass;
	if (!(m_shaderPreview->Initialize(m_D3D->GetDevice(), screenWidth * 0.25f, screenHeight * 0.25f, 1)))
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
	
	if (std::ifstream skyboxFile{ "Skyboxes/cubemap.dds" })
	{ }
	else
	{
		system("texassemble cube -w 512 -h 512 -f R8G8B8A8_UNORM -o Skyboxes/cubemap.dds Skyboxes/posx.bmp Skyboxes/negx.bmp Skyboxes/posy.bmp Skyboxes/negy.bmp Skyboxes/posz.bmp Skyboxes/negz.bmp");
		if (std::ifstream skyboxFile{ "Skyboxes/cubemap.dds" })
		{ }
		else
		{
			system("texassemble cube -w 512 -h 512 -f R8G8B8A8_UNORM -o Skyboxes/cubemap.dds Skyboxes/posx.jpg Skyboxes/negx.jpg Skyboxes/posy.jpg Skyboxes/negy.jpg Skyboxes/posz.jpg Skyboxes/negz.jpg");
			if (std::ifstream skyboxFile{ "Skyboxes/cubemap.dds" })
			{ }
			else
			{
				return false;
			}
		}
	}
	//LOAD SKYBOX
	m_skyboxShader = new SkyboxShaderClass;
	if (!m_skyboxShader->Initialize(m_D3D->GetDevice(), *m_D3D->GetHWND(), L"skybox.vs", L"skybox.ps", input))
		return false;

	m_skyboxShader->LoadTexture(m_D3D->GetDevice(), L"Skyboxes/cubemap.dds", m_skyboxShader->m_skyboxTexture, m_skyboxShader->m_skyboxTextureView);
	
	//RENDER SCENE TO TEXTURE
	m_renderTexture = new RenderTextureClass;
	if (!(m_renderTexture->Initialize(m_D3D->GetDevice(), CONVOLUTION_DIFFUSE_SIZE, CONVOLUTION_DIFFUSE_SIZE, 1)))
		return false;

	m_renderTexturePreview = new UITexture;
	if (!(m_renderTexturePreview->Initialize(m_D3D, ModelClass::ShapeSize::RECTANGLE, -1.0f, 1.0f, 1.0f, -1.0f)))
		return false;

	//Texture downsampled
	m_skyboxDownsampled = new RenderTextureClass;
	if (!(m_skyboxDownsampled->Initialize(m_D3D->GetDevice(), CONVOLUTION_DIFFUSE_SIZE, CONVOLUTION_DIFFUSE_SIZE, 1, RenderTextureClass::Scaling::NONE)))
		return false;

	m_convoluteShader = new SkyboxShaderClass;
	if (!(m_convoluteShader->Initialize(m_D3D->GetDevice(), *m_D3D->GetHWND(), L"convolutionSkybox.vs", L"convolutionSkybox.ps", input)))
		return false;

	m_convoluteShader->LoadTexture(m_D3D->GetDevice(), L"Skyboxes/cubemap.dds", m_convoluteShader->m_skyboxTexture, m_convoluteShader->m_skyboxTextureView);
	m_convoluteShader->SetType(SkyboxShaderClass::SkyboxType::CONV_DIFFUSE);
	m_convoluteShader->SetUpVector(XMFLOAT3{ 0, 1, 0 });
	ConvoluteShader(m_convoluteShader->m_skyboxTextureView, m_skyboxDownsampled);
	
	//Calculate specular IBL environment prefiltered map
	m_environmentTextureMap = new RenderTextureClass;
	if (!(m_environmentTextureMap->Initialize(m_D3D->GetDevice(), ENVIRONMENT_SPECULAR_SIZE, ENVIRONMENT_SPECULAR_SIZE, 1, RenderTextureClass::Scaling::NONE)))
		return false;

	m_brdfLUT = new RenderTextureClass;
	if (!(m_brdfLUT->Initialize(m_D3D->GetDevice(), 512, 512, 1, RenderTextureClass::Scaling::NONE)))
		return false;
	
	m_specularIBLShader = new SkyboxShaderClass;
	if (!(m_specularIBLShader->Initialize(m_D3D->GetDevice(), *m_D3D->GetHWND(), L"environmentPrefilteredMap.vs", L"environmentPrefilteredMap.ps", input)))
		return false;

	m_specularIBLShader->LoadTexture(m_D3D->GetDevice(), L"Skyboxes/cubemap.dds", m_specularIBLShader->m_skyboxTexture, m_specularIBLShader->m_skyboxTextureView);
	m_specularIBLShader->SetType(SkyboxShaderClass::SkyboxType::ENVIRO);
	m_specularIBLShader->SetUpVector(XMFLOAT3{ 0, 1, 0 });

	m_brdfLutShader = new SkyboxShaderClass;
	if (!(m_brdfLutShader->Initialize(m_D3D->GetDevice(), *m_D3D->GetHWND(), L"SpecularLookupIBL.vs", L"SpecularLookupIBL.ps", input)))
		return false;

	PrepareEnvironmentPrefilteredMap(m_specularIBLShader->m_skyboxTextureView, m_environmentTextureMap);
	CreateSingleEnvironmentMap();

	//Skybox texture previews
	//TODO Use contructors
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
		return false;
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
	m_positionBuffer->Initialize(m_D3D->GetDevice(), m_screenWidth, m_screenHeight, 1);
	
	m_normalBuffer = new RenderTextureClass;
	m_normalBuffer->Initialize(m_D3D->GetDevice(), m_screenWidth, m_screenHeight, 1);

	m_albedoBuffer = new RenderTextureClass;
	m_albedoBuffer->Initialize(m_D3D->GetDevice(), m_screenWidth, m_screenHeight, 1);

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
	const std::uniform_real_distribution<float> randomFloats{ 0.0f, 1.0f };
	std::default_random_engine generator;
	XMFLOAT3 tmpSample;
	std::array<XMFLOAT3, 64> ssaoKernel;
	std::array<XMFLOAT2, 16> ssaoNoise;

	for (int i = 0; i < SSAO_KERNEL_SIZE; i++)
	{
		//Generate random vector3 ([-1, 1], [-1, 1], [0, 1])
		XMVECTOR tmpVector = { randomFloats(generator) * 2.0f - 1.0f, randomFloats(generator) * 2.0f - 1.0f, randomFloats(generator) };
		//Normalize vector3
		tmpVector = XMVector3Normalize(tmpVector);

		//Scale samples so they are more aligned to middle of hemisphere
		float scale = float(i) / 64.0f;
		scale = lerp(0.1f, 1.0f, scale * scale);
		tmpVector.m128_f32[0] *= scale;
		tmpVector.m128_f32[1] *= scale;
		tmpVector.m128_f32[2] *= scale;

		//Pass value to array
		ssaoKernel[i] = { tmpVector.m128_f32[0], tmpVector.m128_f32[1], tmpVector.m128_f32[2] };
	}

	for (int i = 0; i < 16; ++i)
	{
		ssaoNoise[i] = { randomFloats(generator), randomFloats(generator) };
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
	if (!m_ssaoTexture->Initialize(m_D3D->GetDevice(), screenWidth, screenHeight, 1))
		return false;

	m_postSSAOTexture = new RenderTextureClass;
	if (!m_postSSAOTexture->Initialize(m_D3D->GetDevice(), screenWidth, screenHeight, 1))
		return false;

	m_ssaoNoiseTexture = new RenderTextureClass;
	m_ssaoNoiseTexture->Initialize(m_D3D->GetDevice(), SSAO_NOISE_SIZE, SSAO_NOISE_SIZE, 1);
	if (!m_ssaoNoiseTexture->LoadTexture(m_D3D->GetDevice(), L"ssaoNoise.dds", m_ssaoNoiseTexture->GetShaderResource(), m_ssaoNoiseTexture->GetShaderResourceView()))
		return false;
	m_GBufferShaderSSAO->SetKernelValues(ssaoKernel);
	m_GBufferShaderSSAO->SetNoiseValues(ssaoNoise);
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
	if (!m_bloomHelperTexture->Initialize(m_D3D->GetDevice(), m_screenWidth, m_screenHeight, 1))
		return false;

	m_bloomHorizontalBlur = new RenderTextureClass;
	if (!m_bloomHorizontalBlur->Initialize(m_D3D->GetDevice(), m_screenWidth, m_screenHeight, 1))
		return false;

	m_bloomVerticalBlur = new RenderTextureClass;
	if (!m_bloomVerticalBlur->Initialize(m_D3D->GetDevice(), m_screenWidth, m_screenHeight, 1))
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

#pragma region FXAA
	m_fxaaShader = new FXAAShader;
	if (!m_fxaaShader->Initialize(m_D3D->GetDevice(), *m_D3D->GetHWND(), L"fxaa.vs", L"fxaa.ps", input))
		return false;

	m_antialiasedTexture = new RenderTextureClass;
	if (!m_antialiasedTexture->Initialize(m_D3D->GetDevice(), m_screenWidth, m_screenHeight, 1))
		return false;

	m_ssaaTexture = new RenderTextureClass;
	if (!m_ssaaTexture->Initialize(m_D3D->GetDevice(), m_screenWidth * 2.0f, m_screenHeight * 2.0f, 1))
		return false;

	m_ssaaTextureBlurred_2 = new RenderTextureClass;
	if (!m_ssaaTextureBlurred_2->Initialize(m_D3D->GetDevice(), m_screenWidth * 2.0f, m_screenHeight * 2.0f, 1))
		return false;

	m_ssaaTextureBlurredHor = new RenderTextureClass;
	if (!m_ssaaTextureBlurredHor->Initialize(m_D3D->GetDevice(), m_screenWidth, m_screenHeight, 1))
		return false;

	m_ssaaTextureBlurredVer = new RenderTextureClass;
	if (!m_ssaaTextureBlurredVer->Initialize(m_D3D->GetDevice(), m_screenWidth, m_screenHeight, 1))
		return false;
#pragma endregion

	//TODO Replace by empty or load last scene
	LoadScene("test.txt");

	m_modelPicker = new ModelPicker(m_D3D);

	m_singleColorShader = new ModelPickerShader;
	if (!m_singleColorShader->Initialize(m_D3D->GetDevice(), *m_D3D->GetHWND(), L"modelpicker.vs", L"modelpicker.ps", m_D3D->GetBaseInputType()))
		return false;

	return true;
}

void GraphicsClass::Shutdown()
{
	//TODO Upgrade or remove completely
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
	static int timer = 0;
	if (timer++ % 300 == 0)
	{
		timer = 0;
		SaveScene("test.txt");
	}

	return Render();
}

void GraphicsClass::SetCameraPosition(XMFLOAT3 position)
{
	m_Camera->SetPosition(position);
}

void GraphicsClass::SetCameraRotation(XMFLOAT3 rotation)
{
	m_Camera->SetRotation(rotation.x, rotation.y, rotation.z);
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

void GraphicsClass::AddPreviewCameraRotation(float x, float y)
{
	if (m_previewData.model)
	{
		m_previewData.screenPos.x += x;
		m_previewData.screenPos.y += y;
		m_previewData.model->SetPosition(cos(m_previewData.screenPos.x) * m_previewData.worldPos.z, cos(m_previewData.screenPos.y) * 0.0f, sin(m_previewData.screenPos.x) * m_previewData.worldPos.z);
	}
}

void GraphicsClass::UpdateUI()
{
	m_D3D->EnableAlphaBlending();
	if (m_shaderEditorManager)
	{
		m_shaderEditorManager->UpdateBlocks();
	}
	m_D3D->DisableAlphaBlending();
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
	if (!m_mouse->GetModel())
		m_mouse->InitializeMouse();

	if (!CreateShaderEditor())
		PostQuitMessage(0);
}

MouseClass* GraphicsClass::GetMouse()
{
	if (!m_mouse)
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
	if (!RENDER_MATERIAL_EDITOR)
	{
		ReinitializeMainModel();
	}
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

void GraphicsClass::CopyBlocks()
{
	m_shaderEditorManager->CopyBlocks();
}

void GraphicsClass::PasteBlocks()
{
	m_shaderEditorManager->PasteBlocks();
}

bool GraphicsClass::MouseAboveEditorPreview() const
{
	return m_shaderEditorManager->MouseAbovePreview();
}

std::pair<float, float> GraphicsClass::GetCurrentMousePosition() const
{
	return m_shaderEditorManager->GetCurrentMousePosition();
}

bool GraphicsClass::Render()
{
	XMMATRIX worldMatrix, viewMatrix, projectionMatrix;

	// Clear the buffers to begin the scene.
	m_D3D->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

	// Generate the view matrix based on the camera's position.
	m_Camera->Render();
	m_Camera->GetViewMatrix(viewMatrix);
	m_D3D->GetWorldMatrix(worldMatrix);
	m_D3D->GetProjectionMatrix(projectionMatrix);

	if (BLUR_BILINEAR)
	{
		//TODO Test if blur is working
		if (!RenderSceneToTexture())
			return false;

		DownsampleTexture();
		BlurFilterScreenSpace(false);
		BlurFilterScreenSpace(true);
		UpscaleTexture();

		if (!m_renderTexturePreview->Render(m_D3D->GetDeviceContext(), 0, worldMatrix, viewMatrix, projectionMatrix))
			return false;
	}
	else
	{
		RefreshModelTick();

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
			if (m_postprocessFXAA) //TODO Currently working on previous frame
			{
				m_fxaaShader->SetScreenBuffer(m_renderTextureMainScene->GetShaderResourceView());
				if (!RenderFXAATexture(m_antialiasedTexture))
					return false;
			}

			m_D3D->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

			//STANDARD SCENE RENDERING		
			if (!RenderScene())
				return false;

			if (m_postprocessFXAA)
			{
				m_renderTexturePreview->BindTexture(m_antialiasedTexture->GetShaderResourceView());
				if (!m_renderTexturePreview->Render(m_D3D->GetDeviceContext(), 0, worldMatrix, viewMatrix, projectionMatrix))
					return false;
			}
		}
		else
		{
			m_D3D->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

			if (!RenderMaterialPreview())
				return false;
			
			m_renderTexturePreview->BindTexture(m_shaderPreview->GetShaderResourceView());
			if (!m_renderTexturePreview->Render(m_D3D->GetDeviceContext(), 0, worldMatrix, viewMatrix, projectionMatrix))
				return false;

			m_D3D->ResetViewport();
			m_D3D->TurnZBufferOff();
			UpdateUI();
			m_D3D->TurnZBufferOn();
		}

	//PREVIEW SKYBOX IN 6 FACES FORM
		//m_skyboxPreviewRight->Render(m_D3D->GetDeviceContext(), m_groundQuadModel->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix);
		//m_skyboxPreviewLeft->Render(m_D3D->GetDeviceContext(), m_groundQuadModel->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix);
		//m_skyboxPreviewUp->Render(m_D3D->GetDeviceContext(), m_groundQuadModel->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix);
		//m_skyboxPreviewDown->Render(m_D3D->GetDeviceContext(), m_groundQuadModel->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix);
		//m_skyboxPreviewForward->Render(m_D3D->GetDeviceContext(), m_groundQuadModel->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix);
		//m_skyboxPreviewBack->Render(m_D3D->GetDeviceContext(), m_groundQuadModel->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix);
	}


	if (m_postprocessMSAA)
	{
		//static RenderTextureClass *renderTexture;
		//if (renderTexture == nullptr)
		//{
		//	renderTexture = new RenderTextureClass;
		//	renderTexture->Initialize(m_D3D->GetDevice(), m_screenWidth, m_screenHeight, 1);
		//}
		//m_D3D->GetDeviceContext()->ResolveSubresource(renderTexture->GetShaderResource(), 0, m_renderTextureMainScene->GetShaderResource(), 0, DXGI_FORMAT_R16G16B16A16_FLOAT);

		//m_renderTexturePreview->BindTexture(renderTexture->GetShaderResourceView());
		//if (!m_renderTexturePreview->Render(m_D3D->GetDeviceContext(), 0, worldMatrix, viewMatrix, projectionMatrix))
		//	return false;

		m_renderTexturePreview->BindTexture(m_renderTextureMainScene->GetShaderResourceView());
		if (!m_renderTexturePreview->Render(m_D3D->GetDeviceContext(), 0, worldMatrix, viewMatrix, projectionMatrix))
			return false;
	}
	else if (m_postprocessSSAA)
	{
		if (m_antialiasingSettings.sampleCount == 2)
		{
			constexpr float size = 1.0f;
			constexpr int depthStencilViewSize = 1;
			if (!DownsampleTexture(m_renderTextureMainScene, m_ssaaTexture, size, depthStencilViewSize))
				return false;
		}
		else if (m_antialiasingSettings.sampleCount == 4)
		{
			constexpr float size = 1.0f;
			constexpr int depthStencilViewSize = 1;
			if (!DownsampleTexture(m_renderTextureMainScene, m_ssaaTexture, size, depthStencilViewSize))
				return false;
		}
		else if (m_antialiasingSettings.sampleCount == 8)
		{
			constexpr float size = 1.0f;
			constexpr int depthStencilViewSize = 1;
			if (!DownsampleTexture(m_renderTextureMainScene, m_ssaaTexture, size, depthStencilViewSize))
				return false;
		}

		//Back to normal render targets
		m_D3D->SetBackBufferRenderTarget();
		if (!(m_renderTexturePreview->Initialize(m_D3D, ModelClass::ShapeSize::RECTANGLE, -1.0f, 1.0f, 1.0f, -1.0f)))
			return false;

		//Render antialiased buffer
		m_renderTexturePreview->BindTexture(m_renderTextureMainScene->GetShaderResourceView());
		if (!m_renderTexturePreview->Render(m_D3D->GetDeviceContext(), 0, worldMatrix, viewMatrix, projectionMatrix))
			return false;
	}
	else
	{
		m_renderTexturePreview->BindTexture(m_renderTextureMainScene->GetShaderResourceView());
		if (!m_renderTexturePreview->Render(m_D3D->GetDeviceContext(), 0, worldMatrix, viewMatrix, projectionMatrix))
			return false;
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

	if (!RenderScene())
		return false;

	m_D3D->SetBackBufferRenderTarget();

	return true;
}

bool GraphicsClass::RenderScene()
{
	XMMATRIX worldMatrix, viewMatrix, projectionMatrix, lightViewMatrix, lightProjectionMatrix;

	m_Camera->Render();

	//RENDER MAIN SCENE MODEL
	m_D3D->ChangeRasterizerCulling(D3D11_CULL_BACK);
	m_D3D->ChangeDepthStencilComparison(D3D11_COMPARISON_LESS_EQUAL);

	m_Camera->GetViewMatrix(viewMatrix);
	m_D3D->GetWorldMatrix(worldMatrix);
	m_D3D->GetProjectionMatrix(projectionMatrix);

	if (RENDER_MATERIAL_EDITOR)
	{
		m_shaderPreview->SetRenderTarget(m_D3D->GetDeviceContext(), m_D3D->GetDepthStencilView());
		m_shaderPreview->ClearRenderTarget(m_D3D->GetDeviceContext(), m_D3D->GetDepthStencilView(), 0.0f, 0.0f, 0.0f, 1.0f);
	}
	else if (m_postprocessSSAA)
	{
		m_ssaaTexture->SetRenderTarget(m_D3D->GetDeviceContext(), m_D3D->GetDepthStencilView(m_antialiasingSettings.sampleCount));
		m_ssaaTexture->ClearRenderTarget(m_D3D->GetDeviceContext(), m_D3D->GetDepthStencilView(m_antialiasingSettings.sampleCount), 0.0f, 0.0f, 0.0f, 1.0f);
	}
	else
	{
		m_renderTextureMainScene->SetRenderTarget(m_D3D->GetDeviceContext(), m_D3D->GetDepthStencilView());
		m_renderTextureMainScene->ClearRenderTarget(m_D3D->GetDeviceContext(), m_D3D->GetDepthStencilView(), 0.0f, 0.0f, 0.0f, 1.0f);
	}

	m_singleColorShader->ChangeColor(0.7f, 0.7f, 0.7f, 1.0f);
	for (ModelClass* const& model : m_sceneModels)
	{
		if (!model->GetMaterial())
		{
			//TODO Create function - repeated in code many times
			if (m_materialList.find("gold") == m_materialList.end())
			{
				m_materialList.insert({ "gold", new MaterialPrefab{ "gold", m_D3D } });
			}
			model->SetMaterial(m_materialList.at("gold"));
			continue;
		}

		m_Camera->GetViewMatrix(viewMatrix);
		m_D3D->GetWorldMatrix(worldMatrix);
		m_D3D->GetProjectionMatrix(projectionMatrix);
		
		worldMatrix = DirectX::XMMatrixMultiply(worldMatrix, DirectX::XMMatrixScaling(model->GetScale().x, model->GetScale().y, model->GetScale().z));
		worldMatrix = DirectX::XMMatrixMultiply(worldMatrix, DirectX::XMMatrixRotationX(model->GetRotation().x * 0.0174532925f));
		worldMatrix = DirectX::XMMatrixMultiply(worldMatrix, DirectX::XMMatrixRotationY(model->GetRotation().y * 0.0174532925f));
		worldMatrix = DirectX::XMMatrixMultiply(worldMatrix, DirectX::XMMatrixRotationZ(model->GetRotation().z * 0.0174532925f));
		worldMatrix = DirectX::XMMatrixMultiply(worldMatrix, XMMatrixTranslation(model->GetPosition().x, model->GetPosition().y, model->GetPosition().z));

		model->GetMaterial()->GetShader()->m_cameraPosition = m_Camera->GetPosition();
		model->Render(m_D3D->GetDeviceContext());
		if (!model->GetMaterial()->GetShader()->Render(m_D3D->GetDeviceContext(), model->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix))
			return false;

		//TODO Add flag for rendering wireframe
		m_D3D->ChangeRasterizerCulling(D3D11_CULL_NONE);
		for (const auto& wireframe : model->GetWireframeList())
		{
			m_Camera->GetViewMatrix(viewMatrix);
			m_D3D->GetWorldMatrix(worldMatrix);
			m_D3D->GetProjectionMatrix(projectionMatrix);

			worldMatrix = DirectX::XMMatrixMultiply(worldMatrix, DirectX::XMMatrixScaling(model->GetScale().x, model->GetScale().y, model->GetScale().z));
			worldMatrix = DirectX::XMMatrixMultiply(worldMatrix, DirectX::XMMatrixRotationX(model->GetRotation().x * 0.0174532925f));
			worldMatrix = DirectX::XMMatrixMultiply(worldMatrix, DirectX::XMMatrixRotationY(model->GetRotation().y * 0.0174532925f));
			worldMatrix = DirectX::XMMatrixMultiply(worldMatrix, DirectX::XMMatrixRotationZ(model->GetRotation().z * 0.0174532925f));
			worldMatrix = DirectX::XMMatrixMultiply(worldMatrix, XMMatrixTranslation(model->GetPosition().x, model->GetPosition().y, model->GetPosition().z));

			wireframe->Render(m_D3D->GetDeviceContext());
			if(!m_singleColorShader->Render(m_D3D->GetDeviceContext(), wireframe->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix))
				return false;
		}
		m_D3D->ChangeRasterizerCulling(D3D11_CULL_BACK);
	}

	if (DRAW_SKYBOX)
	{
		if (RenderSkybox() == false)
			return false;
	}
	m_D3D->SetBackBufferRenderTarget();

	//TODO Test post-process stack
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
		//TODO Every frame initialization?
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
		constexpr int numberOfBlurRepeats{ 10 };
		for (int i = 0; i < numberOfBlurRepeats; i++)
		{
			BlurFilterScreenSpace(false, m_bloomVerticalBlur, m_bloomHorizontalBlur, m_screenWidth / 2); //Blur horizontal
			BlurFilterScreenSpace(true, m_bloomHorizontalBlur, m_bloomVerticalBlur, m_screenHeight / 2); //Blur vertical
		}

		ApplyBloom(m_bloomVerticalBlur->GetShaderResourceView(), m_renderTextureMainScene->GetShaderResourceView());
	}

	//if (!m_postprocessSSAO && !m_postprocessBloom && !m_postprocessLUT)
	//{
	//	//m_convoluteQuadModel->Initialize(m_D3D->GetDevice(), ModelClass::ShapeSize::RECTANGLE, -1.0f, 1.0f, 1.0f, -1.0f, true);
	//	m_convoluteQuadModel->Render(m_D3D->GetDeviceContext());

	//	m_Camera->Render();
	//	m_Camera->GetViewMatrix(viewMatrix);
	//	m_D3D->GetWorldMatrix(worldMatrix);
	//	m_D3D->GetProjectionMatrix(projectionMatrix);

	//	m_renderTexturePreview->BindTexture(m_renderTextureMainScene->GetShaderResourceView());
	//	if (!m_renderTexturePreview->Render(m_D3D->GetDeviceContext(), 0, worldMatrix, viewMatrix, projectionMatrix))
	//		return false;
	//}
	
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

	if (m_postprocessBloom)
	{
		m_bloomHorizontalBlur->ClearRenderTarget(m_D3D->GetDeviceContext(), m_D3D->GetDepthStencilView(), 0.0f, 0.0f, 0.0f, 1.0f);
		m_bloomVerticalBlur->ClearRenderTarget(m_D3D->GetDeviceContext(), m_D3D->GetDepthStencilView(), 0.0f, 0.0f, 0.0f, 1.0f);
	}

	if (m_postprocessVignette)
	{
		//m_ssaoTexture->ClearRenderTarget(m_D3D->GetDeviceContext(), m_D3D->GetDepthStencilView(), 0.0f, 0.0f, 0.0f, 1.0f);
		m_renderTextureMainScene->SetRenderTarget(m_D3D->GetDeviceContext(), nullptr);

		m_Camera->Render();
		m_Camera->GetViewMatrix(viewMatrix);
		m_D3D->GetWorldMatrix(worldMatrix);
		m_D3D->GetProjectionMatrix(projectionMatrix);

		m_convoluteQuadModel->Render(m_D3D->GetDeviceContext());

		m_D3D->EnableAlphaBlending();

		m_renderTexturePreview->BindTexture(m_vignetteShader->m_vignetteResourceView);
		if (!m_renderTexturePreview->Render(m_D3D->GetDeviceContext(), 0, worldMatrix, viewMatrix, projectionMatrix))
			return false;

		m_D3D->DisableAlphaBlending();
		m_D3D->SetBackBufferRenderTarget();
	}

	if (m_postprocessSSAO || m_postprocessBloom || m_postprocessLUT || m_postprocessChromaticAberration || m_postprocessGrain)
	{
		if (!RenderPostprocess(m_renderTextureMainScene->GetShaderResourceView()))
		{
			return false;
		}
	}

	//worldMatrix = XMMatrixTranslation(0.5f, 0.0f, -1.0f);
	//worldMatrix = XMMatrixMultiply(worldMatrix, XMMatrixScaling(4.0f, 6.0f, 1.0f));
	//m_cubeModel->Render(m_D3D->GetDeviceContext());
	//m_colorShader->Render(m_D3D->GetDeviceContext(), m_cubeModel->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix);

	//TODO SHADOWMAPPING presentation
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

	//Draw model picker
	m_D3D->TurnZBufferOff();
	if (m_selectedModel)
	{
		m_D3D->SetBackBufferRenderTarget();
		m_Camera->GetViewMatrix(viewMatrix);
		m_D3D->GetWorldMatrix(worldMatrix);
		m_D3D->GetProjectionMatrix(projectionMatrix);
		if (!m_modelPicker->Render(m_D3D->GetDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, m_selectedModel))
			return false;
	}
	m_D3D->TurnZBufferOn();

	return true;
}

bool GraphicsClass::RenderMaterialPreview()
{
	const std::string materialName = m_shaderEditorManager->GetCurrentMaterialName();
	if (materialName == "")
	{
		return true;
	}
	if (m_materialList.find(materialName) == m_materialList.end())
	{
		m_materialList.insert({ materialName, new MaterialPrefab{ materialName, m_D3D } });
		return true;
	}
	MaterialPrefab* const material = m_materialList.at(materialName);

	XMMATRIX worldMatrix, viewMatrix, projectionMatrix;

	//RENDER MAIN SCENE MODEL
	m_D3D->ChangeRasterizerCulling(D3D11_CULL_BACK);
	m_D3D->ChangeDepthStencilComparison(D3D11_COMPARISON_LESS_EQUAL);

	m_shaderPreview->SetRenderTarget(m_D3D->GetDeviceContext(), m_D3D->GetDepthStencilView());
	m_shaderPreview->ClearRenderTarget(m_D3D->GetDeviceContext(), m_D3D->GetDepthStencilView(), 0.0f, 0.0f, 0.0f, 1.0f);

	ModelClass* const& model = m_previewData.model;
	XMFLOAT3 tmp = m_Camera->GetPosition();
	m_Camera->SetPosition(0.0f, m_previewData.yCameraPos, 0.0f);
	{
		m_Camera->RenderPreview({ model->GetPositionXYZ().x, model->GetPositionXYZ().y, model->GetPositionXYZ().z });
		m_Camera->GetViewPreviewMatrix(viewMatrix);
		//m_Camera->Render();
		//m_Camera->GetViewMatrix(viewMatrix);
		m_D3D->GetWorldMatrix(worldMatrix);
		m_D3D->GetProjectionMatrix(projectionMatrix);

		worldMatrix = DirectX::XMMatrixMultiply(worldMatrix, DirectX::XMMatrixScaling(model->GetScale().x, model->GetScale().y, model->GetScale().z));
		worldMatrix = DirectX::XMMatrixMultiply(worldMatrix, DirectX::XMMatrixRotationX(model->GetRotation().x * 0.0174532925f));
		worldMatrix = DirectX::XMMatrixMultiply(worldMatrix, DirectX::XMMatrixRotationY(model->GetRotation().y * 0.0174532925f));
		worldMatrix = DirectX::XMMatrixMultiply(worldMatrix, DirectX::XMMatrixRotationZ(model->GetRotation().z * 0.0174532925f));
		worldMatrix = DirectX::XMMatrixMultiply(worldMatrix, XMMatrixTranslation(model->GetPosition().x, model->GetPosition().y, model->GetPosition().z));

		material->GetShader()->m_cameraPosition = m_Camera->GetPosition();
		model->Render(m_D3D->GetDeviceContext());
		if (!material->GetShader()->Render(m_D3D->GetDeviceContext(), model->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix))
			return false;
	}
	m_Camera->SetPosition(tmp.x, tmp.y, tmp.z);

	if (DRAW_SKYBOX)
	{
		if (RenderSkybox() == false)
			return false;
	}
	m_D3D->SetBackBufferRenderTarget();

	//TODO Post-process stack duplicated from RenderScene()

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
		if (!m_renderTexturePreview->Render(m_D3D->GetDeviceContext(), 0, worldMatrix, viewMatrix, projectionMatrix))
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

	if (m_postprocessBloom)
	{
		m_bloomHorizontalBlur->ClearRenderTarget(m_D3D->GetDeviceContext(), m_D3D->GetDepthStencilView(), 0.0f, 0.0f, 0.0f, 1.0f);
		m_bloomVerticalBlur->ClearRenderTarget(m_D3D->GetDeviceContext(), m_D3D->GetDepthStencilView(), 0.0f, 0.0f, 0.0f, 1.0f);
	}

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
		if (!m_renderTexturePreview->Render(m_D3D->GetDeviceContext(), 0, worldMatrix, viewMatrix, projectionMatrix))
			return false;

		m_D3D->DisableAlphaBlending();
	}

	if (m_postprocessBloom)
	{
		//Clear to make sure there is no pointer to local variable which outlives scope
		m_bloomShader->m_bloomTexture = nullptr;
		m_bloomShader->m_bloomTextureView = nullptr;
	}

	//Draw model picker
	m_D3D->TurnZBufferOff();
	if (m_selectedModel)
	{
		m_Camera->GetViewMatrix(viewMatrix);
		m_D3D->GetWorldMatrix(worldMatrix);
		m_D3D->GetProjectionMatrix(projectionMatrix);
		if (!m_modelPicker->Render(m_D3D->GetDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, m_selectedModel))
			return false;
	}
	m_D3D->TurnZBufferOn();

	return true;
}

bool GraphicsClass::RenderGUI()
{
	//Load ImGUI methods
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	PassMouseInfo(m_mouse->GetMouse()->GetLMBPressed(), m_mouse->GetMouse()->GetRMBPressed());

	//Render material-choose window
	if (!RENDER_MATERIAL_EDITOR)
	{
		ImGui::SetNextWindowPos({ 1042, 24 });
		ImGui::SetNextWindowSize({ 226, 659 });
		ImGui::Begin("Scene manager", (bool*)0, ImGuiWindowFlags_::ImGuiWindowFlags_NoMove | ImGuiWindowFlags_::ImGuiWindowFlags_NoResize);
		if (ImGui::CollapsingHeader("Scene hierarchy"))
		{
			for (ModelClass* const& model : m_sceneModels)
			{
				if (ImGui::Selectable(model->m_name.c_str(), model->m_selected))
				{
					m_selectedModel = model;
					break;
				}
			}
		}

		if (ImGui::CollapsingHeader("Materials explorer"))
		{
			const float windowLineWidth = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;
			{
				int materialIndex = 0;
				for (const auto& name : m_shaderEditorManager->GetAllMaterialNames())
				{
					std::string tmp;
					int i{ 0 };
					for (const auto& c : name)
					{
						tmp += c;
						i++;
						if (i % 8 == 0)
						{
							i = 0;
							tmp += "\n";
						}
					}
					if (ImGui::Button(tmp.c_str(), ImVec2{ 64, 64 }))
					{
						m_shaderEditorManager->LoadMaterial(materialIndex);
						m_shaderEditorManager->m_refreshModel = true;
					}
					const float lastButtonPos = ImGui::GetItemRectMax().x;
					const float nextButtonPos = lastButtonPos + ImGui::GetStyle().ItemSpacing.x + 64;
					if (nextButtonPos < windowLineWidth)
					{
						ImGui::SameLine();
					}
					materialIndex++;
				}
			}
		}
		m_shaderEditorManager->ResetMouseHoveredOnImGui();
		m_shaderEditorManager->UpdateMouseHoveredOnImGui(ImGui::IsWindowHovered() || ImGui::IsWindowFocused());
		ImGui::End();
	}
	if (RENDER_MATERIAL_EDITOR && m_shaderEditorManager)
	{
		m_shaderEditorManager->ResetMouseHoveredOnImGui();
		//Base window of editor - flow control/variables
		{
			ImGui::SetNextWindowPos({ 2, 180 });
			ImGui::SetNextWindowSize({ 318, 541 });
			ImGui::Begin(m_shaderEditorManager->IsWorkingOnSavedMaterial() ? m_shaderEditorManager->m_materialToSaveName.data() : "Shader editor", (bool*)0, 
				ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
			if (ImGui::Button("Change preview model"))
			{
				ModelClass* model = new ModelClass;
				if (model->Initialize(m_D3D, model->LoadModelCalculatePath().c_str()))
				{
					if (m_materialList.find("gold") == m_materialList.end())
					{
						m_materialList.insert({ "gold", new MaterialPrefab{ "gold", m_D3D } });
					}
					model->SetMaterial(m_materialList.at("gold"));

					CreateAABB(model);
					if (m_previewData.model)
					{
						delete m_previewData.model;
					}
					m_previewData.model = model;
					CreateAABB(m_previewData.model);
					const Bounds oldBounds = m_previewData.bounds;
					m_previewData.bounds = m_previewData.model->GetBounds();

					const XMFLOAT2 size = { m_previewData.model->GetBounds().GetSizeX(), m_previewData.model->GetBounds().GetSizeY() };
					const float multiplier = min(size.x / m_previewData.baseSize.x, size.y / m_previewData.baseSize.y);

					const float yPosDiff = (oldBounds.GetSizeY() - m_previewData.bounds.GetSizeY()) * 0.5f;
					m_previewData.yCameraPos = yPosDiff;
					m_previewData.worldPos = XMFLOAT3{ 0.0f, 0.0f, /*multiplier * m_previewData.worldPos.z*/ 2.0f };
				}
			}
			if (ImGui::Button("Generate shader"))
			{
				m_shaderEditorManager->GenerateCodeToFile();
				ReinitializeMainModel();
			}
			ImGui::Spacing();
			if (!m_shaderEditorManager->IsWorkingOnSavedMaterial())
			{
				ImGui::Text("Material name:");
				ImGui::InputText("", const_cast<char*>(m_shaderEditorManager->m_materialToSaveName.data()), 30);
			}
			if (ImGui::Button("Save material"))
			{
				m_shaderEditorManager->SaveMaterial(m_shaderEditorManager->m_materialToSaveName);
			}
			ImGui::Spacing();
			ImGui::Spacing();

			//Show scalar options
			if (m_shaderEditorManager->m_focusedBlock && m_shaderEditorManager->m_focusedBlock->GetInputCount() == 0
				&& m_shaderEditorManager->m_focusedBlock->GetFirstOutputNode())
			{
				RenderInputForMaterial(m_shaderEditorManager->m_focusedBlock, true, true);
			}
			else
			{
				for (const auto& in : m_shaderEditorManager->GetMaterialInputs())
				{
					if (!in || !in->GetFirstOutputNode())
					{
						continue;
					}
					if (!in->GetFirstOutputNode()->m_isVariable)
					{
						continue;
					}

					const EMaterialInputResult result = RenderInputForMaterial(in, false, m_shaderEditorManager->GetPickingColorElement() == in->GetFirstOutputNode());
					if (result == EMaterialInputResult::StopOthers)
					{
						m_shaderEditorManager->SetPickingColorElement(in->GetFirstOutputNode());
						break;
					}
					else if (result == EMaterialInputResult::Break)
					{
						break;
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

	ImGui::Begin("BaseWindow", (bool*)0, ImGuiWindowFlags_::ImGuiWindowFlags_NoMove | ImGuiWindowFlags_::ImGuiWindowFlags_NoResize);
	if (m_selectedModel)
	{
		ImGui::InputFloat3("Position", m_selectedModel->GetPositionRef(), "%.2f");
		ImGui::SliderFloat3("Scale", m_selectedModel->GetScaleRef(), 0.0f, 5.0f, "%.2f");
		ImGui::SliderFloat3("Rotation", m_selectedModel->GetRotationRef(), 0.0f, 180.0f, "%.1f");
		if (ImGui::InputText("Name", const_cast<char*>(m_selectedModel->m_name.data()), 30))
		{
			m_selectedModel->SaveVisibleName();
		}
		if (ImGui::Button("Load model"))
		{
			m_selectedModel->LoadModel();
		}
		if (ImGui::Button("Delete model"))
		{
			m_sceneModels.erase(std::find(m_sceneModels.begin(), m_sceneModels.end(), m_selectedModel));
			if (m_selectedModel)
			{
				delete m_selectedModel;
				m_selectedModel = nullptr;
			}
		}

		if (m_selectedModel && m_selectedModel->GetMaterial())
		{
			if (ImGui::BeginCombo("Model material", m_selectedModel->GetMaterial()->GetName().c_str()))
			{
				for (const auto& name : m_shaderEditorManager->GetAllMaterialNames())
				{
					if (ImGui::Selectable(name.c_str()))
					{
						if (m_materialList.find(name) == m_materialList.end()) //Material not found, create new one
						{
							m_materialList.insert({ name, new MaterialPrefab{ name, m_D3D } });
						}
						m_selectedModel->SetMaterial(m_materialList.at(name));
						break;
					}
				}
				if (!m_shaderEditorManager->IsMouseHoveredOnImGui())
				{
					m_shaderEditorManager->ResetMouseHoveredOnImGui();
					m_shaderEditorManager->UpdateMouseHoveredOnImGui(ImGui::IsWindowHovered() || ImGui::IsWindowFocused());
				}
				ImGui::EndCombo();
			}
		}
		else if (m_selectedModel)
		{
			if (m_materialList.find("gold") == m_materialList.end())
			{
				m_materialList.insert({ "gold", new MaterialPrefab{ "gold", m_D3D } });
			}
			m_selectedModel->SetMaterial(m_materialList.at("gold"));
		}
	}
	else
	{
		if (ImGui::Button("Add model"))
		{
			ModelClass* model = new ModelClass;
			if (model->Initialize(m_D3D, model->LoadModelCalculatePath().c_str()))
			{
				if (m_materialList.find("gold") == m_materialList.end())
				{
					m_materialList.insert({ "gold", new MaterialPrefab{ "gold", m_D3D } });
				}
				model->SetMaterial(m_materialList.at("gold"));

				m_sceneModels.push_back(std::move(model));
				CreateAABB(model);
			}
		}
	}

	if (ImGui::CollapsingHeader("Post-process stack"))
	{
		if (ImGui::TreeNode("SSAO"))
		{
			ImGui::Checkbox("Use SSAO", &m_postprocessSSAO);
			ImGui::TreePop();
		}
		ImGui::Separator();

		if (ImGui::TreeNode("Vignette"))
		{
			ImGui::Checkbox("Use Vignette", &m_postprocessVignette);
			ImGui::TreePop();
		}
		ImGui::Separator();

		if (ImGui::TreeNode("LUT"))
		{
			//TODO Add LUT options
			ImGui::Checkbox("Use LUT", &m_postprocessLUT);
			ImGui::TreePop();
		}
		ImGui::Separator();

		if (ImGui::TreeNode("Bloom"))
		{
			ImGui::Checkbox("Use Bloom", &m_postprocessBloom);

			//BLOOM settings
			ImGui::SliderFloat("Weight 0", &m_bloomSettings.weights[0], 0.0f, 1.0f, "%.5f");
			ImGui::SliderFloat("Weight 1", &m_bloomSettings.weights[1], 0.0f, 1.0f, "%.5f");
			ImGui::SliderFloat("Weight 2", &m_bloomSettings.weights[2], 0.0f, 1.0f, "%.5f");
			ImGui::SliderFloat("Weight 3", &m_bloomSettings.weights[3], 0.0f, 1.0f, "%.5f");
			ImGui::SliderFloat("Weight 4", &m_bloomSettings.weights[4], 0.0f, 1.0f, "%.5f");
			ImGui::ColorPicker3("Intensity Bloom", m_bloomSettings.intensity);
			ImGui::TreePop();
		}
		ImGui::Separator();

		if (ImGui::TreeNode("Chromatic Aberration"))
		{
			ImGui::Checkbox("Use Chromatic Aberration", &m_postprocessChromaticAberration);
			//CHROMATIC ABERRATION settings
			ImGui::SliderFloat("R offset", &m_chromaticOffset.red, -0.01f, 0.01f, "%.5f");
			ImGui::SliderFloat("G offset", &m_chromaticOffset.green, -0.01f, 0.01f, "%.5f");
			ImGui::SliderFloat("B offset", &m_chromaticOffset.blue, -0.01f, 0.01f, "%.5f");
			ImGui::SliderFloat("Intensity CA", &m_chromaticIntensity, 0.001f, 2.0f, "%.3f");
			ImGui::TreePop();
		}
		ImGui::Separator();

		if (ImGui::TreeNode("Grain"))
		{
			ImGui::Checkbox("Use Grain", &m_postprocessGrain);

			//GRAIN settings
			ImGui::SliderFloat("Intensity Grain", &m_grainSettings.intensity, 0.0f, 1.0f, "%.2f");
			ImGui::SliderFloat("Size Grain", &m_grainSettings.size, 0.01f, 10.0f, "%.2f");

			if (ImGui::BeginCombo("Type Grain", CurrentGrainType))
			{
				constexpr int count = (int)GrainType::Count;
				for (int i = 0; i < count; ++i)
				{
					bool isSelected = (i == (int)m_grainSettings.type);
					if (ImGui::Selectable(GrainTypeArray[i], isSelected))
					{
						CurrentGrainType = GrainTypeArray[i];
						m_grainSettings.type = (GrainType)i;
					}
					if (isSelected)
					{
						ImGui::SetItemDefaultFocus();
					}
				}

				ImGui::EndCombo();
			}
			ImGui::Checkbox("Has color", &m_grainSettings.hasColor);
			ImGui::TreePop();
		}
	}

	if (ImGui::CollapsingHeader("Anti-aliasing"))
	{
		if (ImGui::BeginCombo("Anti-aliasing type", CurrentAntialiasingType))
		{
			constexpr int count = AntialiasingTypeArray.size();
			for (int i = 0; i < count; ++i)
			{
				const bool isSelected = (i == (int)m_antialiasingSettings.type);
				if (ImGui::Selectable(AntialiasingTypeArray[i], isSelected))
				{
					CurrentAntialiasingType = AntialiasingTypeArray[i];
					m_antialiasingSettings.type = (AntialiasingType)i;

					m_postprocessFXAA = false;
					m_postprocessSSAA = false;
					m_postprocessMSAA = false;

					if (m_antialiasingSettings.type == AntialiasingType::SSAA)
					{
						m_postprocessSSAA = true;
						m_D3D->CreateDepthBuffer(m_antialiasingSettings.sampleCount);

						if (!m_ssaaTexture->Initialize(m_D3D->GetDevice(), m_screenWidth * m_antialiasingSettings.sampleCount, m_screenHeight * m_antialiasingSettings.sampleCount, 1))
							return false;
					}
					else if (m_antialiasingSettings.type == AntialiasingType::MSAA)
					{
						m_postprocessMSAA = true;
						//m_D3D->MSAA_NUMBER_OF_SAMPLES = m_antialiasingSettings.sampleCount;
						//if (!(m_renderTextureMainScene->Initialize(m_D3D->GetDevice(), m_screenWidth, m_screenHeight, m_D3D->MSAA_NUMBER_OF_SAMPLES)))
						//	return false;
						//m_D3D->CreateDepthBuffer(1, m_D3D->MSAA_NUMBER_OF_SAMPLES);
					}
					else if (m_antialiasingSettings.type == AntialiasingType::FXAA)
					{
						m_postprocessFXAA = true;
					}
				}
				if (isSelected)
				{
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndCombo();
		}
		if (m_antialiasingSettings.type == AntialiasingType::SSAA)
		{
			if (ImGui::BeginCombo("Anti-aliasing quality MSAA", CurrentAntialiasingCountSSAA))
			{
				constexpr int count = AntialiasingCountArraySSAA.size();
				for (int i = 0; i < count; ++i)
				{
					const bool isSelected = (std::pow(2, i + 1) == (int)m_antialiasingSettings.sampleCount);
					if (ImGui::Selectable(AntialiasingCountArraySSAA[i]))
					{
						CurrentAntialiasingCountSSAA = AntialiasingCountArraySSAA[i];
						m_antialiasingSettings.sampleCount = std::pow(2, i + 1);

						m_postprocessFXAA = false;
						m_postprocessMSAA = false;
						m_postprocessSSAA = true;
						m_D3D->CreateDepthBuffer(m_antialiasingSettings.sampleCount);

						if (!m_ssaaTexture->Initialize(m_D3D->GetDevice(), m_screenWidth * m_antialiasingSettings.sampleCount, m_screenHeight * m_antialiasingSettings.sampleCount, 1))
							return false;
					}
					if (isSelected)
					{
						ImGui::SetItemDefaultFocus();
					}
				}
				ImGui::EndCombo();
			}
		}
		else if (m_antialiasingSettings.type == AntialiasingType::MSAA)
		{
			if (ImGui::BeginCombo("Anti-aliasing quality MSAA", CurrentAntialiasingCountMSAA))
			{
				constexpr int count = AntialiasingCountArrayMSAA.size();
				for (int i = 0; i < count; ++i)
				{
					const bool isSelected = (std::pow(2, i + 1) == (int)m_antialiasingSettings.sampleCount);
					if (ImGui::Selectable(AntialiasingCountArrayMSAA[i]))
					{
						CurrentAntialiasingCountMSAA = AntialiasingCountArrayMSAA[i];
						m_antialiasingSettings.sampleCount = std::pow(2, i + 1);

						m_postprocessFXAA = false;
						m_postprocessSSAA = false;
						m_postprocessMSAA = true;
						//m_D3D->MSAA_NUMBER_OF_SAMPLES = m_antialiasingSettings.sampleCount;
						//if (!(m_renderTextureMainScene->Initialize(m_D3D->GetDevice(), m_screenWidth, m_screenHeight, m_D3D->MSAA_NUMBER_OF_SAMPLES)))
						//	return false;
						//m_D3D->CreateDepthBuffer(1, m_D3D->MSAA_NUMBER_OF_SAMPLES);
					}
					if (isSelected)
					{
						ImGui::SetItemDefaultFocus();
					}
				}
				ImGui::EndCombo();
			}
		}
		else if (m_antialiasingSettings.type == AntialiasingType::FXAA)
		{
			m_postprocessFXAA = true;
		}
	}

	//SSAO radius/bias slider - DO NOT DELETE
		//ImGui::SliderFloat("SSAO Radius", &m_GBufferShaderSSAO->m_radiusSize, 0.0f, 5.0f, "%.2f");
		//ImGui::SliderFloat("SSAO Bias", &m_GBufferShaderSSAO->m_bias, 0.0f, 0.1f, "%.3f");

	m_internalTextureViewIndex = 0;
	
	//Finish ImGui and render
	if (!m_shaderEditorManager->IsMouseHoveredOnImGui())
	{
		m_shaderEditorManager->ResetMouseHoveredOnImGui();
		m_shaderEditorManager->UpdateMouseHoveredOnImGui(ImGui::IsWindowHovered() || ImGui::IsWindowFocused());
	}
	ImGui::End();

Finished_drawing:
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	return true;
}

void GraphicsClass::ReinitializeMainModel()
{
	static std::vector <LPCSTR> names{ "position", "texcoord", "normal", "tangent", "binormal" };
	static std::vector <DXGI_FORMAT> formats{ DXGI_FORMAT_R32G32B32_FLOAT, DXGI_FORMAT_R32G32_FLOAT, DXGI_FORMAT_R32G32B32_FLOAT, DXGI_FORMAT_R32G32B32_FLOAT, DXGI_FORMAT_R32G32B32_FLOAT };
	static const BaseShaderClass::vertexInputType input{ names, formats };

	m_pbrShader->m_materialNames = m_shaderEditorManager->GetUsedTextures();
	m_pbrShader->Initialize(m_D3D->GetDevice(), *m_D3D->GetHWND(), L"pbr_used.vs", L"pbr_used.ps", input);
}

void GraphicsClass::RefreshModelTick()
{
	if (m_shaderEditorManager->m_refreshModel)
	{
		m_shaderEditorManager->m_refreshModelTicks = 2;
		m_shaderEditorManager->m_refreshModel = false;
	}
	if (m_shaderEditorManager->m_refreshModelTicks == 0)
	{
		m_shaderEditorManager->m_refreshModelTicks--;
		m_shaderEditorManager->GenerateCodeToFile();
		ReinitializeMainModel();
	}
	else if (m_shaderEditorManager->m_refreshModelTicks > 0)
	{
		m_shaderEditorManager->m_refreshModelTicks--;
	}
}

bool GraphicsClass::RenderSkybox()
{
	XMMATRIX worldMatrix, viewMatrix, projectionMatrix;
	//DRAW SKYBOX
	XMFLOAT3 tmp = m_Camera->GetPosition();
	m_Camera->SetPosition(0.0f, 0.0f, 0.01f);
	m_Camera->Render();
	m_Camera->SetPosition(tmp);
	m_Camera->GetViewMatrix(viewMatrix);
	m_D3D->GetWorldMatrix(worldMatrix);
	m_D3D->GetProjectionMatrix(projectionMatrix);
	//DO NOT DELETE - Skybox rotation
	//worldMatrix = DirectX::XMMatrixMultiply(worldMatrix, DirectX::XMMatrixRotationY(m_Camera->GetRotation().y / 3.14f));
	//worldMatrix = DirectX::XMMatrixMultiply(worldMatrix, DirectX::XMMatrixRotationX(m_Camera->GetRotation().x / 3.14f));

	m_D3D->ChangeRasterizerCulling(D3D11_CULL_BACK);
	m_D3D->ChangeDepthStencilComparison(D3D11_COMPARISON_LESS_EQUAL);

	m_skyboxModel->Render(m_D3D->GetDeviceContext());
	m_skyboxShader->Render(m_D3D->GetDeviceContext(), m_skyboxModel->GetIndexCount(), worldMatrix, /*viewMatrix*/ XMMatrixIdentity(), projectionMatrix);

	m_D3D->ChangeRasterizerCulling(D3D11_CULL_BACK);
	m_D3D->ChangeDepthStencilComparison(D3D11_COMPARISON_LESS);

	return true;
}

bool GraphicsClass::DownsampleTexture()
{
	XMMATRIX worldMatrix, viewMatrix, orthoMatrix;

	m_renderTextureDownsampled->SetRenderTarget(m_D3D->GetDeviceContext(), m_D3D->GetDepthStencilView());
	m_renderTextureDownsampled->ClearRenderTarget(m_D3D->GetDeviceContext(), m_D3D->GetDepthStencilView(), 1.0f, 0.0f, 0.0f, 1.0f);

	m_Camera->Render();

	m_Camera->GetViewMatrix(viewMatrix);
	m_D3D->GetWorldMatrix(worldMatrix);
	m_renderTextureDownsampled->GetOrthoMatrix(orthoMatrix);

	m_D3D->TurnZBufferOff();
	for (ModelClass* const& model : m_sceneModels)
	{
		if (!m_renderTexturePreview->Render(m_D3D->GetDeviceContext(), model->GetIndexCount(), worldMatrix, viewMatrix, orthoMatrix))
			return false;
	}
	m_D3D->TurnZBufferOn();

	m_D3D->SetBackBufferRenderTarget();
	m_D3D->ResetViewport();

	m_renderTexturePreview->BindTexture(m_renderTextureDownsampled->GetShaderResourceView());
	return true;
}

bool GraphicsClass::DownsampleTexture(RenderTextureClass * dst, RenderTextureClass * const src, const float size, const int depthStencilViewSize)
{
	XMMATRIX worldMatrix, viewMatrix, projectionMatrix;

	m_Camera->Render();
	m_D3D->GetWorldMatrix(worldMatrix);
	m_D3D->GetProjectionMatrix(projectionMatrix);
	m_Camera->GetViewMatrix(viewMatrix);

	if (!(m_renderTexturePreview->Initialize(m_D3D, ModelClass::ShapeSize::RECTANGLE, -1.0f, size, 1.0f, -size)))
		return false;

	dst->SetRenderTarget(m_D3D->GetDeviceContext(), m_D3D->GetDepthStencilView(1));
	dst->ClearRenderTarget(m_D3D->GetDeviceContext(), m_D3D->GetDepthStencilView(1), 0.0f, 0.0f, 0.0f, 1.0f);

	m_convoluteQuadModel->Render(m_D3D->GetDeviceContext());
	m_renderTexturePreview->BindTexture(src->GetShaderResourceView());
	if (!m_renderTexturePreview->Render(m_D3D->GetDeviceContext(), m_convoluteQuadModel->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix))
		return false;

	return true;
}

bool GraphicsClass::UpscaleTexture()
{
	XMMATRIX worldMatrix, viewMatrix, orthoMatrix;

	m_renderTextureUpscaled->SetRenderTarget(m_D3D->GetDeviceContext(), m_D3D->GetDepthStencilView());
	m_renderTextureUpscaled->ClearRenderTarget(m_D3D->GetDeviceContext(), m_D3D->GetDepthStencilView(), 0.0f, 0.0f, 0.0f, 1.0f);

	m_Camera->Render();

	m_Camera->GetViewMatrix(viewMatrix);
	m_D3D->GetWorldMatrix(worldMatrix);
	m_renderTextureUpscaled->GetOrthoMatrix(orthoMatrix);

	m_D3D->TurnZBufferOff();
	for (ModelClass* const& model : m_sceneModels)
	{
		if (!m_renderTexturePreview->Render(m_D3D->GetDeviceContext(), model->GetIndexCount(), worldMatrix, viewMatrix, orthoMatrix))
			return false;
	}
	m_D3D->TurnZBufferOn();

	m_D3D->SetBackBufferRenderTarget();
	m_D3D->ResetViewport();

	m_renderTexturePreview->BindTexture(m_renderTextureUpscaled->GetShaderResourceView());
	return true;
}

bool GraphicsClass::BlurFilterScreenSpaceTexture(bool vertical, const RenderTextureClass * textureToBlur, RenderTextureClass * textureToReturn, int screenWidth)
{
	ID3D11ShaderResourceView* resourceView = textureToBlur->GetShaderResourceViewCopy();
	return BlurFilterScreenSpaceTexture(vertical, resourceView, textureToReturn, screenWidth);
}

bool GraphicsClass::BlurFilterScreenSpaceTexture(bool vertical, const ID3D11ShaderResourceView * textureToBlur, RenderTextureClass * textureToReturn, int screenWidth)
{
	XMMATRIX worldMatrix, viewMatrix, orthoMatrix;

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
		//for (const auto& model : m_sceneModels)
		{
			m_convoluteQuadModel->Render(m_D3D->GetDeviceContext());
			if (!m_blurShaderHorizontal->Render(m_D3D->GetDeviceContext(), m_convoluteQuadModel->GetIndexCount(), worldMatrix, viewMatrix, orthoMatrix))
				return false;
		}

		m_D3D->TurnZBufferOn();

		m_D3D->SetBackBufferRenderTarget();
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
		//for (const auto& model : m_sceneModels)
		{
			m_convoluteQuadModel->Render(m_D3D->GetDeviceContext());
			if (!m_blurShaderVertical->Render(m_D3D->GetDeviceContext(), m_convoluteQuadModel->GetIndexCount(), worldMatrix, viewMatrix, orthoMatrix))
				return false;
		}
		m_D3D->TurnZBufferOn();
		m_D3D->SetBackBufferRenderTarget();
	}

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
		//for (const auto& model : m_sceneModels)
		for (const auto& model : m_sceneModels)
		{
			if (!m_blurShaderHorizontal->Render(m_D3D->GetDeviceContext(), model->GetIndexCount(), worldMatrix, viewMatrix, orthoMatrix))
				return false;
		}

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
		for (const auto& model : m_sceneModels)
		{
			if (!m_blurShaderVertical->Render(m_D3D->GetDeviceContext(), model->GetIndexCount(), worldMatrix, viewMatrix, orthoMatrix))
				return false;
		}

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
		for (ModelClass* const& model : m_sceneModels)
		{
			if (!m_blurShaderHorizontal->Render(m_D3D->GetDeviceContext(), model->GetIndexCount(), worldMatrix, viewMatrix, orthoMatrix))
				return false;
		}

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
		for (ModelClass* const& model : m_sceneModels)
		{
			if (!m_blurShaderVertical->Render(m_D3D->GetDeviceContext(), model->GetIndexCount(), worldMatrix, viewMatrix, orthoMatrix))
				return false;
		}

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
	m_skyboxBlurHorizontal = new RenderTextureClass;
	if (!(m_skyboxBlurHorizontal->Initialize(m_D3D->GetDevice(), CONVOLUTION_DIFFUSE_SIZE, CONVOLUTION_DIFFUSE_SIZE, 1, RenderTextureClass::Scaling::NONE)))
		return false;

	//Texture vertical blur
	m_skyboxBlurVertical = new RenderTextureClass;
	if (!(m_skyboxBlurVertical->Initialize(m_D3D->GetDevice(), CONVOLUTION_DIFFUSE_SIZE, CONVOLUTION_DIFFUSE_SIZE, 1, RenderTextureClass::Scaling::NONE)))
		return false;

	DownsampleSkyboxFace(L"Skyboxes/posx.jpg", L"Skyboxes/conv_posx.dds", false);
	//DownsampleSkyboxFace(L"Skyboxes/posy.jpg", L"Skyboxes/conv_posy.dds", false);
	//DownsampleSkyboxFace(L"Skyboxes/posz.jpg", L"Skyboxes/conv_posz.dds", false);
	//DownsampleSkyboxFace(L"Skyboxes/negx.jpg", L"Skyboxes/conv_negx.dds", false);
	//DownsampleSkyboxFace(L"Skyboxes/negy.jpg", L"Skyboxes/conv_negy.dds", false);
	//DownsampleSkyboxFace(L"Skyboxes/negz.jpg", L"Skyboxes/conv_negz.dds", false);

	if (std::ifstream skyboxFile{ "Skyboxes/conv_cubemap.dds" })
	{}
	else
	{	//Couldn't open file - create
		system("texassemble cube -w 256 -h 256 -f R8G8B8A8_UNORM -o Skyboxes/conv_cubemap.dds Skyboxes/conv_posx.dds Skyboxes/conv_negx.dds Skyboxes/conv_posy.dds Skyboxes/conv_negy.dds Skyboxes/conv_posz.dds Skyboxes/conv_negz.dds");
		if (std::ifstream skyboxFile{ "Skyboxes/conv_cubemap.dds" }) {}
		else
		{
			return false; //File wasn't created - error!
		}
	}

	return true;
}

bool GraphicsClass::DownsampleSkyboxFace(const wchar_t * inputFilename, const wchar_t * outputFilename, bool isDDS)
{
	XMMATRIX worldMatrix, viewMatrix, orthoMatrix;

	m_renderTexture->LoadTexture(m_D3D->GetDevice(), inputFilename, m_renderTexture->GetShaderResource(), m_renderTexture->GetShaderResourceView(), isDDS);
	m_renderTexturePreview->BindTexture(m_renderTexture->GetShaderResourceView());

	m_skyboxDownsampled->SetRenderTarget(m_D3D->GetDeviceContext(), m_D3D->GetDepthStencilView());
	m_skyboxDownsampled->ClearRenderTarget(m_D3D->GetDeviceContext(), m_D3D->GetDepthStencilView(), 1.0f, 0.0f, 0.0f, 1.0f);

	m_Camera->Render();

	m_Camera->GetViewMatrix(viewMatrix);
	m_D3D->GetWorldMatrix(worldMatrix);
	m_skyboxDownsampled->GetOrthoMatrix(orthoMatrix);

	m_D3D->TurnZBufferOff();
	for (ModelClass* const& model : m_sceneModels)
	{
		if (!m_renderTexturePreview->Render(m_D3D->GetDeviceContext(), model->GetIndexCount(), worldMatrix, viewMatrix, orthoMatrix))
			return false;
	}
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

//TODO Blur horizontal/vertical/blurfilter repeats themselves - investigave
bool GraphicsClass::BlurFilter(bool vertical, ID3D11ShaderResourceView * srcTex, RenderTextureClass * dstTex, int width)
{
	XMMATRIX worldMatrix, viewMatrix, orthoMatrix;

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
		for (ModelClass* const& model : m_sceneModels)
		{
			if (!m_blurShaderHorizontal->Render(m_D3D->GetDeviceContext(), model->GetIndexCount(), worldMatrix, viewMatrix, orthoMatrix))
				return false;
		}

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
		for (ModelClass* const& model : m_sceneModels)
		{
			if (!m_blurShaderVertical->Render(m_D3D->GetDeviceContext(), model->GetIndexCount(), worldMatrix, viewMatrix, orthoMatrix))
				return false;
		}

		m_D3D->TurnZBufferOn();

		m_D3D->SetBackBufferRenderTarget();
		m_D3D->ResetViewport();

		m_renderTexturePreview->BindTexture(dstTex->GetShaderResourceView());
	}

	return true;
}

bool GraphicsClass::ConvoluteShader(ID3D11ShaderResourceView * srcTex, RenderTextureClass * dstTex)
{
	if (std::ifstream skyboxFile{ "Skyboxes/conv_cubemap.dds" })
	{
		return true;
	}
	else
	{
		XMMATRIX worldMatrix, viewMatrix, projectionMatrix;

		//XMFLOAT3 position = XMFLOAT3(0, 0, 0);
		//XMVECTOR tar[] = { XMVectorSet(1, 0, 0, 0), XMVectorSet(-1, 0, 0, 0), XMVectorSet(0, 1, 0, 0), XMVectorSet(0, -1, 0, 0), XMVectorSet(0, 0, 1, 0), XMVectorSet(0, 0, -1, 0) };
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
}

bool GraphicsClass::CreateShadowMap(RenderTextureClass*& targetTex)
{
	targetTex->SetRenderTarget(m_D3D->GetDeviceContext(), m_D3D->GetDepthStencilView());
	targetTex->ClearRenderTarget(m_D3D->GetDeviceContext(), m_D3D->GetDepthStencilView(), 1.0f, 1.0f, 1.0f, 1.0f);

	if (!RenderDepthScene())
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

	//Set light position
	m_directionalLight->SetLookAt(0, 0, 0);
	m_shadowMapShader->SetLightPosition(m_directionalLight->GetPosition());
	for (ModelClass* const& model : m_sceneModels)
	{
		model->Render(m_D3D->GetDeviceContext());
	}

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

	//for (int i = 0; i < 5; i++)
	//{
	//	m_D3D->GetWorldMatrix(worldMatrix);
	//	m_directionalLight->GenerateViewMatrix();
	//	m_directionalLight->GetViewMatrix(lightViewMatrix);
	//	m_directionalLight->GetProjectionMatrix(lightProjectionMatrix);

	//	worldMatrix = XMMatrixTranslation(i * 2.0f, 2.0f, 1.0f);
	//	m_Model->Render(m_D3D->GetDeviceContext());
	//	result = m_shadowMapShader->Render(m_D3D->GetDeviceContext(), m_Model->GetIndexCount(), worldMatrix, lightViewMatrix, lightProjectionMatrix);
	//	if (!result)
	//		return false;
	//}

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
	renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMS;
	renderTargetViewDesc.Texture2D.MipSlice = 0;

	ZeroMemory(&shaderResourceViewDesc, sizeof(shaderResourceViewDesc));
	shaderResourceViewDesc.Format = textureDesc.Format;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DMS;
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

//TODO Reduntant? "PrepareEnvironmentPrefilteredMap"
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
		renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMSARRAY;
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
	for (ModelClass* const& model : m_sceneModels)
	{
		model->Render(m_D3D->GetDeviceContext());

		worldMatrix = DirectX::XMMatrixMultiply(worldMatrix, XMMatrixTranslation(model->GetPosition().x, model->GetPosition().y, model->GetPosition().z));
		worldMatrix = DirectX::XMMatrixMultiply(worldMatrix, DirectX::XMMatrixRotationX(model->GetRotation().x * 0.0174532925f));
		worldMatrix = DirectX::XMMatrixMultiply(worldMatrix, DirectX::XMMatrixRotationY(model->GetRotation().y * 0.0174532925f));
		worldMatrix = DirectX::XMMatrixMultiply(worldMatrix, DirectX::XMMatrixRotationZ(model->GetRotation().z * 0.0174532925f));
		worldMatrix = DirectX::XMMatrixMultiply(worldMatrix, DirectX::XMMatrixScaling(model->GetScale().x, model->GetScale().y, model->GetScale().z));
		result = shaderToExecute->Render(m_D3D->GetDeviceContext(), model->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix);
		if (!result)
			return false;
	}
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

	//Create matrices based on light position
	m_Camera->Render();
	m_D3D->GetWorldMatrix(worldMatrix);
	m_Camera->GetViewMatrix(viewMatrix);
	m_D3D->GetProjectionMatrix(projectionMatrix);

	//Render test buddha
	m_convoluteQuadModel->Render(m_D3D->GetDeviceContext());

	if(!shaderToExecute->Render(m_D3D->GetDeviceContext(), m_convoluteQuadModel->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix))
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

	//Create matrices based on light position
	m_Camera->Render();
	m_D3D->GetWorldMatrix(worldMatrix);
	m_Camera->GetViewMatrix(viewMatrix);
	m_D3D->GetProjectionMatrix(projectionMatrix);

	//Render test buddha
	m_convoluteQuadModel->Render(m_D3D->GetDeviceContext());

	if (!shaderToExecute->Render(m_D3D->GetDeviceContext(), m_convoluteQuadModel->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix))
		return false;
	////////////////////////////////////
	//m_renderTexturePreview->BindTexture(targetTex->GetShaderResourceView());

	m_D3D->SetBackBufferRenderTarget();
	m_D3D->ResetViewport();

	return true;
}

void GraphicsClass::RenderTextureViewImGui(ID3D11Resource *& resource, ID3D11ShaderResourceView *& resourceView, const char* label)
{
	//Propably unused but DO NOT DELETE
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

inline void GraphicsClass::RenderTextureViewImGuiEditor(ID3D11Resource *& resource, ID3D11ShaderResourceView *& resourceView, const char * label, std::string& path, bool skipLabel)
{
	if (!skipLabel)
	{
		ImGui::Text(label);
	}
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

GraphicsClass::EMaterialInputResult GraphicsClass::RenderInputForMaterial(UIShaderEditorBlock * block, bool changeName, bool isActive)
{
	if (UIShaderEditorOutput* out = block->GetFirstOutputNode())
	{
		if (changeName && out->m_isVariable)
		{
			if (ImGui::Button("Demote variable"))
			{
				out->DemoteVariable();
				block->UpdateVariable();
			}
			if (ImGui::InputText("Variable name", const_cast<char*>(out->m_visibleName.data()), 30))
			{
				out->SaveVisibleName();
				block->ChangeBlockName();
			}
		}
		else if (!out->m_isVariable)
		{
			if (ImGui::Button("Promote to variable"))
			{
				out->PromoteToVariable();
				block->UpdateVariable();
			}
		}

		stringstream ss;
		ss << block->GetBlockID();
		std::string hash = ss.str();

		if (block->GetFunctionName() == "texture")
		{
			RenderTextureViewImGuiEditor(out->m_connectedTexture, out->m_connectedTextureView, out->m_visibleName.c_str(), out->m_texturePath, changeName);
		}
		else if (out->m_returnType == "float")
		{
			if (!changeName)
				ImGui::Text(out->m_visibleName.c_str());
			ImGui::InputFloat(hash.c_str(), &out->m_value);
		}
		else if (out->m_returnType == "float2")
		{
			if (!changeName)
				ImGui::Text(out->m_visibleName.c_str());
			ImGui::InputFloat2(hash.c_str(), &out->m_valueTwo[0]);
		}
		else if (out->m_returnType == "float3")
		{
			if (!changeName)
				ImGui::Text(out->m_visibleName.c_str());

			ImGui::ColorEdit3(hash.c_str(), out->m_valueThree, ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoSmallPreview);
			ImGui::SameLine();
			if (ImGui::ColorButton(hash.c_str(), { out->m_valueThree[0], out->m_valueThree[1], out->m_valueThree[2], 1.0f }) || isActive)
			{
				ImGui::ColorPicker3(hash.c_str(), out->m_valueThree, ImGuiColorEditFlags_NoInputs);
				if (!ImGui::IsItemHovered() && m_mouse->GetMouse()->GetLMBPressed() && !m_shaderEditorManager->m_wasLeftButtonUp)
				{
					m_shaderEditorManager->m_wasLeftButtonUp = true;
					m_shaderEditorManager->SetPickingColorElement(nullptr);
					return EMaterialInputResult::Break;
				}
				else if (!m_mouse->GetMouse()->GetLMBPressed())
				{
					m_shaderEditorManager->m_wasLeftButtonUp = ImGui::IsItemHovered();
				}
				return EMaterialInputResult::StopOthers;
			}
		}
		else if (out->m_returnType == "float4")
		{
			if (!changeName)
				ImGui::Text(out->m_visibleName.c_str());

			ImGui::ColorEdit4(hash.c_str(), out->m_valueFour, ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoSmallPreview);
			ImGui::SameLine();
			if (ImGui::ColorButton(hash.c_str(), { out->m_valueFour[0], out->m_valueFour[1], out->m_valueFour[2], out->m_valueFour[3] }) || isActive)
			{
				ImGui::ColorPicker4(hash.c_str(), out->m_valueFour, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf);
				if (!ImGui::IsItemHovered() && m_mouse->GetMouse()->GetLMBPressed() && !m_shaderEditorManager->m_wasLeftButtonUp)
				{
					m_shaderEditorManager->m_wasLeftButtonUp = true;
					m_shaderEditorManager->SetPickingColorElement(nullptr);
					return EMaterialInputResult::Break;
				}
				else if (!m_mouse->GetMouse()->GetLMBPressed())
				{
					m_shaderEditorManager->m_wasLeftButtonUp = ImGui::IsItemHovered();
				}
				return EMaterialInputResult::StopOthers;
			}
		}
	}

	return EMaterialInputResult::Continue;
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

bool GraphicsClass::RenderPostprocess(ID3D11ShaderResourceView * mainFrameBuffer)
{
	XMMATRIX worldMatrix, viewMatrix, projectionMatrix;
	m_Camera->Render();
	m_Camera->GetViewMatrix(viewMatrix);
	m_D3D->GetWorldMatrix(worldMatrix);
	m_D3D->GetProjectionMatrix(projectionMatrix);

	if (mainFrameBuffer != nullptr)
		m_postProcessShader->SetScreenBuffer(mainFrameBuffer);

	m_convoluteQuadModel->Initialize(m_D3D->GetDevice(), ModelClass::ShapeSize::RECTANGLE, -1.0f, 1.0f, 1.0f, -1.0f, true);
	m_convoluteQuadModel->Render(m_D3D->GetDeviceContext());
	worldMatrix = worldMatrix * 0;
	return m_postProcessShader->Render(m_D3D->GetDeviceContext(), m_convoluteQuadModel->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix);
}

bool GraphicsClass::RenderFXAATexture(RenderTextureClass * targetTex)
{
	targetTex->SetRenderTarget(m_D3D->GetDeviceContext(), m_D3D->GetDepthStencilView());
	targetTex->ClearRenderTarget(m_D3D->GetDeviceContext(), m_D3D->GetDepthStencilView(), 0.0f, 0.0f, 0.0f, 1.0f);

	/////// RENDER SCENE TO BUFFER ///////
	XMMATRIX worldMatrix, viewMatrix, projectionMatrix;

	//Create matrices based on light position
	m_Camera->Render();
	m_D3D->GetWorldMatrix(worldMatrix);
	m_Camera->GetViewMatrix(viewMatrix);
	m_D3D->GetProjectionMatrix(projectionMatrix);

	//Render test buddha
	m_convoluteQuadModel->Render(m_D3D->GetDeviceContext());

	if (!m_fxaaShader->Render(m_D3D->GetDeviceContext(), m_convoluteQuadModel->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix))
		return false;
	////////////////////////////////////
	//m_renderTexturePreview->BindTexture(targetTex->GetShaderResourceView());

	m_D3D->SetBackBufferRenderTarget();
	m_D3D->ResetViewport();

	return true;
}

bool GraphicsClass::CreateShaderEditor()
{
	m_shaderEditorManager = new ShaderEditorManager(m_D3D, m_mouse->GetMouse());
	m_shaderEditorManager->SetRefToClickedOutside(&m_focusOnChoosingWindowsShader);
	return true;
}

float GraphicsClass::lerp(float a, float b, float val)
{
	return a * (1.0f - val) + b * val;
}

inline __int64 GraphicsClass::GetTimeMillis()
{
	std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
	auto duration = now.time_since_epoch();
	auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();

	return millis;
}

void GraphicsClass::SaveScene(const std::string name)
{
	if (ofstream output{ "Scenes/" + name })
	{
		for (ModelClass* const& model : m_sceneModels)
		{
			output << model->GetSaveData() << "\n";
		}
	}
}

void GraphicsClass::ToggleGUI()
{
	ENABLE_GUI = !ENABLE_GUI;
}

static std::pair<float, float> savedMousePos{};
void GraphicsClass::TryPickObjects()
{
	if (m_shaderEditorManager->IsMouseHoveredOnImGui())
	{
		return;
	}
	if (m_modelPickerBools.xAxis || m_modelPickerBools.yAxis || m_modelPickerBools.zAxis)
	{
		return;
	}

	for (const auto& model : m_sceneModels)
	{
		const auto result = RaycastToModel(model);
		const XMFLOAT3 lb = model->GetBounds().GetMinBounds(model);
		const XMFLOAT3 rt = model->GetBounds().GetMaxBounds(model);

		if (TestAABBIntersection(lb, rt, result.origin, result.dirfrac))
		{
			if (m_selectedModel)
			{
				for (const auto& model : m_sceneModels)
					model->m_selected = false;
			}
			m_selectedModel = model;
			model->m_selected = true;
			return;
		}
	}

	m_selectedModel = nullptr;
}
void GraphicsClass::TryRayPick()
{
	//for (const auto& model : m_sceneModels)
	{
		if (m_modelPickerBools.xAxis || m_modelPickerBools.yAxis || m_modelPickerBools.zAxis)
		{
			return;
		}
		if (!m_selectedModel)
		{
			return;
		}

		const auto result = RaycastToModel(m_selectedModel);
		if (TryPickModelPickerArrow(m_selectedModel, ModelPicker::Axis::X, { result.origin.x, result.origin.y, result.origin.z }, result.dirfrac))
		{
			m_modelPickerBools.xAxis = true;
			m_mouse->GetMouse()->SetVisibility(FALSE);
			savedMousePos = GetCurrentMousePosition();
			return;
		}
		else if (TryPickModelPickerArrow(m_selectedModel, ModelPicker::Axis::Y, { result.origin.x, result.origin.y, result.origin.z }, result.dirfrac))
		{
			m_modelPickerBools.yAxis = true;
			m_mouse->GetMouse()->SetVisibility(FALSE);
			savedMousePos = GetCurrentMousePosition();
			return;
		}
		else if (TryPickModelPickerArrow(m_selectedModel, ModelPicker::Axis::Z, { result.origin.x, result.origin.y, result.origin.z }, result.dirfrac))
		{
			m_modelPickerBools.zAxis = true;
			m_mouse->GetMouse()->SetVisibility(FALSE);
			savedMousePos = GetCurrentMousePosition();
			return;
		}
	}
}

void GraphicsClass::UpdateRayPick()
{
	if (!m_modelPickerBools.xAxis && !m_modelPickerBools.yAxis && !m_modelPickerBools.zAxis)
	{
		return;
	}

	const XMFLOAT2 mouse = { GetCurrentMousePosition().first - savedMousePos.first, GetCurrentMousePosition().second - savedMousePos.second };
	const XMFLOAT2 mouseNormalized = { XMVector2Normalize({ mouse.x, mouse.y }).m128_f32[0], XMVector2Normalize({ mouse.x, mouse.y }).m128_f32[1] };
	const float mouseLength = std::sqrt(mouse.x * mouse.x + mouse.y * mouse.y);

	if (std::isnan(mouse.x) || std::isnan(mouse.y))
	{
		return;
	}
	if (std::isinf(mouse.x) || std::isinf(mouse.y))
	{
		return;
	}

	XMMATRIX worldMatrix;
	XMMATRIX viewMatrix;
	XMMATRIX projectionMatrix;

	//for (const auto& model : m_sceneModels)
	if (const auto model = m_selectedModel)
	{
		CreateModelPickerMVP(model, worldMatrix, viewMatrix, projectionMatrix);
		const XMFLOAT3 boundsScreenSize = model->GetBounds().BoundingBoxSize(worldMatrix, viewMatrix, projectionMatrix);

		if (m_modelPickerBools.xAxis)
		{
			const float dot = CalculateMouseArrowDotProduct(model, ModelPicker::Axis::X, mouseNormalized);
			const float sizeX = model->GetBounds().GetSizeX() * (mouseLength * 0.5f / boundsScreenSize.x);
			model->SetPosition(model->GetPosition().x + std::abs(sizeX * dot) * (mouse.x > 0.0f ? 1.0f : -1.0f), model->GetPosition().y, model->GetPosition().z);
		}
		else if (m_modelPickerBools.yAxis)
		{
			const float dot = CalculateMouseArrowDotProduct(model, ModelPicker::Axis::Y, mouseNormalized);
			const float sizeY = model->GetBounds().GetSizeY() * (mouseLength * 0.5f / boundsScreenSize.y);
			model->SetPosition(model->GetPosition().x, model->GetPosition().y + std::abs(sizeY * dot) * (mouse.y > 0.0f ? 1.0f : -1.0f), model->GetPosition().z);
		}
		else if (m_modelPickerBools.zAxis)
		{
			const float dot = CalculateMouseArrowDotProduct(model, ModelPicker::Axis::Z, mouseNormalized);
			const float sizeZ = model->GetBounds().GetSizeZ() * (mouseLength * 0.1f / boundsScreenSize.z);
			model->SetPosition(model->GetPosition().x, model->GetPosition().y, model->GetPosition().z + sizeZ * dot);
		}
	}
	savedMousePos = GetCurrentMousePosition();
}

void GraphicsClass::ResetRayPick()
{
	if (m_modelPickerBools.xAxis || m_modelPickerBools.yAxis || m_modelPickerBools.zAxis)
	{
		if (m_selectedModel)
		{
			m_modelPickerBools.Reset();
			m_mouse->GetMouse()->SetCursorPosition(m_modelPicker->WorldToScreenspace(this, m_selectedModel->GetBounds().GetCenter(), m_selectedModel), false);
			m_mouse->GetMouse()->SetVisibility(TRUE);
		}
	}
}

float GraphicsClass::CalculateMouseArrowDotProduct(ModelClass * const model, const ModelPicker::Axis axis, const XMFLOAT2 mouseNormalized)
{
	XMFLOAT2 min = m_modelPicker->GetMinScreenspace(this, model, axis);
	XMFLOAT2 max = m_modelPicker->GetMaxScreenspace(this, model, axis);

	XMFLOAT2 dir = { max.x - min.x, max.y - min.y };
	dir = { XMVector2Normalize({ dir.x, dir.y }).m128_f32[0], XMVector2Normalize({ dir.x, dir.y }).m128_f32[1] };

	std::vector<float> arrowDir{ dir.x, dir.y };
	std::vector<float> mouseDir{ mouseNormalized.x, mouseNormalized.y };

	return std::inner_product(std::begin(arrowDir), std::end(arrowDir), std::begin(mouseDir), 0.0f);
}

void GraphicsClass::CreateModelPickerMVP(ModelClass* const model, XMMATRIX & worldMatrix, XMMATRIX & viewMatrix, XMMATRIX & projectionMatrix)
{
	m_Camera->GetViewMatrix(viewMatrix);
	m_D3D->GetProjectionMatrix(projectionMatrix);
	m_D3D->GetWorldMatrix(worldMatrix);
	worldMatrix = DirectX::XMMatrixMultiply(worldMatrix, DirectX::XMMatrixScaling(model->GetScale().x, model->GetScale().y, model->GetScale().z));
	worldMatrix = DirectX::XMMatrixMultiply(worldMatrix, DirectX::XMMatrixRotationX(model->GetRotation().x * 0.0174532925f));
	worldMatrix = DirectX::XMMatrixMultiply(worldMatrix, DirectX::XMMatrixRotationY(model->GetRotation().y * 0.0174532925f));
	worldMatrix = DirectX::XMMatrixMultiply(worldMatrix, DirectX::XMMatrixRotationZ(model->GetRotation().z * 0.0174532925f));
	worldMatrix = DirectX::XMMatrixMultiply(worldMatrix, XMMatrixTranslation(model->GetPosition().x, model->GetPosition().y, model->GetPosition().z));
}

GraphicsClass::MouseRaycastResult GraphicsClass::RaycastToModel(ModelClass* const model)
{
	//Go to [-1, 1] coordinates
	float x = GetCurrentMousePosition().first;
	float y = GetCurrentMousePosition().second;
	if (x > 1.0f)
		x = 1.0f;
	else if (x < -1.0f)
		x = -1.0f;

	if (y > 1.0f)
		y = 1.0f;
	else if (y < -1.0f)
		y = -1.0f;

	float pointX, pointY;
	XMMATRIX projectionMatrix, viewMatrix, inverseViewMatrix, worldMatrix, translateMatrix, inverseWorldMatrix;
	XMVECTOR origin, rayOrigin, direction, rayDirection;

	pointX = x;
	pointY = y;

	m_D3D->GetProjectionMatrix(projectionMatrix);
	pointX = pointX / projectionMatrix.r[0].m128_f32[0];
	pointY = pointY / projectionMatrix.r[1].m128_f32[1];

	m_Camera->GetViewMatrix(viewMatrix);
	inverseViewMatrix = XMMatrixInverse(nullptr, viewMatrix);

	direction = {
		(pointX * inverseViewMatrix.r[0].m128_f32[0]) + (pointY * inverseViewMatrix.r[1].m128_f32[0]) + inverseViewMatrix.r[2].m128_f32[0],
		(pointX * inverseViewMatrix.r[0].m128_f32[1]) + (pointY * inverseViewMatrix.r[1].m128_f32[1]) + inverseViewMatrix.r[2].m128_f32[1],
		(pointX * inverseViewMatrix.r[0].m128_f32[2]) + (pointY * inverseViewMatrix.r[1].m128_f32[2]) + inverseViewMatrix.r[2].m128_f32[2]
	};

	origin = { m_Camera->GetPosition().x, m_Camera->GetPosition().y, m_Camera->GetPosition().z };

	m_D3D->GetWorldMatrix(worldMatrix);
	translateMatrix = XMMatrixTranslation(model->GetPosition().x, model->GetPosition().y, model->GetPosition().z);
	worldMatrix = XMMatrixMultiply(worldMatrix, translateMatrix);

	inverseWorldMatrix = XMMatrixInverse(nullptr, worldMatrix);

	rayOrigin = XMVector3TransformCoord(origin, inverseWorldMatrix);
	rayDirection = XMVector3TransformNormal(direction, inverseWorldMatrix);

	rayDirection = XMVector3Normalize(rayDirection);

	XMFLOAT3 dirfrac{ 1.0f / rayDirection.m128_f32[0], 1.0f / rayDirection.m128_f32[1], 1.0f / rayDirection.m128_f32[2] };

	return MouseRaycastResult{ {origin.m128_f32[0], origin.m128_f32[1], origin.m128_f32[2] }, dirfrac };
}

void GraphicsClass::LoadScene(const std::string name)
{
	std::string line;
	std::string err;
	if (ifstream input{ "Scenes/" + name, std::ios_base::binary })
	{
		while (getline(input, line))
		{
			const auto json = json11::Json::parse(line, err);
			const std::string modelName = json["modelName"].string_value();
			if (modelName != "")
			{
				ModelClass* model = new ModelClass;
				if (model->Initialize(m_D3D, (modelName + ".obj").c_str()))
				{
					const std::string sceneName = json["sceneName"].string_value();
					const json11::Json::array position = json["position"].array_items();
					const json11::Json::array scale = json["scale"].array_items();
					const json11::Json::array rotation = json["rotation"].array_items();
					const std::string materialName = json["material"].string_value();
					model->m_name = sceneName;
					model->SaveVisibleName();
					if (position.size() == 3 && scale.size() == 3 && rotation.size() == 3)
					{
						model->SetPosition(position.at(0).number_value(), position.at(1).number_value(), position.at(2).number_value());
						model->SetScale(scale.at(0).number_value(), scale.at(1).number_value(), scale.at(2).number_value());
						model->SetRotation(rotation.at(0).number_value(), rotation.at(1).number_value(), rotation.at(2).number_value());
					}
					m_sceneModels.push_back(std::move(model));
					CreateAABB(model);
					continue;
				}
				else
				{
					delete model;
				}
			}
		}
	}
}

void GraphicsClass::CreateAABB(ModelClass * baseModel)
{
	baseModel->CreateWireframe();
	//CreateAABBBox(baseModel->GetBounds());
}

void GraphicsClass::CreateAABBBox(const Bounds bounds)
{
	//Front
	{
		ModelClass* model = new ModelClass;
		model->Initialize(m_D3D->GetDevice(), { bounds.minX, bounds.minY, bounds.minZ }, { bounds.minX, bounds.maxY, bounds.minZ }, { bounds.maxX, bounds.minY, bounds.minZ },
		{ bounds.maxX, bounds.maxY, bounds.minZ });
		m_sceneModels.push_back(std::move(model));
	}
	//Back
	{
		ModelClass* model = new ModelClass;
		model->Initialize(m_D3D->GetDevice(), { bounds.minX, bounds.minY, bounds.maxZ }, { bounds.minX, bounds.maxY, bounds.maxZ }, { bounds.maxX, bounds.minY, bounds.maxZ },
		{ bounds.maxX, bounds.maxY, bounds.maxZ });
		m_sceneModels.push_back(std::move(model));
	}
	//Right
	{
		ModelClass* model = new ModelClass;
		model->Initialize(m_D3D->GetDevice(), { bounds.maxX, bounds.minY, bounds.minZ }, { bounds.maxX, bounds.maxY, bounds.minZ }, { bounds.maxX, bounds.minY, bounds.maxZ },
		{ bounds.maxX, bounds.maxY, bounds.maxZ });
		m_sceneModels.push_back(std::move(model));
	}
	//Left
	{
		ModelClass* model = new ModelClass;
		model->Initialize(m_D3D->GetDevice(), { bounds.minX, bounds.minY, bounds.minZ }, { bounds.minX, bounds.maxY, bounds.minZ }, { bounds.minX, bounds.minY, bounds.maxZ },
		{ bounds.minX, bounds.maxY, bounds.maxZ });
		m_sceneModels.push_back(std::move(model));
	}
	//Top
	{
		ModelClass* model = new ModelClass;
		model->Initialize(m_D3D->GetDevice(), { bounds.minX, bounds.maxY, bounds.minZ }, { bounds.minX, bounds.maxY, bounds.maxZ }, { bounds.maxX, bounds.maxY, bounds.minZ },
		{ bounds.maxX, bounds.maxY, bounds.maxZ });
		m_sceneModels.push_back(std::move(model));
	}
	//Bottom
	{
		ModelClass* model = new ModelClass;
		model->Initialize(m_D3D->GetDevice(), { bounds.minX, bounds.minY, bounds.minZ }, { bounds.minX, bounds.minY, bounds.maxZ }, { bounds.maxX, bounds.minY, bounds.minZ },
		{ bounds.maxX, bounds.minY, bounds.maxZ });
		m_sceneModels.push_back(std::move(model));
	}
}

bool GraphicsClass::TestAABBIntersection(XMFLOAT3 lb, XMFLOAT3 rt, XMFLOAT3 origin, XMFLOAT3 dirfrac)
{
	assert(lb.x <= rt.x);
	assert(lb.y <= rt.y);
	assert(lb.z <= rt.z);

	const float t1 = (lb.x - origin.x)*dirfrac.x;
	const float t2 = (rt.x - origin.x)*dirfrac.x;
	const float t3 = (lb.y - origin.y)*dirfrac.y;
	const float t4 = (rt.y - origin.y)*dirfrac.y;
	const float t5 = (lb.z - origin.z)*dirfrac.z;
	const float t6 = (rt.z - origin.z)*dirfrac.z;

	const float tmin = max(max(min(t1, t2), min(t3, t4)), min(t5, t6));
	const float tmax = min(min(max(t1, t2), max(t3, t4)), max(t5, t6));

	// if tmax < 0, ray (line) is intersecting AABB, but the whole AABB is behind us
	if (tmax < 0)
	{
		return false;
	}

	// if tmin > tmax, ray doesn't intersect AABB
	if (tmin > tmax)
	{
		return false;
	}
	return true;
}

bool GraphicsClass::TryPickModelPickerArrow(ModelClass* model, const ModelPicker::Axis axis, XMFLOAT3&& origin, XMFLOAT3 dirfrac)
{
	XMMATRIX wMatrix;
	m_D3D->GetWorldMatrix(wMatrix);
	const XMFLOAT3 lb = m_modelPicker->GetMinBounds(model, axis);
	m_D3D->GetWorldMatrix(wMatrix);
	const XMFLOAT3 rt = m_modelPicker->GetMaxBounds(model, axis);

	return TestAABBIntersection(lb, rt, origin, dirfrac);
}
