#include "renderer.h"

Renderer::Renderer()
{
}


bool Renderer::Initialize(HWND hwnd)
{
	m_hwnd = hwnd;

	RenderFrame();
	return true;
}

void Renderer::RenderFrame()
{
	RenderSettings* renderSettings = new RenderSettings;
	renderSettings->Initialize(800, 600, true, m_hwnd, false, 0.0f, 0.1f);
	renderSettings->ClearScene(1.0f, 1.0f, 1.0f, 1.0f);
	
	ModelClass* model = new ModelClass;
	model->Initialize(renderSettings->GetDevice(), renderSettings->GetDeviceContext());
	model->RenderModel();

	ShaderController *shader = new ShaderController;
	shader->Initialize(m_hwnd, renderSettings->GetDevice());
	shader->Render(renderSettings->GetDeviceContext(), model->GetIndexCount());

	renderSettings->PresentScene();
}

void Renderer::Shutdown()
{
}
