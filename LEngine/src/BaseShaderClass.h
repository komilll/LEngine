#pragma once
#ifndef _BASESHADERCLASS_H_
#define _BASESHADERCLASS_H_

#pragma comment(lib, "d3dcompiler.lib")

//////////////
// INCLUDES //
//////////////
#include <d3d11.h>
#include <DDSTextureLoader.h>
#include <WICTextureLoader.h>
#include <DirectXMath.h>
#include <d3dcompiler.h>
#include <fstream>
#include <vector>
#include <map>
#include <string>
#include <array>
using namespace std;
using namespace DirectX;

class BaseShaderClass
{
//////// STRUCTS ////////
public:
	using vertexInputMap = std::map<LPCSTR, int>;
	struct vertexInputType
	{
	public:
		std::vector<LPCSTR> &name;
		std::vector<DXGI_FORMAT> &format;

		vertexInputType() : name(std::vector<LPCSTR>{}), format(std::vector<DXGI_FORMAT>{})
		{
		}
		vertexInputType(std::vector<LPCSTR> &n, std::vector<DXGI_FORMAT> &f) : name(n), format(f) 
		{ 
		}
		vertexInputType vertexInputType::operator=(const vertexInputType &ref)
		{
			return vertexInputType(ref.name, ref.format);
		}

		void SaveData(std::vector<LPCSTR> &n, std::vector<DXGI_FORMAT> &f)
		{
			name = n;
			format = f;
		}
	};

protected:
	class SamplerType
	{
	public:
		bool isVectorSampler{ false };
		UINT startSlot{ 0 };
		UINT numSamplers{ 0 };
		ID3D11SamplerState* samplerStates{ nullptr };

		SamplerType(bool isVectorSampler_, UINT startSlot_, UINT numSamplers_, ID3D11SamplerState* &samplerStates_) {
			isVectorSampler = isVectorSampler_;
			startSlot = startSlot_;
			numSamplers = numSamplers_;
			samplerStates = samplerStates_;
		}
	};

private:
	struct MatrixBufferType
	{
		XMMATRIX world;
		XMMATRIX view;
		XMMATRIX projection;
	};

//////// METHODS ////////
public:
	bool Initialize(ID3D11Device* device, HWND hwnd, WCHAR* vsFilename, WCHAR* psFilename, vertexInputType vertexInput);
	void Shutdown();
	bool Render(ID3D11DeviceContext*, int, XMMATRIX&, XMMATRIX&, XMMATRIX&);
	static bool LoadTexture(ID3D11Device * device, const wchar_t* filename, ID3D11Resource *&m_texture, ID3D11ShaderResourceView *&m_textureView, bool isDDS = true);
	static bool LoadTexture(ID3D11Device* device, ID3D11Resource *& inTexture, ID3D11Resource *& outTexture, ID3D11ShaderResourceView *& outTextureView);
	void AddSampler(bool isVectorSampler, UINT startSlot, UINT numSapmlers, ID3D11SamplerState* &samplerStates);

protected:
	virtual bool CreateBufferAdditionals(ID3D11Device *&device);
	virtual bool CreateSamplerState(ID3D11Device* device);
	virtual bool SetShaderParameters(ID3D11DeviceContext*, XMMATRIX&, XMMATRIX&, XMMATRIX&);
	void RenderShader(ID3D11DeviceContext*, int);

private:
	bool CreateInputLayout(ID3D11Device* device, ID3D10Blob* vertexShaderBuffer, vertexInputType vertexInput);
	bool CreateBuffers(ID3D11Device * device);
	bool InitializeShader(ID3D11Device* device, HWND hwnd, WCHAR* vsFilename, WCHAR* psFilename, vertexInputType vertexInput);
	void OutputShaderErrorMessage(ID3D10Blob*, HWND, CHAR*) const;	
	void ShutdownShader();

//////// VARIABLES ////////
protected:
	ID3D11VertexShader* m_vertexShader{ nullptr };
	ID3D11PixelShader* m_pixelShader{ nullptr };
	ID3D11InputLayout* m_layout{ nullptr };
	ID3D11SamplerState* m_samplerState{ nullptr };
	std::vector<SamplerType*> m_samplers;
	std::vector<ID3D11Buffer*> m_buffers;

private:
	ID3D11Buffer* m_matrixBuffer{ nullptr };
};
#endif