#include "UIBase.h"

UIBase::UIBase()
{
}

bool UIBase::InitializeModel(ID3D11Device * device, ModelClass::ShapeSize shape, float left, float right, float top, float bottom)
{
	m_model = new ModelClass;
	if (!m_model->Initialize(device, shape, left, right, top, bottom))
		return false;

	return true;
}

bool UIBase::Render(ID3D11DeviceContext * deviceContext, int indexCount, XMMATRIX & worldMatrix, XMMATRIX & viewMatrix, XMMATRIX & projectionMatrix)
{
	/*if (m_model == nullptr)
		return false;
	m_model->Render(deviceContext);*/

	if (!SetShaderParameters(deviceContext, worldMatrix, viewMatrix, projectionMatrix))
		return false;

	RenderShader(deviceContext, indexCount);
}

bool UIBase::CreateBufferAdditionals(ID3D11Device *& device)
{
	return BaseShaderClass::CreateBufferAdditionals(device);
}

bool UIBase::CreateSamplerState(ID3D11Device * device)
{
	return BaseShaderClass::CreateSamplerState(device);
}

bool UIBase::SetShaderParameters(ID3D11DeviceContext * deviceContext, XMMATRIX & worldMatrix, XMMATRIX & viewMatrix, XMMATRIX & projectionMatrix)
{
	return BaseShaderClass::SetShaderParameters(deviceContext, worldMatrix, viewMatrix, projectionMatrix);
}
