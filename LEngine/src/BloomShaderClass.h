#pragma once
#ifndef _BLOOMSHADERCLASS_H_
#define _BLOOMSHADERCLASS_H_

#include "BaseShaderClass.h"

class BloomShaderClass : public BaseShaderClass
{
private:
	struct BloomIntensity
	{
		XMFLOAT3 intensity{ 0.0f, 0.0f, 0.0f };
		float padding{ 0.0f };
	};

public:
	virtual bool CreateBufferAdditionals(ID3D11Device *&device) override;
	virtual bool SetShaderParameters(ID3D11DeviceContext *deviceContext, XMMATRIX &worldMatrix, XMMATRIX &viewMatrix, XMMATRIX &projectionMatrix) override;
	void SetBloomIntensity(const XMFLOAT3 bloomIntensity);

public:
	ID3D11Resource* m_bloomTexture{ nullptr };
	ID3D11ShaderResourceView* m_bloomTextureView{ nullptr };

	ID3D11Buffer* m_bloomIntensityBuffer{ nullptr };

	XMFLOAT3 m_bloomIntensity{ 0.0f, 0.0f, 0.0f };
};

#endif // !_BLOOMSHADERCLASS_H_