#include "MouseClass.h"

bool MouseClass::Initialize(D3DClass* d3d, HINSTANCE hInstance, HWND hwnd, int screenWidth, int screenHeight)
{
	//TODO Create in constructor
	m_d3d = d3d;
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

	m_lastMousePoint = CalculateMousePosition();

	m_frameMovement.x = 0;
	m_frameMovement.y = 0;

	return true;
}

void MouseClass::Shutdown()
{
	//TODO Upgrade or remove completely
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
	if (!ReadMouse())
		return false;

	ProcessInput();
	CalculateMouseMovement();

	return true;
}

bool MouseClass::ReadMouse()
{
	const HRESULT result = m_mouse->GetDeviceState(sizeof(DIMOUSESTATE), (LPVOID)&m_mouseState);
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
	m_mouseY -= m_mouseState.lY;

	// Ensure the mouse location doesn't exceed the screen width or height.
	if (m_mouseX < -m_screenWidth) { m_mouseX = -m_screenWidth; }
	if (m_mouseY < -m_screenHeight) { m_mouseY = -m_screenHeight; }

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
	mouseX = static_cast<float>(m_mouseX) / static_cast<float>(m_screenWidth);
	mouseY = static_cast<float>(m_mouseY) / static_cast<float>(m_screenHeight);
}

std::pair<float, float> MouseClass::GetMouseMovementFrame() const
{
	return { m_mouseState.lX, m_mouseState.lY };
}

bool MouseClass::GetLMBPressed() const
{
	return m_mouseState.rgbButtons[0];
}

bool MouseClass::GetRMBPressed() const
{
	return m_mouseState.rgbButtons[1];
}

void MouseClass::SetRMBPressed(bool enable)
{
	m_mouseState.rgbButtons[1] = enable;
}

bool MouseClass::GetMMBPressed() const
{
	return m_mouseState.rgbButtons[2];
}

void MouseClass::SetMMBPressed(bool enable)
{
	m_mouseState.rgbButtons[2] = enable;
}

int MouseClass::GetMouseScroll() const
{
	return (m_mouseState.lZ / 120.0f);
}

POINT MouseClass::CurrentMouseLocation() const
{
	return m_lastMousePoint;
}

std::pair<float, float> MouseClass::MouseFrameMovement()
{
	const POINT p = m_frameMovement;
	float mouseX{ 0 };
	float mouseY{ 0 };

	//Calculate mouse X
	mouseX = static_cast<float>(p.x) / static_cast<float>(GetD3D()->GetWindowSize().x);
	if (mouseX > 1.0f)
		mouseX = 1.0f;
	else if (mouseX < -1.0f)
		mouseX = -1.0f;

	//Calculate mouse Y
	mouseY = static_cast<float>(p.y) / static_cast<float>(GetD3D()->GetWindowSize().y);
	if (mouseY > 1.0f)
		mouseY = 1.0f;
	else if (mouseY < -1.0f)
		mouseY = -1.0f;

	mouseY *= -1.0f;

	return std::pair<float, float>{ mouseX, mouseY };
}

D3DClass * MouseClass::GetD3D()
{
	return m_d3d;
}

void MouseClass::SetVisibility(BOOL visible)
{
	if (m_visible != visible)
	{
		m_visible = visible;
		ShowCursor(visible);
	}
}

void MouseClass::CalculateMouseMovement()
{
	const POINT currentMousePos = CalculateMousePosition();
	m_frameMovement = { currentMousePos.x - m_lastMousePoint.x, currentMousePos.y - m_lastMousePoint.y };
	m_lastMousePoint = currentMousePos;
}

bool MouseClass::SetCursorPosition(XMFLOAT2 screenPos, bool clamped)
{
	if (clamped)
	{
		screenPos.x = (screenPos.x * 2.0f) - 1.0f;
		screenPos.y = (screenPos.y * 2.0f) - 1.0f;
	}
	RECT desktop;
	const HWND hwnd = ::GetDesktopWindow();
	::GetWindowRect(hwnd, &desktop);
	POINT oldPoint;
	GetCursorPos(&oldPoint);
	
	POINT point = { static_cast<LONG>(desktop.right * 0.5), static_cast<LONG>(desktop.bottom * 0.5) };
	point.x += static_cast<LONG>(screenPos.x * (GetD3D()->GetWindowSize().x * 0.5f));
	point.y -= static_cast<LONG>(screenPos.y * (GetD3D()->GetWindowSize().y * 0.5f));

	return SetCursorPos(point.x, point.y);
}

POINT MouseClass::CalculateMousePosition()
{
	POINT p;
	if (!GetCursorPos(&p))
	{
		return{ 0,0 };
	}	

	RECT desktop;
	HWND hwnd = ::GetDesktopWindow();

	::GetWindowRect(hwnd, &desktop);
	//Calculate mouse X
	p.x -= (desktop.right - GetD3D()->GetWindowSize().x) * 0.5f;
	if (p.x > GetD3D()->GetWindowSize().x)
		p.x = GetD3D()->GetWindowSize().x;
	else if (p.x < 0.0f)
		p.x = 0.0f;

	//Calculate mouse Y
	p.y -= (desktop.bottom - GetD3D()->GetWindowSize().y) * 0.5f;
	if (p.y > GetD3D()->GetWindowSize().y)
		p.y = GetD3D()->GetWindowSize().y;
	else if (p.y < 0.0f)
		p.y = 0.0f;

	return p;
}

void MouseClass::SetLMBPressed(bool enable)
{
	m_mouseState.rgbButtons[0] = enable;
}