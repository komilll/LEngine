#pragma once
#ifndef _SHADERSPECULARCLASS_H_
#define _SHADERSPECULARCLASS_H_

#include "BaseShaderClass.h"

class ShaderSpecularClass : public BaseShaderClass
{
private:

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
	ID3D11Resource* m_normalTexture;
	ID3D11ShaderResourceView* m_normalTextureView;
	XMFLOAT3 m_lightDirection;
	XMFLOAT3 m_cameraPosition;

protected:
	virtual bool CreateBufferAdditionals(ID3D11Device *&device) override;
	virtual bool SetShaderParameters(ID3D11DeviceContext*, XMMATRIX&, XMMATRIX&, XMMATRIX&) override;

private:
	ID3D11Buffer* m_lightingBuffer;
	ID3D11Buffer* m_cameraBuffer;
};

#endif // !_SHADERSPECULARCLASS_H_