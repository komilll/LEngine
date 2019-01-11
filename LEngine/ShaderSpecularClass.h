#pragma once
#ifndef _SHADERSPECULARCLASS_H_
#define _SHADERSPECULARCLASS_H_

#include "BaseShaderClass.h"

class ShaderSpecularClass : public BaseShaderClass
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
	ID3D11Resource* m_diffuseTexture;
	ID3D11ShaderResourceView* m_diffuseTextureView;
	XMFLOAT3 m_lightDirection;
	XMFLOAT3 m_cameraPosition;

protected:
	virtual bool CreateInputLayout(ID3D11Device* device, ID3D10Blob* vertexShaderBuffer) override;
	virtual bool CreateBuffers(ID3D11Device * device) override;
	virtual bool CreateSamplerState(ID3D11Device* device) override;
	virtual void ShutdownShader() override;
	virtual bool SetShaderParameters(ID3D11DeviceContext*, XMMATRIX&, XMMATRIX&, XMMATRIX&) override;
	virtual void RenderShader(ID3D11DeviceContext*, int) override;

private:
	ID3D11Buffer* m_matrixBuffer;
	ID3D11Buffer* m_lightingBuffer;
	ID3D11Buffer* m_cameraBuffer;
};

#endif // !_SHADERSPECULARCLASS_H_