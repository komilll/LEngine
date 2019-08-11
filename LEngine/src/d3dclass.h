#ifndef _D3DCLASS_H_
#define _D3DCLASS_H_

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")

#include <dxgi.h>
#include <d3dcommon.h>
#include <d3d11.h>
#include <DirectXMath.h>
#include <exception>
#include "BaseShaderClass.h"
#include <array>
using namespace DirectX;

class D3DClass
{
private:
	struct WindowSize
	{
		float x{ 0 };
		float y{ 0 };

		WindowSize(float X, float Y) : x(X), y(Y) { }
	};

public:
	bool Initialize(int screenWidth, int screenHeight, bool vsync, HWND hwnd, bool fullscreen, float screenDepth, float screenNear);
	void ReinitializeForMSAA();
	void Shutdown();
	
	void BeginScene(float red, float green, float blue, float alpha);
	void EndScene();

	ID3D11Device* GetDevice() const;
	ID3D11DeviceContext* GetDeviceContext() const;
	HWND* GetHWND() const;
	ID3D11DepthStencilView* GetDepthStencilView(int count = -1) const;

	//Get MVP matrices
	void GetProjectionMatrix(XMMATRIX&) const;
	void GetWorldMatrix(XMMATRIX&) const;
	void GetOrthoMatrix(XMMATRIX&) const;

	void GetVideoCardInfo(char* cardName, int& memory) const;

	//Enable/Disable alpha blending for translucent 
	void EnableAlphaBlending() const;
	void DisableAlphaBlending() const;

	void ChangeRasterizerCulling(D3D11_CULL_MODE cullMode);
	void ChangeDepthStencilComparison(D3D11_COMPARISON_FUNC comparisionFunc);

	void SetBackBufferRenderTarget() const;

	void TurnZBufferOn() const;
	void TurnZBufferOff() const;

	void DisableDepthTesting() const;
	void EnableDepthTesting() const;

	void ResetViewport() const;
	BaseShaderClass::vertexInputType GetBaseInputType() const;

	WindowSize GetWindowSize() const;
	IDXGISwapChain* GetSwapChain();

	bool CreateDepthBuffer(int sizeMultiplier = 1, int count = 1);
	ID3D11DepthStencilView* CreateDepthBufferReturn(int sizeMultiplier = 1, int count = 1);

public:
	int MSAA_NUMBER_OF_SAMPLES{ 1 }; //[1/2/4/8] on GTX750Ti StormX Dual

private:
	bool m_vsync_enabled;
	int m_videoCardMemory;
	char m_videoCardDescription[128];
	IDXGISwapChain* m_swapChain;
	ID3D11Device* m_device;
	ID3D11DeviceContext* m_deviceContext;
	ID3D11RenderTargetView* m_renderTargetView;
	ID3D11Texture2D* m_depthStencilBuffer;
	ID3D11DepthStencilView* m_depthStencilView;
	ID3D11DepthStencilView* m_depthStencilViewBackBuffer;
	ID3D11DepthStencilView* m_depthStencilView_1;
	ID3D11DepthStencilView* m_depthStencilView_2;
	ID3D11DepthStencilView* m_depthStencilView_4;
	ID3D11DepthStencilView* m_depthStencilView_8;
	ID3D11BlendState* m_enableAlphaBlending;
	ID3D11BlendState* m_disableAlphaBlending;

	ID3D11RasterizerState* m_rasterState;
	ID3D11RasterizerState* m_rasterStateSkybox;
	ID3D11RasterizerState* m_rasterStateNoCulling;

	ID3D11DepthStencilState* m_depthStencilState;
	ID3D11DepthStencilState* m_depthStencilStateSkybox;

	ID3D11DepthStencilState* m_depthStencilStateZBufferOff;

	XMMATRIX m_projectionMatrix;
	XMMATRIX m_worldMatrix;
	XMMATRIX m_orthoMatrix;

	D3D11_VIEWPORT m_viewport;

	HWND* m_hwnd;

	int m_windowSizeX;
	int m_windowSizeY;
};

#endif