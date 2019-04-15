#pragma once
#ifndef _GBUFFERSHADER_H_
#define _GBUFFERSHADER_H_

#include "BaseShaderClass.h"

class GBufferShader : public BaseShaderClass 
{
public:
	enum BufferType {
		POSITION, NORMAL, ALBEDO, SSAO_NOISE, SSAO
	};

private:
	struct PositionBuffer
	{
		XMFLOAT3 position;
		float padding;
	};

	struct NormalBuffer
	{
		XMFLOAT3 normal;
		float padding;
	};

	struct SSAONoiseBuffer
	{
		XMFLOAT2 noise;
		XMFLOAT2 padding;
	};

	struct SSAOBuffer
	{
		float radiusSize;
		float bias;
		float padding_2;
		float padding_3;
		XMFLOAT3 kernelValues[64];
	};

public:
	ID3D11Resource* m_diffuseColorTexture;
	ID3D11ShaderResourceView* m_diffuseColorTextureView;

	void ChangeTextureType(BufferType newType);
	void SetKernelValues(XMFLOAT3 kernelVal[64]);
	void SetNoiseValues(XMFLOAT2 noiseVal[16]);
	void SetRadiusSize(float radiusSize);
	float* GetRadiusSizeRef();

	void LoadPositionTexture(ID3D11ShaderResourceView* view);
	void LoadNormalTexture(ID3D11ShaderResourceView* view);
	void LoadNoiseTexture(ID3D11ShaderResourceView* view);

	bool Initialize(ID3D11Device* device, HWND hwnd, WCHAR* vsFilename, WCHAR* psFilename, vertexInputType vertexInput, BufferType type);
	virtual bool CreateBufferAdditionals(ID3D11Device *&device) override;
	virtual bool SetShaderParameters(ID3D11DeviceContext *deviceContext, XMMATRIX &worldMatrix, XMMATRIX &viewMatrix, XMMATRIX &projectionMatrix) override;

public:
	float m_radiusSize;
	float m_bias;

private:
	BufferType m_bufferType;
	XMFLOAT3 m_kernelValues[64];
	XMFLOAT2 m_noiseValues[16];

	ID3D11ShaderResourceView* m_positionView;
	ID3D11ShaderResourceView* m_normalView;
	ID3D11ShaderResourceView* m_noiseView;

	ID3D11Buffer* m_positionBuffer;
	ID3D11Buffer* m_normalBuffer;
	ID3D11Buffer* m_ssaoNoiseBuffer;
	ID3D11Buffer* m_ssaoBuffer = nullptr;
};

#endif // !_GBUFFERSHADER_H_