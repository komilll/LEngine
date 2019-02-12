////////////////////////////////////////////////////////////////////////////////
// Filename: graphicsclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "graphicsclass.h"


GraphicsClass::GraphicsClass()
{
	m_D3D = 0;
	m_Camera = 0;
	m_Model = 0;
	m_ColorShader = 0;
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
	m_Camera->SetPosition(0.0f, 0.0f, -3.0f);
	
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

	if (!(m_skyboxShader = new SkyboxShaderClass))
		return false;
	if (!m_skyboxShader->Initialize(m_D3D->GetDevice(), *m_D3D->GetHWND(), L"skybox.vs", L"skybox.ps", input))
		return false;

	m_skyboxShader->LoadTexture(m_D3D->GetDevice(), L"skybox.dds", m_skyboxShader->m_skyboxTexture, m_skyboxShader->m_skyboxTextureView);

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

		//Blur shader - vertical and horizontal
		m_blurShaderHorizontal = new BlurShaderClass;
		if (!m_blurShaderHorizontal->Initialize(m_D3D->GetDevice(), *m_D3D->GetHWND(), L"blurHorizontal.vs", L"blurHorizontal.ps", input))
			return false;

		m_blurShaderVertical = new BlurShaderClass;
		if (!m_blurShaderVertical->Initialize(m_D3D->GetDevice(), *m_D3D->GetHWND(), L"blurVertical.vs", L"blurVertical.ps", input))
			return false;
#pragma endregion
	}
	else
	{
		float textureWidth = 256;
		float textureHeight = 256;
		//RENDER SCENE TO TEXTURE
		if (!(m_renderTexture = new RenderTextureClass))
			return false;
		if (!(m_renderTexture->Initialize(m_D3D->GetDevice(), textureWidth, textureHeight)))
			return false;

		m_renderTexture->LoadTexture(m_D3D->GetDevice(), L"seafloor.dds", m_renderTexture->GetShaderResource(), m_renderTexture->GetShaderResourceView());

		if (!(m_renderTexturePreview = new UITexture))
			return false;
		if (!(m_renderTexturePreview->Initialize(m_D3D, ModelClass::ShapeSize::RECTANGLE, -1.0f, 1.0f, 1.0f, -1.0f, true)))
			return false;

		m_renderTexturePreview->BindTexture(m_renderTexture->GetShaderResourceView());

		//Texture downsampled
		if (!(m_skyboxDownsampled = new RenderTextureClass))
			return false;
		if (!(m_skyboxDownsampled->Initialize(m_D3D->GetDevice(), 64, 64, RenderTextureClass::Scaling::NONE)))
			return false;

		if (!(m_renderTextureDownsampled = new RenderTextureClass))
			return false;
		if (!(m_renderTextureDownsampled->Initialize(m_D3D->GetDevice(), 64, 64, RenderTextureClass::Scaling::DOWNSCALE)))
			return false;

		m_skyboxShader->CreateColorSkybox(m_D3D->GetDevice(), 256, 256);
		//DownsampleSkybox();
		//DownsampleTexture();
		//UpscaleTexture();
	}

	return true;
}


void GraphicsClass::Shutdown()
{
	// Release the color shader object.
	if(m_ColorShader)
	{
		m_ColorShader->Shutdown();
		delete m_ColorShader;
		m_ColorShader = 0;
	}

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
		BlurFilter(false);
		BlurFilter(true);
		UpscaleTexture();

		result = m_renderTexturePreview->Render(m_D3D->GetDeviceContext(), 0, worldMatrix, viewMatrix, projectionMatrix);
		if (!result)
			return false;
	}
	else
	{
		////m_debug++;

		//if (m_debug == 300)
		//	DownsampleTexture();
		////else if (m_debug == 600)
		////	BlurFilter(false);
		////else if (m_debug == 900)
		////	BlurFilter(true);
		//else if (m_debug == 600)
		//	UpscaleTexture();
		//DownsampleSkybox();
		//RenderSkybox();

		//result = m_renderTexturePreview->Render(m_D3D->GetDeviceContext(), 0, worldMatrix, viewMatrix, projectionMatrix);
		//if (!result)
		//	return false;

		result = RenderScene();
		if (!result)
			return false;
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
	XMMATRIX worldMatrix, viewMatrix, projectionMatrix;
	bool result;

	m_Camera->GetViewMatrix(viewMatrix);
	m_D3D->GetWorldMatrix(worldMatrix);
	m_D3D->GetProjectionMatrix(projectionMatrix);
	//RENDER MAIN SCENE MODEL
	m_D3D->ChangeRasterizerCulling(D3D11_CULL_BACK);
	m_D3D->ChangeDepthStencilComparison(D3D11_COMPARISON_LESS_EQUAL);

	m_Model->Render(m_D3D->GetDeviceContext());
	m_pbrShader->m_cameraPosition = m_Camera->GetPosition();
	//XMVECTOR light_ = XMVector3Rotate(XMVECTOR{ -1.0f, 0.0, -1.0f, 0.0f }, XMVECTOR{ m_Camera->GetRotation().x / 3.14f, m_Camera->GetRotation().y / 3.14f, m_Camera->GetRotation().z / 3.14f, 0.0f });
	//m_pbrShader->m_lightDirection = XMFLOAT4(light_.m128_f32[0], 0.0, light_.m128_f32[2], 1.2f);

	m_D3D->GetWorldMatrix(worldMatrix);
	worldMatrix = DirectX::XMMatrixMultiply(worldMatrix, DirectX::XMMatrixRotationY(m_Camera->GetRotation().y / 3.14f));
	worldMatrix = DirectX::XMMatrixMultiply(worldMatrix, DirectX::XMMatrixRotationX(m_Camera->GetRotation().x / 3.14f));

	result = m_pbrShader->Render(m_D3D->GetDeviceContext(), m_Model->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix);
	if (!result)
		return false;

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

bool GraphicsClass::BlurFilter(bool vertical)
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
	XMFLOAT3 position = XMFLOAT3(0, 0, 0);
	XMVECTOR tar[] = { XMVectorSet(1, 0, 0, 0), XMVectorSet(-1, 0, 0, 0), XMVectorSet(0, 1, 0, 0), XMVectorSet(0, -1, 0, 0), XMVectorSet(0, 0, 1, 0), XMVectorSet(0, 0, -1, 0) };
	XMVECTOR up[] = { XMVectorSet(0, 1, 0, 0), XMVectorSet(0, 1, 0, 0), XMVectorSet(0, 0, -1, 0), XMVectorSet(0, 0, 1, 0), XMVectorSet(0, 1, 0, 0), XMVectorSet(0, 1, 0, 0) };
	XMMATRIX worldMatrix, viewMatrix, projectionMatrix;
	bool result;

	float color[4]{ 0, 0, 0, 1 };
	m_renderTexturePreview->LoadTexture(m_D3D->GetDevice(), L"skybox.dds", m_renderTexturePreview->GetShaderResource(), m_renderTexturePreview->GetShaderResourceView());
	m_skyboxShader->LoadTexture(m_D3D->GetDevice(), L"skybox.dds", m_skyboxShader->m_skyboxTexture, m_skyboxShader->m_skyboxTextureView);

	m_D3D->TurnZBufferOff();
	for (int i = 0; i < 6; i++)
	{
		m_D3D->GetDeviceContext()->OMSetRenderTargets(1, &m_skyboxDownsampled->GetShaderTargetView(i), 0);
		m_skyboxDownsampled->SetViewport(m_D3D->GetDeviceContext());
		m_D3D->GetDeviceContext()->ClearRenderTargetView(m_skyboxDownsampled->GetShaderTargetView(i), color);
		//m_skyboxDownsampled->SetRenderTarget(m_D3D->GetDeviceContext(), m_D3D->GetDepthStencilView());
		//m_skyboxDownsampled->ClearRenderTarget(m_D3D->GetDeviceContext(), m_D3D->GetDepthStencilView(), 1.0f, 0.0f, 0.0f, 1.0f);

		XMVECTOR dir = XMVector3Rotate(tar[i], XMQuaternionIdentity());
		XMMATRIX view = DirectX::XMMatrixLookToLH(XMLoadFloat3(&position), dir, up[i]);
		viewMatrix = DirectX::XMMatrixTranspose(view);

		XMMATRIX projection = DirectX::XMMatrixPerspectiveFovLH(0.5f * XM_PI, 1.0f, 0.1f, 100.0f);
		projectionMatrix = DirectX::XMMatrixTranspose(projection);

		m_Model->Render(m_D3D->GetDeviceContext());
		result = m_skyboxShader->Render(m_D3D->GetDeviceContext(), m_Model->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix);
		if (!result)
			return false;
	}
	m_D3D->TurnZBufferOn();

	m_D3D->SetBackBufferRenderTarget();
	m_D3D->ResetViewport();

	SaveDDSTextureToFile(m_D3D->GetDeviceContext(), m_skyboxDownsampled->GetShaderResource(), L"test.dds");
	//m_skyboxShader->m_skyboxTextureView = m_skyboxDownsampled->GetShaderResourceView();

	return true;
}