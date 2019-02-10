#pragma once
#ifndef _RENDERTEXTURECLASS_H_
#define _RENDERTEXTURECLASS_H_

#include <d3d11.h>
#include <DirectXMath.h>
#include <DDSTextureLoader.h>
using namespace DirectX;

class RenderTextureClass
{
public:
	RenderTextureClass();

	bool Initialize(ID3D11Device* device, int textureWidth, int textureHeight);
	void Shutdown();

	void SetRenderTarget(ID3D11DeviceContext* deviceContext, ID3D11DepthStencilView* depthStencilView);
	void ClearRenderTarget(ID3D11DeviceContext* deviceContext, ID3D11DepthStencilView* depthStencilView, float red, float green, float blue, float alpha);
	bool LoadTexture(ID3D11Device * device, const wchar_t* filename, ID3D11Resource *&m_texture, ID3D11ShaderResourceView *&m_textureView);
	ID3D11ShaderResourceView*& GetShaderResourceView();
	ID3D11Resource*& GetShaderResource();
	void GetOrthoMatrix(XMMATRIX &orthoMatrix);

private:
	XMMATRIX m_orthoMatrix;
	D3D11_VIEWPORT m_viewport;

	ID3D11Texture2D *m_texture2D;
	ID3D11RenderTargetView* m_renderTargetView;
	ID3D11ShaderResourceView* m_shaderResourceView;
};
#endif // !_RENDERTEXTURECLASS_H_