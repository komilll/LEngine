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

	m_screenWidth = screenWidth;
	m_screenHeight = screenHeight;

	// Create the Direct3D object.
	m_D3D = new D3DClass;
	if(!m_D3D)
	{
		return false;
	}

	// Initialize the Direct3D object.
	result = m_D3D->Initialize(screenWidth, screenHeight, VSYNC_ENABLED, hwnd, FULL_SCREEN, SCREEN_DEPTH, SCREEN_NEAR);
	if(!result)
	{
		//MessageBox(hwnd, L"Could not initialize Direct3D.", L"Error", MB_OK);
		return false;
	}

	// Create the camera object.
	m_Camera = new CameraClass;
	if(!m_Camera)
	{
		return false;
	}

	// Set the initial position of the camera.
	m_Camera->SetPosition(0.0f, 0.0f, -3.0f);
	
	// Create the model object.
	m_Model = new ModelClass;
	if(!m_Model)
	{
		return false;
	}

	// Initialize the model object.
	result = m_Model->Initialize(m_D3D->GetDevice(), "sphere.obj");
	if(!result)
	{
		//MessageBox(hwnd, L"Could not initialize the model object.", L"Error", MB_OK);
		return false;
	}

	if (!(m_pbrShader = new ShaderPBRClass))
		return false;
	
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

	if (!m_pbrShader->LoadTexture(m_D3D->GetDevice(), L"Wood.dds", m_pbrShader->m_diffuseTexture, m_pbrShader->m_diffuseTextureView))
		return false;
	if (!m_pbrShader->LoadTexture(m_D3D->GetDevice(), L"Wood_normal.dds", m_pbrShader->m_normalTexture, m_pbrShader->m_normalTextureView))
		return false;

	m_pbrShader->m_lightDirection = XMFLOAT3(-1.0f, 0.0, -1.0f);


	m_debugBackground = new UIBackground;
	if (!m_debugBackground->Initialize(m_D3D, ModelClass::ShapeSize::RECTANGLE, -0.95f, -0.5f, 0.9f, -0.0f))
		return false;
	m_debugBackground->ChangeColor(102.0f/255.0f, 163.0f/255.0f, 1.0f, 0.4f);

	m_debugTick = new UITick;
	if (!m_debugTick->Initialize(m_D3D, -0.9f, 0.85f, 0.03f))
		return false;

	m_textEngine = new TextEngine;
	m_textEngine->Initialize(m_D3D->GetDevice(), L"Fonts/font.spritefont");
	m_textEngine->WriteText(m_D3D->GetDeviceContext(), m_screenWidth, m_screenHeight, -0.75f, 0.85f, "Hello World", 0.5f, TextEngine::Align::CENTER);
	m_textEngine->WriteText(m_D3D->GetDeviceContext(), m_screenWidth, m_screenHeight, -0.75f, 0.75f, "TEST TEST", 0.5f, TextEngine::Align::CENTER, Colors::ForestGreen);

	//// Create the color shader object.
	//m_ColorShader = new ColorShaderClass;
	//if(!m_ColorShader)
	//{
	//	return false;
	//}

	//// Initialize the color shader object.
	//result = m_ColorShader->Initialize(m_D3D->GetDevice(), hwnd);
	//if(!result)
	//{
	//	//MessageBox(hwnd, L"Could not initialize the color shader object.", L"Error", MB_OK);
	//	return false;
	//}

	//result = m_ColorShader->LoadTexture(m_D3D->GetDevice(), L"Wood.dds", m_ColorShader->m_texture, m_ColorShader->m_textureView);
	//if (!result)
	//	return false;
	//m_ColorShader->SetLightDirection(-1.0f, 0.0f, -1.0f);

	m_mouse = new MouseClassContainer;

	m_debugSlider = new UISlider;
	if (!m_debugSlider->Initialize(m_D3D, -0.82f, -0.52f, 0.6f, 0.05f))
		return false;
	m_debugSlider->ChangeColor(226.0f/255.0f, 211.0f/255.0f, 90.0f/255.0f, 1.0f);
	m_debugSlider->CreateTextArea(AddText(-0.93f, 0.635f, "MORDO", 0.5f, TextEngine::Align::LEFT));
	m_debugSlider->EventOnChangeValue = [=](float roughness) { m_pbrShader->SetRoughness(roughness); };
	//m_debugSlider->EventsOnChangeValue.push_back([=](float metalness) { m_pbrShader->SetMetalness(metalness); });
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
		if (m_debugSlider->IsChanging())
		{
			m_debugSlider->ChangeSliderValue(m_mouse->GetMouse());
		}

		if (m_mouse->GetMouse()->isInputConsumed == true)
			return;

		if (m_debugTick->MouseOnArea(m_mouse->GetMouse()))
		{
			m_mouse->GetMouse()->isInputConsumed = true;
			m_debugTick->ChangeTick();
		}
		else if (m_debugSlider->MouseOnArea(m_mouse->GetMouse()))
		{
			m_mouse->GetMouse()->isInputConsumed = true;
			m_debugSlider->StartUsing();
			m_debugSlider->ChangeSliderValue(m_mouse->GetMouse());
		}
	}
	else
	{
		m_mouse->GetMouse()->isInputConsumed = false;
		if (m_debugSlider->IsChanging())
			m_debugSlider->EndUsing();
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
	m_Model->Render(m_D3D->GetDeviceContext());

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

	result = m_debugTick->Render(m_D3D->GetDeviceContext(), m_Model->GetIndexCount(), worldMatrix * 0, viewMatrix, projectionMatrix);
	if (!result)
		return false;

	result = m_debugSlider->Render(m_D3D->GetDeviceContext(), m_Model->GetIndexCount(), worldMatrix * 0, viewMatrix, projectionMatrix);
	if (!result)
		return false;

	m_D3D->DisableAlphaBlending();
	
	m_textEngine->RenderText(m_D3D->GetDeviceContext(), m_screenWidth, m_screenHeight);

	//ALWAYS RENDER MOUSE AT THE VERY END
	result = m_mouse->Render(m_D3D->GetDeviceContext(), m_Model->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix);
	if (!result)
		return false;

	// Present the rendered scene to the screen.
	m_D3D->EndScene();

	return true;
}