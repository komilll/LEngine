#pragma once
#ifndef _SHADOWSHADER_H_
#define _SHADOWSHADER_H_

#include "BaseShaderClass.h"

//TODO Seem to be unused - investigate
class ShadowShader : public BaseShaderClass
{
private:
	struct LightBuffer
	{
		XMMATRIX viewMatrix;
		XMMATRIX projectionMatrix;
	};

public:
	ID3D11Resource* m_shadowMapResource;
	ID3D11ShaderResourceView* m_shadowMapResourceView;
	ID3D11Resource* m_frameResource;
	ID3D11ShaderResourceView* m_frameResourceView;

	void SetShadowMapResource(ID3D11ShaderResourceView* resource);
	void SetFrameResource(ID3D11ShaderResourceView* resource);
	void SetLightViewProjection(XMMATRIX& viewMatrix, XMMATRIX& projectionMatrix);
	virtual bool CreateBufferAdditionals(ID3D11Device *&device) override;
	virtual bool SetShaderParameters(ID3D11DeviceContext *deviceContext, XMMATRIX &worldMatrix, XMMATRIX &viewMatrix, XMMATRIX &projectionMatrix) override;

private:
	XMMATRIX m_viewMatrix, m_projectionMatrix;

	ID3D11Buffer* m_lightBuffer;
};

#endif // !_SHADOWMAPCLASS_H_