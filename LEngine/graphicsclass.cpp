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
	m_Camera->SetPosition(0.0f, 0.0f, 0.0f);
	
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
	m_pbrShader->m_lightDirection = XMFLOAT4(-1.0f, 0.0, -1.0f, 1.5f);
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
	m_rotationY += 0.01f;
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

	// Get the world, view, and projection matrices from the camera and d3d objects.
	m_Camera->GetViewMatrix(viewMatrix);
	m_D3D->GetWorldMatrix(worldMatrix);
	m_D3D->GetProjectionMatrix(projectionMatrix);

	// Put the model vertex and index buffers on the graphics pipeline to prepare them for drawing.
	//m_Model->Render(m_D3D->GetDeviceContext());
/*
	//worldMatrix = DirectX::XMMatrixScaling(0.2f, 0.2f, 0.2f);
	worldMatrix = DirectX::XMMatrixMultiply(worldMatrix, DirectX::XMMatrixRotationY(m_rotationY / 3.14f));
	// Render the model using the color shader.
	m_pbrShader->m_cameraPosition = m_Camera->GetPosition();
	//m_ColorShader->SetCameraPosition(m_Camera->GetPosition());

	result = m_pbrShader->Render(m_D3D->GetDeviceContext(), m_Model->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix);
	if (!result)
		return false;

	m_D3D->EnableAlphaBlending();

	result = m_debugBackground->Render(m_D3D->GetDeviceContext(), m_Model->GetIndexCount(), worldMatrix * 0, viewMatrix, projectionMatrix);
	if(!result)
		return false;

	result = m_roughnessSlider->Render(m_D3D->GetDeviceContext(), m_Model->GetIndexCount(), worldMatrix * 0, viewMatrix, projectionMatrix);
	if (!result)
		return false;

	result = m_metalnessSlider->Render(m_D3D->GetDeviceContext(), m_Model->GetIndexCount(), worldMatrix * 0, viewMatrix, projectionMatrix);
	if (!result)
		return false;

	m_D3D->DisableAlphaBlending();
	
	m_texturePreviewRoughness->Render(m_D3D->GetDeviceContext(), m_Model->GetIndexCount(), worldMatrix * 0, viewMatrix, projectionMatrix);

	m_texturePreviewMetalness->Render(m_D3D->GetDeviceContext(), m_Model->GetIndexCount(), worldMatrix * 0, viewMatrix, projectionMatrix);
	
	m_texturePreviewNormal->Render(m_D3D->GetDeviceContext(), m_Model->GetIndexCount(), worldMatrix * 0, viewMatrix, projectionMatrix);

	m_texturePreviewAlbedo->Render(m_D3D->GetDeviceContext(), m_Model->GetIndexCount(), worldMatrix * 0, viewMatrix, projectionMatrix);

	m_textEngine->RenderText(m_D3D->GetDeviceContext(), m_screenWidth, m_screenHeight);
*/
	worldMatrix = DirectX::XMMatrixMultiply(worldMatrix, DirectX::XMMatrixRotationY(m_Camera->GetRotation().y / 3.14f));
	XMMATRIX scaleMatrix = XMMatrixScaling(1.7f, 1.7f, 1.7f);
	worldMatrix = DirectX::XMMatrixMultiply(worldMatrix, scaleMatrix);

	m_D3D->ChangeRasterizerCulling(D3D11_CULL_FRONT);
	m_D3D->ChangeDepthStencilComparison(D3D11_COMPARISON_LESS_EQUAL);

	m_Model->Render(m_D3D->GetDeviceContext());
	m_skyboxShader->Render(m_D3D->GetDeviceContext(), m_Model->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix);

	//ALWAYS RENDER MOUSE AT THE VERY END
	//result = m_mouse->Render(m_D3D->GetDeviceContext(), m_Model->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix);
	//if (!result)
	//	return false;

	m_D3D->ChangeRasterizerCulling(D3D11_CULL_BACK);
	m_D3D->ChangeDepthStencilComparison(D3D11_COMPARISON_LESS);
	// Present the rendered scene to the screen.
	m_D3D->EndScene();

	return true;
}