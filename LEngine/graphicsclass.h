////////////////////////////////////////////////////////////////////////////////
// Filename: graphicsclass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _GRAPHICSCLASS_H_
#define _GRAPHICSCLASS_H_


///////////////////////
// MY CLASS INCLUDES //
///////////////////////
#include "d3dclass.h"
#include "cameraclass.h"
#include "modelclass.h"
#include "colorshaderclass.h"
#include "ShaderSpecularClass.h"
#include "UIBase.h"
#include "TextEngine.h"
#include "UIBackground.h"
#include "UIButton.h"
#include "UITick.h"
#include "MouseClassContainer.h"
#include "UISlider.h"
#include "ShaderPBRClass.h"

/////////////
// GLOBALS //
/////////////
const bool FULL_SCREEN = false;
const bool VSYNC_ENABLED = true;
const float SCREEN_DEPTH = 1000.0f;
const float SCREEN_NEAR = 0.1f;


////////////////////////////////////////////////////////////////////////////////
// Class name: GraphicsClass
////////////////////////////////////////////////////////////////////////////////
class GraphicsClass
{
public:
	GraphicsClass();
	GraphicsClass(const GraphicsClass&);
	~GraphicsClass();

	bool Initialize(int, int, HWND);
	void Shutdown();
	bool Frame();
	void MoveCameraForward();
	void MoveCameraBackward();
	void MoveCameraLeft();
	void MoveCameraRight();
	void MoveCameraUp();
	void MoveCameraDown();
	void RotateCamera(XMVECTOR rotation);

	void UpdateUI();
	void SetMouseRef(MouseClass* mouse);
		
	D3DClass* GetD3D();

	TextEngine::FontData* AddText(float&& posX, float&& posY, std::string&& text, float&& scale = 1.0f, TextEngine::Align &&align = TextEngine::Align::LEFT, XMVECTOR&& color = DirectX::Colors::White);

private:
	bool Render();

private:
	D3DClass* m_D3D;
	CameraClass* m_Camera;
	ModelClass* m_Model;
	ColorShaderClass* m_ColorShader;
	ShaderSpecularClass* m_specularShader;
	ShaderPBRClass* m_pbrShader;
	TextEngine* m_textEngine;

	UIBackground* m_debugBackground;
	UITick* m_debugTick;
	MouseClassContainer* m_mouse;
	UISlider* m_debugSlider;

	float m_rotationY = 0.0f;
	int m_screenWidth = 0;
	int m_screenHeight = 0;
};

#endif