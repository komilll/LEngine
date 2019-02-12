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
#include "UITexturePreview.h"
#include "SkyboxShaderClass.h"
#include "RenderTextureClass.h"
#include "UITexture.h"
#include "BlurShaderClass.h"
#include <ScreenGrab.h>

/////////////
// GLOBALS //
/////////////
const bool FULL_SCREEN = false;
const bool VSYNC_ENABLED = true;
const float SCREEN_DEPTH = 1000.0f;
const float SCREEN_NEAR = 0.1f;
const bool BLUR_BILINEAR = false;
const bool ENABLE_DEBUG = false;
const bool DRAW_SKYBOX = true;

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
	bool RenderScene();

	bool RenderDebugSettings();
	bool RenderSkybox();
	//BILINEAR SCREEN BLUR
	bool RenderSceneToTexture();
	bool DownsampleTexture();
	bool UpscaleTexture();
	bool BlurFilter(bool vertical); //Vertical = true; Horizontal = false
	//IBL DIFFUSE CONVOLUTION
	bool DownsampleSkybox();

private:
	D3DClass* m_D3D;
	CameraClass* m_Camera;
	ModelClass* m_Model;
	ColorShaderClass* m_ColorShader;
	ShaderSpecularClass* m_specularShader;
	ShaderPBRClass* m_pbrShader;
	SkyboxShaderClass* m_skyboxShader;
	TextEngine* m_textEngine;
	UITexturePreview* m_texturePreviewRoughness;
	UITexturePreview* m_texturePreviewMetalness;
	UITexturePreview* m_texturePreviewNormal;
	UITexturePreview* m_texturePreviewAlbedo;

	UIBackground* m_debugBackground;
	MouseClassContainer* m_mouse;
	UISlider* m_roughnessSlider;
	UISlider* m_metalnessSlider;

	//BILINEAR SCREEN BLUR
	RenderTextureClass* m_renderTexture;
	RenderTextureClass* m_renderTextureDownsampled;
	RenderTextureClass* m_renderTextureUpscaled;
	RenderTextureClass* m_renderTextureHorizontalBlur;
	RenderTextureClass* m_renderTextureVerticalBlur;
	UITexture* m_renderTexturePreview;
	BlurShaderClass* m_blurShaderVertical;
	BlurShaderClass* m_blurShaderHorizontal;

	RenderTextureClass* m_skyboxDownsampled;

	float m_rotationY = 0.0f;
	int m_screenWidth = 0;
	int m_screenHeight = 0;
	int m_debug = 0;
};

#endif