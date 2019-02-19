#pragma once
#ifndef _SINGLECOLORCLASS_H_
#define _SINGLECOLORCLASS_H_

#include "BaseShaderClass.h"

class SingleColorClass : public BaseShaderClass
{
private:
	struct LightBuffer
	{
		XMMATRIX lightViewMatrix;
		XMMATRIX lightProjectionMatrix;
		XMFLOAT3 lightPosition;
		float padding;
	};

public:
	ID3D11Resource* m_shadowMapResource;
	ID3D11ShaderResourceView* m_shadowMapResourceView;

	void SetLightPosition(XMFLOAT3 lightPosition);
	void SetLightViewProjection(XMMATRIX& viewMatrix, XMMATRIX& projectionMatrix);
	virtual bool CreateBufferAdditionals(ID3D11Device *&device) override;
	virtual bool SetShaderParameters(ID3D11DeviceContext*, XMMATRIX&, XMMATRIX&, XMMATRIX&) override;
	virtual bool CreateSamplerState(ID3D11Device* device) override;

private:
	XMFLOAT3 m_lightPosition;
	XMMATRIX m_lightViewMatrix, m_lightProjectionMatrix;

	ID3D11Buffer* m_lightBuffer;
};

#endif // !_SINGLECOLORCLASS_H_