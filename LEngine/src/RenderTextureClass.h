#pragma once
#ifndef _RENDERTEXTURECLASS_H_
#define _RENDERTEXTURECLASS_H_

#include <d3d11.h>
#include <DirectXMath.h>
#include <DDSTextureLoader.h>
#include <WICTextureLoader.h>
#include <string>
#include <array>
using namespace DirectX;
using namespace std;

class RenderTextureClass
{
public:
	enum class Scaling{
		NONE, UPSCALE, DOWNSCALE
	};

public:
	//TODO Add constructor
	bool InitializeShadowMap(ID3D11Device* device, int textureWidth, int textureHeight);
	bool Initialize(ID3D11Device* device, int textureWidth, int textureHeight, int msaaCount, RenderTextureClass::Scaling scaling = RenderTextureClass::Scaling::NONE, bool skybox = false);
	bool InitializeWithMip(ID3D11Device* device, int textureWidth, int textureHeight, int mipLevels);

	///<summary>Choose target to pass render info</summary>
	void SetRenderTarget(ID3D11DeviceContext* deviceContext, ID3D11DepthStencilView* depthStencilView, bool withViewport = true);
	///<summary>Clear target every time before usage!</summary>
	void ClearRenderTarget(ID3D11DeviceContext* deviceContext, ID3D11DepthStencilView* depthStencilView, float red, float green, float blue, float alpha);
	void SetViewport(ID3D11DeviceContext * deviceContext);
	///<summary>Non-DDS textures require different loader - choose if your data is in DDS format</summary>
	bool LoadTexture(ID3D11Device * device, const wchar_t* filename, ID3D11Resource *&m_texture, ID3D11ShaderResourceView *&m_textureView, bool isDDS = true);
	ID3D11ShaderResourceView*& GetShaderResourceView();
	ID3D11ShaderResourceView* GetShaderResourceViewCopy() const;
	ID3D11RenderTargetView*& GetShaderTargetView(int skyboxIndex);
	ID3D11Resource*& GetShaderResource();
	void GetOrthoMatrix(XMMATRIX &orthoMatrix) const;

private:
	bool Initialize2DTexture(ID3D11Device*& device, int textureWidth, int textureHeight, int msaaCount, RenderTextureClass::Scaling scaling);
	bool InitializeSkybox(ID3D11Device*& device, int textureWidth, int textureHeight, RenderTextureClass::Scaling scaling);

private:
	XMMATRIX m_orthoMatrix;
	D3D11_VIEWPORT m_viewport;

	ID3D11Texture2D *m_texture2D;
	ID3D11RenderTargetView* m_renderTargetView;
	std::array<ID3D11RenderTargetView*, 6> m_renderTargetViewSkybox;
	ID3D11ShaderResourceView* m_shaderResourceView;
};
#endif // !_RENDERTEXTURECLASS_H_