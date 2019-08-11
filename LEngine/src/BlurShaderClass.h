#pragma once
#ifndef _BLURSHADERCLASS_H_
#define _BLURSHADERCLASS_H_

#include "BaseShaderClass.h"
#include <array>

class BlurShaderClass : public BaseShaderClass
{
public:
	static const int k_numberOfWeights{ 5 };

	__declspec(align(16))
	struct ScreenSizeBuffer
	{
		float size;
		float padding_1;
		float padding_2;
		float padding_3;
	};

	struct BlurWeightsBuffer
	{
		XMFLOAT4 weights{};
		XMFLOAT4 lastWeightAndpadding{};
	};

public:
	void SetTextureSize(const float size);
	void SetTextureResourceView(ID3D11ShaderResourceView* const shaderResource);
	void SetWeights(std::array<float, k_numberOfWeights> weights);

protected:
	virtual bool CreateBufferAdditionals(ID3D11Device *&device) override;
	virtual bool SetShaderParameters(ID3D11DeviceContext*, XMMATRIX&, XMMATRIX&, XMMATRIX&) override;
	virtual bool CreateSamplerState(ID3D11Device* device) override;

private:
	ID3D11Buffer* m_screenSizeBuffer;
	ID3D11Buffer* m_weightsBuffer;
	ID3D11ShaderResourceView* m_shaderResource;

	float m_size = 0;
	std::array<float, k_numberOfWeights> m_weights{ 1 };
};

#endif // !_BLURSHADERCLASS_H_