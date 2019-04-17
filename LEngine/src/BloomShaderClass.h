#pragma once
#ifndef _BLOOMSHADERCLASS_H_
#define _BLOOMSHADERCLASS_H_

#include "BaseShaderClass.h"

class BloomShaderClass : public BaseShaderClass
{

public:
	virtual bool SetShaderParameters(ID3D11DeviceContext *deviceContext, XMMATRIX &worldMatrix, XMMATRIX &viewMatrix, XMMATRIX &projectionMatrix) override;

public:
	ID3D11Resource* m_bloomTexture;
	ID3D11ShaderResourceView* m_bloomTextureView;

	float m_bloomIntensity = 1.0f;
};

#endif // !_BLOOMSHADERCLASS_H_