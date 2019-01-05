#pragma once
#ifndef _RENDERER_H_
#define _RENDERER_H_

#include <Windows.h>

#include "renderSettings.h"
#include "modelClass.h"
#include "shaderController.h"
#include "baseCamera.h"

class Renderer
{
public:
	Renderer();

	bool Renderer::Initialize(HWND hwnd);
	void RenderFrame();
	void Shutdown();

private:
	HWND m_hwnd;
	RenderSettings* m_renderSettings;
	BaseCamera *m_camera;
	ModelClass *m_model;
	ShaderController *m_shader;
};


#endif // !_RENDERER_H_