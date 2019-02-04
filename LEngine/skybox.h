#pragma once

#pragma comment(lib, "d3dcompiler.lib")

//////////////
// INCLUDES //
//////////////
#include <d3d11.h>
#include <DDSTextureLoader.h>
#include <DirectXMath.h>
#include <d3dcompiler.h>
#include <fstream>
using namespace std;
using namespace DirectX;

class Skybox
{
private:
	struct MatrixBufferType
	{
		XMMATRIX world;
		XMMATRIX view;
		XMMATRIX projection;
	};

public:
	Skybox();
	Skybox(const Skybox&);
	~Skybox();

	bool Initialize(ID3D11Device* device, HWND hwnd);
	void Shutdown();
	bool Render(ID3D11DeviceContext *deviceContext, int indexCount, XMMATRIX &worldMatrix, XMMATRIX &viewMatrix, XMMATRIX &projectionMatrix);

private:
	bool InitializeShader(ID3D11Device *device, HWND hwnd, LPCWSTR vsFilename, LPCWSTR psFilename);
	void ShutdownShader();
	void OutputShaderErrorMessage(ID3D10Blob *message, HWND hwnd, CHAR *shaderFilename);

	bool SetShaderParameters(ID3D11DeviceContext *deviceContext, XMMATRIX &worldMatrix, XMMATRIX &viewMatrix, XMMATRIX& projectionMatrix);
	void RenderShader(ID3D11DeviceContext *deviceContext, int indexCount);

private:
	ID3D11VertexShader* m_vertexShader;
	ID3D11PixelShader* m_pixelShader;
	ID3D11InputLayout* m_layout;
	ID3D11Buffer* m_matrixBuffer;	
	ID3D11SamplerState* m_sampleState;
	ID3D11ShaderResourceView* m_texture;
	ID3D11Resource* m_textureTmp;
};