#pragma once
#ifndef _GBUFFERSHADER_H_
#define _GBUFFERSHADER_H_

#include "BaseShaderClass.h"
#include <array>

class GBufferShader : public BaseShaderClass 
{
public:
	enum class BufferType {
		POSITION, NORMAL, ALBEDO, SSAO_NOISE, SSAO
	};

private:
	struct PositionBuffer
	{
		XMFLOAT3 position{};
		float padding{ 0.0f };
	};

	struct NormalBuffer
	{
		XMFLOAT3 normal{};
		float padding{ 0.0f };
	};

	struct SSAONoiseBuffer
	{
		XMFLOAT2 noise{};
		XMFLOAT2 padding{ 0.0f, 0.0f };
	};

	struct SSAOBuffer
	{
		float radiusSize{ 0 };
		float bias{ 0 };
		float padding_2{ 0 };
		float padding_3{ 0 };
		std::array<XMFLOAT3, 64> kernelValues;
	};

public:
	ID3D11Resource* m_diffuseColorTexture;
	ID3D11ShaderResourceView* m_diffuseColorTextureView;

	void ChangeTextureType(BufferType newType);
	void SetKernelValues(std::array<XMFLOAT3, 64> kernelVal);
	void SetNoiseValues(std::array<XMFLOAT2, 16> noiseVal);
	void SetRadiusSize(float radiusSize);
	float* GetRadiusSizeRef();

	void LoadPositionTexture(ID3D11ShaderResourceView* view);
	void LoadNormalTexture(ID3D11ShaderResourceView* view);
	void LoadNoiseTexture(ID3D11ShaderResourceView* view);

	bool Initialize(ID3D11Device* device, HWND hwnd, WCHAR* vsFilename, WCHAR* psFilename, vertexInputType vertexInput, BufferType type);
	virtual bool CreateBufferAdditionals(ID3D11Device *&device) override;
	virtual bool SetShaderParameters(ID3D11DeviceContext *deviceContext, XMMATRIX &worldMatrix, XMMATRIX &viewMatrix, XMMATRIX &projectionMatrix) override;

public:
	float m_radiusSize{ 1.0f };
	float m_bias{ 0.03f };

private:
	BufferType m_bufferType;
	std::array<XMFLOAT3, 64> m_kernelValues;
	std::array<XMFLOAT2, 16> m_noiseValues;

	ID3D11ShaderResourceView* m_positionView{ nullptr };
	ID3D11ShaderResourceView* m_normalView{ nullptr };
	ID3D11ShaderResourceView* m_noiseView{ nullptr };

	ID3D11Buffer* m_positionBuffer{ nullptr };
	ID3D11Buffer* m_normalBuffer{ nullptr };
	ID3D11Buffer* m_ssaoNoiseBuffer{ nullptr };
	ID3D11Buffer* m_ssaoBuffer{ nullptr };
};

#endif // !_GBUFFERSHADER_H_