#include "UITextureMoveable.h"

bool UITextureMoveable::SetShaderParameters(ID3D11DeviceContext * deviceContext, XMMATRIX & worldMatrix, XMMATRIX & viewMatrix, XMMATRIX & projectionMatrix)
{
	if (UIBase::SetShaderParameters(deviceContext, worldMatrix, viewMatrix, projectionMatrix) == false)
		return false;

	deviceContext->PSSetShaderResources(0, 1, &m_textureView);
	return true;
}

void UITextureMoveable::LoadTexture(ID3D11Device* device, wchar_t * name, bool isDDS)
{
	BaseShaderClass::LoadTexture(device, name, m_texture, m_textureView, isDDS);
}

void UITextureMoveable::LoadTexture(ID3D11Device * device, ID3D11ShaderResourceView * resourceView)
{
	m_textureView = resourceView;
}
