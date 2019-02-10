#include "UITexture.h"

bool UITexture::Initialize(D3DClass * d3d, float centerX, float centerY, float size, wchar_t * textureFilename)
{
	if (!BaseShaderClass::Initialize(d3d->GetDevice(), *d3d->GetHWND(), L"textureShader.vs", L"textureShader.ps", BaseShaderClass::vertexInputType(GetInputNames(), GetInputFormats())))
		return false;

	if (textureFilename != L"")
	{
		if (!LoadTexture(d3d->GetDevice(), textureFilename, m_texture, m_textureView))
			return false;
	}
	
	return InitializeSquare(d3d->GetDevice(), centerX, centerY, size, false, true);
}

bool UITexture::Initialize(D3DClass * d3d, ModelClass::ShapeSize shape, float left, float right, float top, float bottom, bool squareToHeight)
{
	if (squareToHeight)
	{
		left *= 9.0f / 16.0f;
		right *= 9.0f / 16.0f;
	}

	if (!BaseShaderClass::Initialize(d3d->GetDevice(), *d3d->GetHWND(), L"textureShader.vs", L"textureShader.ps", BaseShaderClass::vertexInputType(GetInputNames(), GetInputFormats())))
		return false;

	return InitializeModelGeneric(d3d->GetDevice(), shape, left, right, top, bottom);
}

void UITexture::BindTexture(ID3D11ShaderResourceView *& textureView)
{
	m_textureView = textureView;	
}

bool UITexture::SetShaderParameters(ID3D11DeviceContext * deviceContext, XMMATRIX & worldMatrix, XMMATRIX & viewMatrix, XMMATRIX & projectionMatrix)
{
	deviceContext->PSSetShaderResources(0, 1, &m_textureView);

	return UIBase::SetShaderParameters(deviceContext, worldMatrix, viewMatrix, projectionMatrix);
}

ModelClass * UITexture::GetModel()
{
	return m_model;
}

ID3D11ShaderResourceView *& UITexture::GetShaderResourceView()
{
	return m_textureView;
}

ID3D11Resource *& UITexture::GetShaderResource()
{
	return m_texture;
}
