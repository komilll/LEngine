////////////////////////////////////////////////////////////////////////////////
// Filename: colorshaderclass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _COLORSHADERCLASS_H_
#define _COLORSHADERCLASS_H_

#pragma comment(lib, "d3dcompiler.lib")

//////////////
// INCLUDES //
//////////////
#include <d3d11.h>
#include <DDSTextureLoader.h>
#include <DirectXMath.h>
#include <d3dcompiler.h>
#include <fstream>
#include <vector>
using namespace std;
using namespace DirectX;

////////////////////////////////////////////////////////////////////////////////
// Class name: ColorShaderClass
////////////////////////////////////////////////////////////////////////////////
class ColorShaderClass
{
private:
	struct MatrixBufferType
	{
		XMMATRIX world;
		XMMATRIX view;
		XMMATRIX projection;
	};

	struct LightingBufferType 
	{
		XMFLOAT3 direction;
		float padding;
	};

	struct CameraBufferType
	{
		XMFLOAT3 cameraDirection;
		float padding;
	};

public:
	ColorShaderClass();
	ColorShaderClass(const ColorShaderClass&);
	~ColorShaderClass();

	bool Initialize(ID3D11Device*, HWND);
	void Shutdown();
	bool Render(ID3D11DeviceContext*, int, XMMATRIX&, XMMATRIX&, XMMATRIX&);
	bool LoadTexture(ID3D11Device * device, const wchar_t * filename, ID3D11Resource*& resource, ID3D11ShaderResourceView*& resourceView);
	void SetLightDirection(float x, float y, float z);
	void SetCameraPosition(XMFLOAT3 &&position);

private:
	bool InitializeShader(ID3D11Device*, HWND, CHAR*, CHAR*);
	void ShutdownShader();
	void OutputShaderErrorMessage(ID3D10Blob*, HWND, CHAR*);

	bool SetShaderParameters(ID3D11DeviceContext*, XMMATRIX&, XMMATRIX&, XMMATRIX&);
	void RenderShader(ID3D11DeviceContext*, int);

public:
	ID3D11Resource* m_texture;
	ID3D11ShaderResourceView* m_textureView;

private:
	ID3D11VertexShader* m_vertexShader;
	ID3D11PixelShader* m_pixelShader;
	ID3D11InputLayout* m_layout;
	ID3D11Buffer* m_matrixBuffer;
	ID3D11Buffer* m_lightingBuffer;
	ID3D11Buffer* m_cameraBuffer;
	ID3D11SamplerState* m_samplerState;
	XMFLOAT3 m_lightDirection;
	XMFLOAT3 m_cameraPosition;
};

#endif