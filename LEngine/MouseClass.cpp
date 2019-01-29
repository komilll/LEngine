#include "MouseClass.h"

MouseClass::MouseClass()
{
	m_directInput = 0;
	m_mouse = 0;
}

MouseClass::MouseClass(const MouseClass &)
{
}

MouseClass::~MouseClass()
{
}

bool MouseClass::Initialize(HINSTANCE hInstance, HWND hwnd, int screenWidth, int screenHeight)
{
	HRESULT result;

	m_screenWidth = screenWidth;
	m_screenHeight = screenHeight;

	m_mouseX = 0;
	m_mouseY = 0;

	result = DirectInput8Create(hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&m_directInput, NULL);
	if (FAILED(result))
	{
		return false;
	}

	result = m_directInput->CreateDevice(GUID_SysMouse, &m_mouse, NULL);
	if (FAILED(result))
	{
		return false;
	}

	result = m_mouse->SetDataFormat(&c_dfDIMouse);
	if (FAILED(result))
	{
		return false;
	}

	result = m_mouse->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_EXCLUSIVE);
	if (FAILED(result))
	{
		return false;
	}

	result = m_mouse->Acquire();
	if (FAILED(result))
	{
		return false;
	}

	return true;
}

void MouseClass::Shutdown()
{
	if (m_directInput)
	{
		m_directInput->Release();
		m_directInput = 0;
	}

	if (m_mouse)
	{
		m_mouse->Release();
		m_mouse = 0;
	}
}

bool MouseClass::Frame()
{
	bool result;

	result = ReadMouse();
	if (!result)
	{
		return false;
	}

	ProcessInput();

	return true;
}

bool MouseClass::ReadMouse()
{
	HRESULT result;

	result = m_mouse->GetDeviceState(sizeof(DIMOUSESTATE), (LPVOID)&m_mouseState);
	if (FAILED(result))
	{
		if (result == DIERR_INPUTLOST || result == DIERR_NOTACQUIRED)
		{
			m_mouse->Acquire();
		}
		else
		{
			return false;
		}
	}

	return true;
}

void MouseClass::ProcessInput()
{
	// Update the location of the mouse cursor based on the change of the mouse location during the frame.
	m_mouseX += m_mouseState.lX;
	m_mouseY += m_mouseState.lY;

	// Ensure the mouse location doesn't exceed the screen width or height.
	if (m_mouseX < 0) { m_mouseX = 0; }
	if (m_mouseY < 0) { m_mouseY = 0; }

	if (m_mouseX > m_screenWidth) { m_mouseX = m_screenWidth; }
	if (m_mouseY > m_screenHeight) { m_mouseY = m_screenHeight; }
}

void MouseClass::GetMouseLocation(int & mouseX, int & mouseY)
{
	mouseX = m_mouseX;
	mouseY = m_mouseY;
}

void MouseClass::GetMouseLocationScreenSpace(float & mouseX, float & mouseY)
{
	mouseX = (float)m_mouseX / (float)m_screenWidth;
	mouseY = (float)m_mouseY / (float)m_screenHeight;
}

bool MouseClass::SetMouseLocation(int mouseX, int mouseY)
{
	m_mouseState.lX += mouseX;
	m_mouseState.lY += mouseY;

	ProcessInput();

	return true;
}

bool MouseClass::GetLMBPressed()
{
	return m_mouseState.rgbButtons[0];
}

bool MouseClass::GetRMBPressed()
{
	return m_mouseState.rgbButtons[1];
}

void MouseClass::SetRMBPressed(bool enable)
{
	m_mouseState.rgbButtons[1] = enable;
}

void MouseClass::SetLMBPressed(bool enable)
{
	m_mouseState.rgbButtons[0] = enable;
}