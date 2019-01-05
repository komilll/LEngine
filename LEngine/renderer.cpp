#include "renderer.h"

Renderer::Renderer()
{
}


bool Renderer::Initialize(HWND hwnd)
{
	m_hwnd = hwnd;

	m_renderSettings = new RenderSettings;
	m_renderSettings->Initialize(800, 600, false, m_hwnd, false, 1000.0f, 0.01f);

	m_camera = new BaseCamera;
	m_camera->SetPosition(0.0f, 0.0f, -10.0f);

	m_model = new ModelClass;
	m_model->Initialize(m_renderSettings->GetDevice(), m_renderSettings->GetDeviceContext());

	m_shader = new ShaderController;
	m_shader->Initialize(m_hwnd, m_renderSettings->GetDevice());
	return true;
}

void Renderer::RenderFrame()
{
	//m_renderSettings->TurnZBufferOff();
	//m_renderSettings->TurnOffAlphaBlending();
	m_renderSettings->ClearScene(0.0f, 0.0f, 0.0f, 1.0f);

	m_camera->SetPosition(m_camera->GetPosition().x, m_camera->GetPosition().y, m_camera->GetPosition().z);
	m_camera->Render();

	XMMATRIX worldMatrix, viewMatrix, projectionMatrix;
	m_camera->GetViewMatrix(viewMatrix);
	m_renderSettings->GetProjectionMatrix(projectionMatrix);
	m_renderSettings->GetWorldMatrix(worldMatrix);
	
	m_model->RenderModel();

	//projectionMatrix = XMMatrixMultiply(projectionMatrix, XMMatrixIdentity() * 0.1f);

	XMVECTOR position = { -1.0f, -1.0f, 0.0f, 1.0f };
	position = XMVector4Transform(position, worldMatrix);
	position = XMVector4Transform(position, viewMatrix);
	position = XMVector4Transform(position, projectionMatrix);

	m_shader->Render(m_renderSettings->GetDeviceContext(), m_model->GetIndexCount(),
		worldMatrix, viewMatrix, projectionMatrix);

	m_renderSettings->PresentScene();
	m_renderSettings->TurnZBufferOn();
}

void Renderer::Shutdown()
{
}
