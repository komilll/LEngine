#include "RenderTextureClass.h"
#include "d3dclass.h"

bool RenderTextureClass::InitializeShadowMap(ID3D11Device * device, int textureWidth, int textureHeight)
{
	HRESULT result;

	D3D11_TEXTURE2D_DESC textureDesc;
	ZeroMemory(&textureDesc, sizeof(textureDesc));
	textureDesc.Width = textureWidth;
	textureDesc.Height = textureHeight;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	result = device->CreateTexture2D(&textureDesc, NULL, &m_texture2D);
	if (FAILED(result))
		return false;

	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
	ZeroMemory(&textureDesc, sizeof(textureDesc));
	renderTargetViewDesc.Format = textureDesc.Format;
	renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMS;
	renderTargetViewDesc.Texture2D.MipSlice = 0;

	result = device->CreateRenderTargetView(m_texture2D, &renderTargetViewDesc, &m_renderTargetView);
	if (FAILED(result))
		return false;

	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
	ZeroMemory(&shaderResourceViewDesc, sizeof(shaderResourceViewDesc));
	shaderResourceViewDesc.Format = textureDesc.Format;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DMS;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;

	result = device->CreateShaderResourceView(m_texture2D, &shaderResourceViewDesc, &m_shaderResourceView);
	if (FAILED(result))
		return false;

	m_orthoMatrix = XMMatrixOrthographicLH((float)textureWidth, (float)textureHeight, 0.0f, 1.0f);

	m_viewport.Height = (float)textureHeight;
	m_viewport.Width = (float)textureWidth;
	m_viewport.MinDepth = 0.0f;
	m_viewport.MaxDepth = 1.0f;
	m_viewport.TopLeftX = 0;
	m_viewport.TopLeftY = 0;

	return true;
}

bool RenderTextureClass::Initialize(ID3D11Device* device, int textureWidth, int textureHeight, int msaaCount, RenderTextureClass::Scaling scaling, bool skybox)
{
	if (skybox)
		return InitializeSkybox(device, textureWidth, textureHeight, scaling);
	else
		return Initialize2DTexture(device, textureWidth, textureHeight, msaaCount, scaling);
}

bool RenderTextureClass::InitializeWithMip(ID3D11Device * device, int textureWidth, int textureHeight, int mipLevels)
{
	HRESULT result;

	D3D11_TEXTURE2D_DESC textureDesc;
	ZeroMemory(&textureDesc, sizeof(textureDesc));
	textureDesc.Width = textureWidth;
	textureDesc.Height = textureHeight;
	textureDesc.MipLevels = mipLevels;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

	result = device->CreateTexture2D(&textureDesc, NULL, &m_texture2D);
	if (FAILED(result))
		return false;

	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
	ZeroMemory(&textureDesc, sizeof(textureDesc));
	renderTargetViewDesc.Format = textureDesc.Format;
	renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMSARRAY;
	renderTargetViewDesc.Texture2DArray.MipSlice = 0;
	renderTargetViewDesc.Texture2DArray.ArraySize = 1;

	result = device->CreateRenderTargetView(m_texture2D, &renderTargetViewDesc, &m_renderTargetView);
	if (FAILED(result))
		return false;

	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
	ZeroMemory(&shaderResourceViewDesc, sizeof(shaderResourceViewDesc));
	shaderResourceViewDesc.Format = textureDesc.Format;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DMS;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.MipLevels = mipLevels;

	result = device->CreateShaderResourceView(m_texture2D, &shaderResourceViewDesc, &m_shaderResourceView);
	if (FAILED(result))
		return false;

	m_orthoMatrix = XMMatrixOrthographicLH((float)textureWidth, (float)textureHeight, 0.0f, 1.0f);

	m_viewport.Height = (float)textureHeight;
	m_viewport.MinDepth = 0.0f;
	m_viewport.MaxDepth = 1.0f;
	m_viewport.TopLeftY = 0;
	m_viewport.Width = (float)textureWidth;
	m_viewport.TopLeftX = 0;

	return true;
}

void RenderTextureClass::SetRenderTarget(ID3D11DeviceContext * deviceContext, ID3D11DepthStencilView * depthStencilView, bool withViewport)
{
	deviceContext->OMSetRenderTargets(1, &m_renderTargetView, depthStencilView);
	deviceContext->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

	if (withViewport)
		deviceContext->RSSetViewports(1, &m_viewport);
}

void RenderTextureClass::ClearRenderTarget(ID3D11DeviceContext * deviceContext, ID3D11DepthStencilView * depthStencilView, float red, float green, float blue, float alpha)
{
	const std::array<float, 4> color{ red, green, blue, alpha };

	deviceContext->ClearRenderTargetView(m_renderTargetView, &color[0]);
	deviceContext->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
}

void RenderTextureClass::SetViewport(ID3D11DeviceContext * deviceContext)
{
	deviceContext->RSSetViewports(1, &m_viewport);
}

bool RenderTextureClass::LoadTexture(ID3D11Device * device, const wchar_t * filename, ID3D11Resource *& m_texture, ID3D11ShaderResourceView *& m_textureView, bool isDDS)
{
	if (m_texture)
		m_texture->Release();

	if (m_textureView)
		m_textureView->Release();

	HRESULT result;
	//Import texture based on type - DDS or not
	if (isDDS)
		result = CreateDDSTextureFromFile(device, filename, &m_texture, &m_textureView);
	else
		result = CreateWICTextureFromFile(device, filename, &m_texture, &m_textureView);

	if (FAILED(result))
	{
		if (m_texture)
			m_texture->Release();
		if (m_textureView)
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

ID3D11ShaderResourceView * RenderTextureClass::GetShaderResourceViewCopy() const
{
	return m_shaderResourceView;
}

ID3D11RenderTargetView *& RenderTextureClass::GetShaderTargetView(int skyboxIndex)
{
	return m_renderTargetViewSkybox[skyboxIndex];
}

ID3D11Resource *& RenderTextureClass::GetShaderResource()
{
	return (ID3D11Resource*&)m_texture2D;
}

void RenderTextureClass::GetOrthoMatrix(XMMATRIX & orthoMatrix) const
{
	orthoMatrix = m_orthoMatrix;
}

bool RenderTextureClass::Initialize2DTexture(ID3D11Device *& device, int textureWidth, int textureHeight, int msaaCount, RenderTextureClass::Scaling scaling)
{
	HRESULT result;

	D3D11_TEXTURE2D_DESC textureDesc;
	ZeroMemory(&textureDesc, sizeof(textureDesc));
	textureDesc.Width = textureWidth;
	textureDesc.Height = textureHeight;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	textureDesc.SampleDesc.Count = msaaCount;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	result = device->CreateTexture2D(&textureDesc, NULL, &m_texture2D);
	if (FAILED(result))
		return false;

	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
	ZeroMemory(&textureDesc, sizeof(textureDesc));
	renderTargetViewDesc.Format = textureDesc.Format;
	renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMS;
	renderTargetViewDesc.Texture2D.MipSlice = 0;

	result = device->CreateRenderTargetView(m_texture2D, &renderTargetViewDesc, &m_renderTargetView);
	if (FAILED(result))
		return false;

	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
	ZeroMemory(&shaderResourceViewDesc, sizeof(shaderResourceViewDesc));
	shaderResourceViewDesc.Format = textureDesc.Format;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DMS;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;

	result = device->CreateShaderResourceView(m_texture2D, &shaderResourceViewDesc, &m_shaderResourceView);
	if (FAILED(result))
		return false;

	m_orthoMatrix = XMMatrixOrthographicLH((float)textureWidth, (float)textureHeight, 0.0f, 1.0f);

	m_viewport.Height = (float)textureHeight;
	m_viewport.MinDepth = 0.0f;
	m_viewport.MaxDepth = 1.0f;
	m_viewport.TopLeftY = 0;

	if (scaling == RenderTextureClass::Scaling::DOWNSCALE)
	{
		//m_viewport.Width = (float)textureWidth * 2.0f;
		//m_viewport.TopLeftX = -textureWidth / 2;
		m_viewport.Width = (float)textureWidth;
		m_viewport.TopLeftX = 0;
	}
	else if (scaling == RenderTextureClass::Scaling::UPSCALE)
	{
		//m_viewport.Width = (float)textureWidth * 1.8f;
		//m_viewport.TopLeftX = -textureWidth / 2 + 26;
		m_viewport.Width = (float)textureWidth;
		m_viewport.TopLeftX = 0;
	}
	else if (scaling == RenderTextureClass::Scaling::NONE)
	{
		m_viewport.Width = (float)textureWidth;
		m_viewport.TopLeftX = 0;
	}

	return true;
}

bool RenderTextureClass::InitializeSkybox(ID3D11Device *& device, int textureWidth, int textureHeight, RenderTextureClass::Scaling scaling)
{
	HRESULT result;

	D3D11_TEXTURE2D_DESC textureDesc;
	ZeroMemory(&textureDesc, sizeof(textureDesc));
	textureDesc.Width = textureWidth;
	textureDesc.Height = textureHeight;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 6;
	textureDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE | D3D11_RESOURCE_MISC_GENERATE_MIPS;;

	result = device->CreateTexture2D(&textureDesc, NULL, &m_texture2D);
	if (FAILED(result))
		return false;

	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
	ZeroMemory(&textureDesc, sizeof(textureDesc));
	renderTargetViewDesc.Format = textureDesc.Format;
	renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMSARRAY;
	renderTargetViewDesc.Texture2DArray.MipSlice = 0;
	renderTargetViewDesc.Texture2DArray.ArraySize = 1;

	result = device->CreateRenderTargetView(m_texture2D, &renderTargetViewDesc, &m_renderTargetView);
	if (FAILED(result))
		return false;

	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
	ZeroMemory(&shaderResourceViewDesc, sizeof(shaderResourceViewDesc));
	shaderResourceViewDesc.Format = textureDesc.Format;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	shaderResourceViewDesc.TextureCube.MostDetailedMip = 0;
	shaderResourceViewDesc.TextureCube.MipLevels = 1;
	for (int i = 0; i < 6; ++i)
	{
		shaderResourceViewDesc.Texture2DArray.FirstArraySlice = i;
		result = device->CreateRenderTargetView(m_texture2D, &renderTargetViewDesc, &m_renderTargetViewSkybox[i]);
		if (FAILED(result))
			return false;
	}

	result = device->CreateShaderResourceView(m_texture2D, &shaderResourceViewDesc, &m_shaderResourceView);
	if (FAILED(result))
		return false;

	m_orthoMatrix = XMMatrixOrthographicLH((float)textureWidth, (float)textureHeight, 0.0f, 1.0f);

	m_viewport.Width = (float)textureWidth;
	m_viewport.Height = (float)textureHeight;
	m_viewport.MinDepth = 0.0f;
	m_viewport.MaxDepth = 1.0f;
	m_viewport.TopLeftX = 0;
	m_viewport.TopLeftY = 0;

	return true;
}
