#pragma once

#define DIRECTINPUT_VERSION 0x0800

#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

#include <dinput.h>
#include "d3dclass.h"

class MouseClass
{
public:
	MouseClass();
	MouseClass(const MouseClass&);
	~MouseClass();

	bool Initialize(D3DClass* d3d, HINSTANCE hInstance, HWND hwnd, int screenWidth, int screenHeight);
	void Shutdown();
	bool Frame();
	bool ReadMouse();
	void ProcessInput();
	void GetMouseLocation(int &mouseX, int &mouseY);
	///<summary> Return [-1, 1] </summary>
	void GetMouseLocationScreenSpace(float &mouseX, float &mouseY);
	bool SetMouseLocation(int mouseX, int mouseY);
	bool GetLMBPressed();
	void SetLMBPressed(bool enable);
	bool GetRMBPressed();
	void SetRMBPressed(bool enable);
	
	D3DClass* GetD3D();

public:
	bool isInputConsumed = false;

private:
	IDirectInput8* m_directInput;
	IDirectInputDevice8* m_mouse;

	DIMOUSESTATE m_mouseState;

	int m_screenWidth, m_screenHeight;
	float m_mouseX, m_mouseY;
	float m_mouseMaxX, m_mouseMaxY;
	float m_mouseSpeedSlowdown = 4.0f;

	D3DClass* m_d3d;
};