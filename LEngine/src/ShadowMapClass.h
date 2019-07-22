#pragma once
#ifndef _SHADOWMAPCLASS_H_
#define _SHADOWMAPCLASS_H_

#include "BaseShaderClass.h"

//TODO Seem to be unused - investigate
class ShadowMapClass : public BaseShaderClass
{
private:
	struct LightBuffer
	{
		XMFLOAT3 lightPosition;
		float padding;
	};

	struct LightMatrixBuffer
	{
		XMMATRIX viewMatrix;
		XMMATRIX projectionMatrix;
	};

public:
	ID3D11Resource* m_shadowMapResource;
	ID3D11ShaderResourceView* m_shadowMapResourceView;

	void SetLightPosition(XMFLOAT3 lightPosition);
	void SetLightViewProjection(XMMATRIX& viewMatrix, XMMATRIX& projectionMatrix);
	virtual bool CreateBufferAdditionals(ID3D11Device *&device) override;
	virtual bool SetShaderParameters(ID3D11DeviceContext *deviceContext, XMMATRIX &worldMatrix, XMMATRIX &viewMatrix, XMMATRIX &projectionMatrix) override;

private:
	XMFLOAT3 m_lightPosition;
	XMMATRIX m_viewMatrix, m_projectionMatrix;

	ID3D11Buffer* m_lightBuffer;
	ID3D11Buffer* m_lightMatrixBuffer;
};

#endif // !_SHADOWMAPCLASS_H_