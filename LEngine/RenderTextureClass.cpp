#include "RenderTextureClass.h"

RenderTextureClass::RenderTextureClass()
{
	m_texture2D = 0;
	m_renderTargetView = 0;
	m_shaderResourceView = 0;
}

bool RenderTextureClass::Initialize(ID3D11Device* device, int textureWidth, int textureHeight)
{
	D3D11_TEXTURE2D_DESC textureDesc;
	HRESULT result;
	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;

	ZeroMemory(&textureDesc, sizeof(textureDesc));

	textureDesc.Width = textureWidth;
	textureDesc.Height = textureHeight;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	result = device->CreateTexture2D(&textureDesc, NULL, &m_texture2D);
	if (FAILED(result))
		return false;

	renderTargetViewDesc.Format = textureDesc.Format;
	renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	renderTargetViewDesc.Texture2D.MipSlice = 0;

	result = device->CreateRenderTargetView(m_texture2D, &renderTargetViewDesc, &m_renderTargetView);
	if (FAILED(result))
		return false;

	shaderResourceViewDesc.Format = textureDesc.Format;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;

	result = device->CreateShaderResourceView(m_texture2D, &shaderResourceViewDesc, &m_shaderResourceView);
	if (FAILED(result))
		return false;

	m_orthoMatrix = XMMatrixOrthographicLH((float)textureWidth, (float)textureHeight, 0.0f, 1.0f);

	m_viewport.Width = (float)textureWidth;
	m_viewport.Height = (float)textureHeight;
	m_viewport.MinDepth = 0.0f;
	m_viewport.MaxDepth = 1.0f;
	m_viewport.TopLeftX = 0;//width * 0.5f - textureWidth * 0.5f;
	m_viewport.TopLeftY = 0;

	return true;
}

void RenderTextureClass::Shutdown()
{
}

void RenderTextureClass::SetRenderTarget(ID3D11DeviceContext * deviceContext, ID3D11DepthStencilView * depthStencilView)
{
	deviceContext->OMSetRenderTargets(1, &m_renderTargetView, depthStencilView);

	deviceContext->RSSetViewports(1, &m_viewport);
}

void RenderTextureClass::ClearRenderTarget(ID3D11DeviceContext * deviceContext, ID3D11DepthStencilView * depthStencilView, float red, float green, float blue, float alpha)
{
	float color[4]{ red, green, blue, alpha };

	deviceContext->ClearRenderTargetView(m_renderTargetView, color);
	deviceContext->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
}

bool RenderTextureClass::LoadTexture(ID3D11Device * device, const wchar_t * filename, ID3D11Resource *& m_texture, ID3D11ShaderResourceView *& m_textureView)
{
	if (m_texture != nullptr)
		m_texture->Release();

	if (m_textureView != nullptr)
		m_textureView->Release();


	HRESULT result = CreateDDSTextureFromFile(device, filename, &m_texture, &m_textureView);
	if (FAILED(result))
	{
		if (m_texture != nullptr)
			m_texture->Release();
		if (m_textureView != nullptr)
			m_textureView->Release();

		m_texture = nullptr;
		m_textureView = nullptr;

		return false;
	}

	return true;
}

ID3D11ShaderResourceView *& RenderTextureClass::GetShaderResourceView()
{
	return m_shaderResourceView;
}

ID3D11Resource *& RenderTextureClass::GetShaderResource()
{
	return (ID3D11Resource*&)m_texture2D;
}

void RenderTextureClass::GetOrthoMatrix(XMMATRIX & orthoMatrix)
{
	orthoMatrix = m_orthoMatrix;
}