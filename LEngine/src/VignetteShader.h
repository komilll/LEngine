#pragma once
#ifndef _VIGNETTESHADER_H_
#define _VIGNETTESHADER_H_

#include "BaseShaderClass.h"

class VignetteShader : public BaseShaderClass
{
protected:
	virtual bool SetShaderParameters(ID3D11DeviceContext *deviceContext, XMMATRIX& worldMatrix, XMMATRIX& viewMatrix, XMMATRIX& projectionMatrix) override;

public:
	ID3D11Resource* m_vignetteResource;
	ID3D11ShaderResourceView* m_vignetteResourceView;
};

#endif // !_POSTPROCESSSHADER_H_