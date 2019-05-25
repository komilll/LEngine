////////////////////////////////////////////////////////////////////////////////
// Filename: systemclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "systemclass.h"


SystemClass::SystemClass()
{
	m_Input = 0;
	m_Graphics = 0;
}


SystemClass::SystemClass(const SystemClass& other)
{
}


SystemClass::~SystemClass()
{
}


bool SystemClass::Initialize()
{
	int screenWidth, screenHeight;
	bool result;

	// Initialize the width and height of the screen to zero before sending the variables into the function.
	screenWidth = 0;
	screenHeight = 0;

	// Initialize the windows api.
	InitializeWindows(screenWidth, screenHeight);

	// Create the input object.  This object will be used to handle reading the keyboard input from the user.
	m_Input = new InputClass;
	if(!m_Input)
	{
		return false;
	}

	// Initialize the input object.
	m_Input->Initialize();

	// Create the graphics object.  This object will handle rendering all the graphics for this application.
	m_Graphics = new GraphicsClass;
	if(!m_Graphics)
	{
		return false;
	}

	// Initialize the graphics object.
	result = m_Graphics->Initialize(screenWidth, screenHeight, m_hwnd);
	if(!result)
	{
		return false;
	}

	m_Mouse = new MouseClass;
	if (!m_Mouse)
		return false;
	if (!m_Mouse->Initialize(m_Graphics->GetD3D(), m_hinstance, m_hwnd, screenWidth, screenHeight))
		return false;

	m_Graphics->SetMouseRef(m_Mouse);
	m_Mouse->m_inputClass = m_Input;
	
	return true;
}


void SystemClass::Shutdown()
{
	// Release the graphics object.
	if(m_Graphics)
	{
		m_Graphics->Shutdown();
		delete m_Graphics;
		m_Graphics = 0;
	}

	// Release the input object.
	if(m_Input)
	{
		delete m_Input;
		m_Input = 0;
	}

	// Shutdown the window.
	ShutdownWindows();
	
	return;
}


void SystemClass::Run()
{
	MSG msg;
	bool done, result;


	// Initialize the message structure.
	ZeroMemory(&msg, sizeof(MSG));
	
	// Loop until there is a quit message from the window or the user.
	done = false;
	while(!done)
	{
		// Handle the windows messages.
		if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		// If windows signals to end the application then exit out.
		if(msg.message == WM_QUIT)
		{
			done = true;
		}
		else
		{
			// Otherwise do the frame processing.
			result = Frame();
			if(!result)
			{
				done = true;
			}
		}

	}

	return;
}


bool SystemClass::Frame()
{
	if (m_Mouse)
	{
		if (m_Mouse->GetLMBPressed())
			MessageHandler(m_hwnd, WM_KEYDOWN, 17, -1);
		else
			MessageHandler(m_hwnd, WM_KEYUP, 17, -1);
	}

	if(m_Input->IsKeyDown(VK_ESCAPE))
		return false;

	if (m_Input->IsKeyDown(VK_TAB))
	{
		if (m_changeToMaterialEditorPressed == false)
		{
			m_changeToMaterialEditorPressed = true;
			m_Graphics->ChangeRenderWindow();
		}
	}
	else
	{
		m_changeToMaterialEditorPressed = false;
	}

	HandleInput();

	if (m_Mouse)
	{
		if (!m_Mouse->Frame())
			return false;
		m_lmbPressed = m_Mouse->GetLMBPressed();
	}

	if (m_Graphics->Frame() == false)
		return false;
	
	if (m_Graphics->RENDER_MATERIAL_EDITOR)
		m_Graphics->UpdateShaderEditorMouseOnly();

	return true;
}


extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
LRESULT CALLBACK SystemClass::MessageHandler(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam)
{
	if (ImGui::GetCurrentContext() == NULL)
	{
		ImGui::CreateContext();
	}

	if (ImGui_ImplWin32_WndProcHandler(hwnd, umsg, wparam, lparam))
		return true;

	switch(umsg)
	{
		// Check if a key has been pressed on the keyboard.
		case WM_KEYDOWN:
		{
			// If a key is pressed send it to the input object so it can record that state.
			m_Input->KeyDown((unsigned int)wparam);
			return 0;
		}

		// Check if a key has been released on the keyboard.
		case WM_KEYUP:
		{
			// If a key is released then send it to the input object so it can unset the state for that key.
			m_Input->KeyUp((unsigned int)wparam);
			return 0;
		}

		// Any other messages send to the default message handler as our application won't make use of them.
		default:
		{
			return DefWindowProc(hwnd, umsg, wparam, lparam);
		}
	}
}


void SystemClass::InitializeWindows(int& screenWidth, int& screenHeight)
{
	WNDCLASSEX wc;
	DEVMODE dmScreenSettings;
	int posX, posY;


	// Get an external pointer to this object.	
	ApplicationHandle = this;

	// Get the instance of this application.
	m_hinstance = GetModuleHandle(NULL);

	// Give the application a name.
	m_applicationName = "Engine";

	// Setup the windows class with default settings.
	wc.style         = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc   = WndProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = m_hinstance;
	wc.hIcon		 = LoadIcon(NULL, IDI_WINLOGO);
	wc.hIconSm       = wc.hIcon;
	wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.lpszMenuName  = NULL;
	wc.lpszClassName = m_applicationName;
	wc.cbSize        = sizeof(WNDCLASSEX);
	
	// Register the window class.
	RegisterClassEx(&wc);

	// Determine the resolution of the clients desktop screen.
	screenWidth  = GetSystemMetrics(SM_CXSCREEN);
	screenHeight = GetSystemMetrics(SM_CYSCREEN);

	// Setup the screen settings depending on whether it is running in full screen or in windowed mode.
	if(FULL_SCREEN)
	{
		// If full screen set the screen to maximum size of the users desktop and 32bit.
		memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
		dmScreenSettings.dmSize       = sizeof(dmScreenSettings);
		dmScreenSettings.dmPelsWidth  = (unsigned long)screenWidth;
		dmScreenSettings.dmPelsHeight = (unsigned long)screenHeight;
		dmScreenSettings.dmBitsPerPel = 32;			
		dmScreenSettings.dmFields     = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		// Change the display settings to full screen.
		ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN);

		// Set the position of the window to the top left corner.
		posX = posY = 0;
	}
	else
	{
		// If windowed then set it to 1280x720 resolution.
		screenWidth  = m_windowSizeX;
		screenHeight = m_windowSizeY;

		// Place the window in the middle of the screen.
		posX = (GetSystemMetrics(SM_CXSCREEN) - screenWidth)  / 2;
		posY = (GetSystemMetrics(SM_CYSCREEN) - screenHeight) / 2;
	}

	// Create the window with the screen settings and get the handle to it.
	m_hwnd = CreateWindowEx(WS_EX_APPWINDOW, m_applicationName, m_applicationName, 
						    WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP,
						    posX, posY, screenWidth, screenHeight, NULL, NULL, m_hinstance, NULL);

	// Bring the window up on the screen and set it as main focus.
	ShowWindow(m_hwnd, SW_SHOW);
	SetForegroundWindow(m_hwnd);
	SetFocus(m_hwnd);

	// Hide the mouse cursor.
	ShowCursor(true);

	return;
}


void SystemClass::ShutdownWindows()
{
	// Show the mouse cursor.
	ShowCursor(true);

	// Fix the display settings if leaving full screen mode.
	if(FULL_SCREEN)
	{
		ChangeDisplaySettings(NULL, 0);
	}

	// Remove the window.
	DestroyWindow(m_hwnd);
	m_hwnd = NULL;

	// Remove the application instance.
	UnregisterClass(m_applicationName, m_hinstance);
	m_hinstance = NULL;

	// Release the pointer to this class.
	ApplicationHandle = NULL;

	return;
}

void SystemClass::HandleInput()
{
	XMVECTOR cameraMovement{ 0, 0, 0 };
	XMVECTOR cameraRotation{0, 0, 0};
	float movementPerTick = 0.3f;
	float rotatePerTick = 0.2f;

	if (m_Input->IsKeyDown(VK_W))
		m_Graphics->MoveCameraForward(movementPerTick);
	if (m_Input->IsKeyDown(VK_S))
		m_Graphics->MoveCameraBackward(movementPerTick);
	if (m_Input->IsKeyDown(VK_A))
		m_Graphics->MoveCameraLeft(movementPerTick);
	if (m_Input->IsKeyDown(VK_D))
		m_Graphics->MoveCameraRight(movementPerTick);
	if (m_Input->IsKeyDown(VK_E))
		m_Graphics->MoveCameraUp(movementPerTick);
	if (m_Input->IsKeyDown(VK_Q))
		m_Graphics->MoveCameraDown(movementPerTick);


	//Handle deleting blocks in Shader Editor
	if (m_Input->IsKeyDown(VK_DELETE))
	{
		m_Input->KeyUp(VK_DELETE);
		m_Graphics->DeleteCurrentShaderBlock();
	}

	if (m_Graphics->RENDER_MATERIAL_EDITOR)
	{
		if (m_Graphics->IsChoosingShaderWindowActive())
		{
			if (m_Input->IsKeyDown(VK_UP))
				m_Graphics->ChangeChoosingWindowShaderFocus(GraphicsClass::ShaderWindowDirection::Up);
			else if (m_Input->IsKeyDown(VK_DOWN))
				m_Graphics->ChangeChoosingWindowShaderFocus(GraphicsClass::ShaderWindowDirection::Down);
			else if (m_Input->IsAlphanumericKeyDown())
			{
				m_Graphics->FocusOnChoosingWindowsShader();
			}

			m_Input->KeyUp(VK_UP);
			m_Input->KeyUp(VK_DOWN);

			if (m_Input->IsKeyDown(VK_RETURN))
			{
				m_Graphics->AcceptCurrentChoosingWindowShader();
			}
		}
		if ((::GetKeyState(VK_CONTROL) & 0x8000) != 0 && !m_Mouse->GetLMBPressed())
		{
			if (m_Input->IsKeyDown(VK_C))
			{
				m_Input->KeyUp(VK_C);
				m_Graphics->CopyBlocks();
			}
			else if (m_Input->IsKeyDown(VK_V))
			{
				m_Input->KeyUp(VK_V);
				m_Graphics->PasteBlocks();
			}
		}
	}
	else
	{
		if (m_Input->IsKeyDown(VK_RIGHT))
			cameraRotation = XMVectorAdd(cameraRotation, XMVECTOR{ 0, rotatePerTick, 0 });
		if (m_Input->IsKeyDown(VK_LEFT))
			cameraRotation = XMVectorAdd(cameraRotation, XMVECTOR{ 0, -rotatePerTick, 0 });
		if (m_Input->IsKeyDown(VK_UP))
			cameraRotation = XMVectorAdd(cameraRotation, XMVECTOR{ -rotatePerTick, 0, 0 });
		if (m_Input->IsKeyDown(VK_DOWN))
			cameraRotation = XMVectorAdd(cameraRotation, XMVECTOR{ rotatePerTick, 0, 0 });

		m_Graphics->RotateCamera(cameraRotation);
	}
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam)
{
	switch(umessage)
	{
		// Check if the window is being destroyed.
		case WM_DESTROY:
		{
			PostQuitMessage(0);
			return 0;
		}

		// Check if the window is being closed.
		case WM_CLOSE:
		{
			PostQuitMessage(0);		
			return 0;
		}

		// All other messages pass to the message handler in the system class.
		default:
		{
			return ApplicationHandle->MessageHandler(hwnd, umessage, wparam, lparam);
		}
	}
}