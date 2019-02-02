#include "UITexturePreview.h"

bool UITexturePreview::Initialize(D3DClass * d3d, float centerX, float centerY, float size, wchar_t* textureFilename)
{
	if (!BaseShaderClass::Initialize(d3d->GetDevice(), *d3d->GetHWND(), L"uiTexturePreview.vs", L"uiTexturePreview.ps", BaseShaderClass::vertexInputType(GetInputNames(), GetInputFormats())))
		return false;

	if (textureFilename != L"")
		BaseShaderClass::LoadTexture(d3d->GetDevice(), textureFilename, m_texture, m_textureView);

	return InitializeSquare(d3d->GetDevice(), centerX, centerY, size, false, true);
}

bool UITexturePreview::SetShaderParameters(ID3D11DeviceContext * deviceContext, XMMATRIX & worldMatrix, XMMATRIX & viewMatrix, XMMATRIX & projectionMatrix)
{
	deviceContext->PSSetShaderResources(0, 1, &m_textureView);

	return UIBase::SetShaderParameters(deviceContext, worldMatrix, viewMatrix, projectionMatrix);
}
