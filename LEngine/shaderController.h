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
public:
	ShaderController();

	bool Initialize(HWND hwnd, ID3D11Device* device);
	void Render(ID3D11DeviceContext* deviceContext, int indexCount);

private:
	void RenderShader(ID3D11DeviceContext* deviceContext, int indexCount);

private:
	ID3D11VertexShader* m_vertexShader;
	ID3D11PixelShader* m_pixelShader;
	ID3D10Blob* m_vertexShaderBuffer;
	ID3D10Blob* m_pixelShaderBuffer;
	ID3D11InputLayout* m_layout;
	ID3D11Buffer* m_matrixBuffer;
	ID3D11Buffer* m_colorBuffer;
};

#endif // !_SHADER_CONTROLLER_H_