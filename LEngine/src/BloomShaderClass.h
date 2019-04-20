#pragma once
#ifndef _BLOOMSHADERCLASS_H_
#define _BLOOMSHADERCLASS_H_

#include "BaseShaderClass.h"

class BloomShaderClass : public BaseShaderClass
{
private:
	struct BloomIntensity
	{
		XMFLOAT3 intensity;
		float padding;
	};

public:
	virtual bool CreateBufferAdditionals(ID3D11Device *&device) override;
	virtual bool SetShaderParameters(ID3D11DeviceContext *deviceContext, XMMATRIX &worldMatrix, XMMATRIX &viewMatrix, XMMATRIX &projectionMatrix) override;
	void SetBloomIntensity(XMFLOAT3 bloomIntensity);

public:
	ID3D11Resource* m_bloomTexture;
	ID3D11ShaderResourceView* m_bloomTextureView;

	ID3D11Buffer* m_bloomIntensityBuffer;

	XMFLOAT3 m_bloomIntensity;
};

#endif // !_BLOOMSHADERCLASS_H_