#pragma once

#define DIRECTINPUT_VERSION 0x0800

#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

#include <dinput.h>
#include "d3dclass.h"
#include "inputclass.h"
#include <utility>

#include <windows.h>

class MouseClass
{
public:
	bool Initialize(D3DClass* d3d, HINSTANCE hInstance, HWND hwnd, int screenWidth, int screenHeight);
	void Shutdown();
	bool Frame();
	bool ReadMouse();
	void ProcessInput();
	void GetMouseLocation(int &mouseX, int &mouseY);
	///<summary> Return [-1, 1] </summary>
	void GetMouseLocationScreenSpace(float &mouseX, float &mouseY);
	bool SetCursorPosition(XMFLOAT2 screenPos, bool clamped = false);
	std::pair<float, float> GetMouseMovementFrame() const;

	void SetLMBPressed(bool enable);
	void SetRMBPressed(bool enable);
	void SetMMBPressed(bool enable);

	bool GetLMBPressed() const;
	bool GetRMBPressed() const;
	bool GetMMBPressed() const;
	int GetMouseScroll() const;

	POINT CurrentMouseLocation() const;
	std::pair<float, float> MouseFrameMovement();
	
	D3DClass* GetD3D();

	void SetVisibility(BOOL visible);

private:
	void CalculateMouseMovement();
	POINT CalculateMousePosition();

public:
	bool isInputConsumed = false;
	InputClass* m_inputClass;

private:
	IDirectInput8* m_directInput;
	IDirectInputDevice8* m_mouse;

	DIMOUSESTATE m_mouseState;

	POINT m_lastMousePoint;
	POINT m_frameMovement;
	int m_screenWidth, m_screenHeight;
	float m_mouseX, m_mouseY;

	D3DClass* m_d3d;
	BOOL m_visible{ TRUE };
};