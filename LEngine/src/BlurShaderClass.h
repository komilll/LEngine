#pragma once
#ifndef _BLURSHADERCLASS_H_
#define _BLURSHADERCLASS_H_

#include "BaseShaderClass.h"

class BlurShaderClass : public BaseShaderClass
{
	static const int NUMBER_OF_WEIGHTS = 5;

public:
	struct ScreenSizeBuffer
	{
		float size;
		XMFLOAT3 padding;
	};

	struct BlurWeightsBuffer
	{
		XMFLOAT4 weights;
		XMFLOAT4 lastWeightAndpadding;
	};

public:
	BlurShaderClass();

	void SetTextureSize(float size);
	void SetTextureResourceView(ID3D11ShaderResourceView* shaderResource);
	void SetWeights(float weights[NUMBER_OF_WEIGHTS]);

protected:
	virtual bool CreateBufferAdditionals(ID3D11Device *&device) override;
	virtual bool SetShaderParameters(ID3D11DeviceContext*, XMMATRIX&, XMMATRIX&, XMMATRIX&) override;
	virtual bool CreateSamplerState(ID3D11Device* device) override;

private:
	ID3D11Buffer* m_screenSizeBuffer;
	ID3D11Buffer* m_weightsBuffer;
	ID3D11ShaderResourceView* m_shaderResource;

	float m_size = 0;
	float m_weights[NUMBER_OF_WEIGHTS];
};

#endif // !_BLURSHADERCLASS_H_