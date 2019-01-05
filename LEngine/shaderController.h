#pragma once
#ifndef _SHADER_CONTROLLER_H_
#define _SHADER_CONTROLLER_H_

#pragma comment(lib, "d3dcompiler.lib")

#include <d3d11.h>
#include <DirectXMath.h>
#include <d3dcompiler.h>
#include <fstream>
#include <vector>

class ShaderController
{
private:
	struct MatrixBufferType
	{
		DirectX::XMMATRIX world;
		DirectX::XMMATRIX view;
		DirectX::XMMATRIX projection;
	};

public:
	ShaderController();

	bool Initialize(HWND hwnd, ID3D11Device* device);
	void Render(ID3D11DeviceContext* deviceContext, int indexCount, DirectX::XMMATRIX &worldMatrix, DirectX::XMMATRIX &viewMatrix,
		DirectX::XMMATRIX &projectionMatrix);
	bool SetBuffersOnRender(ID3D11DeviceContext* deviceContext, DirectX::XMMATRIX &worldMatrix, DirectX::XMMATRIX &viewMatrix,
		DirectX::XMMATRIX &projectionMatrix);

private:
	void RenderShader(ID3D11DeviceContext* deviceContext, int indexCount);

private:
	ID3D11VertexShader* m_vertexShader;
	ID3D11PixelShader* m_pixelShader;
	ID3D10Blob* m_vertexShaderBuffer;
	ID3D10Blob* m_pixelShaderBuffer;
	ID3D11InputLayout* m_layout;
	ID3D11Buffer* m_matrixBuffer;
};

#endif // !_SHADER_CONTROLLER_H_