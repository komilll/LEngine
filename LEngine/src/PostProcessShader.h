#pragma once
#ifndef _POSTPROCESSSHADER_H_
#define _POSTPROCESSSHADER_H_

#include "BaseShaderClass.h"

class PostProcessShader : public BaseShaderClass
{
private:
	struct TextureBufferType
	{
		float hasSSAO;
		float hasBloom;
		float hasLUT;
		float hasChromaticAberration;
		float hasGrain;
		
		XMFLOAT3 padding;
	};

	struct ChromaticAberrationBuffer
	{
		float red;
		float green;
		float blue;
		float intensity;
	};


	struct GrainSettings
	{
		float size;
		float intensity;
		float hasColor;
		int type;
	};

public:
	void SetScreenBuffer(ID3D11ShaderResourceView *&screenBuffer);
	void SetSSAOBuffer(ID3D11ShaderResourceView *&ssaoBuffer);
	void SetBloomBuffer(ID3D11ShaderResourceView *&bloomBuffer);
	void SetLUTBuffer(ID3D11ShaderResourceView *&lutBuffer);

	void UseChromaticAberration(bool setActive);
	void SetChromaticAberrationOffsets(float red, float green, float blue);
	void SetChromaticAberrationOffsets(XMFLOAT3 offset);
	void SetChromaticAberrationIntensity(float intensity);

	void UseGrain(bool setActive);
	void SetNoise(ID3D11ShaderResourceView *& whiteNoise, ID3D11ShaderResourceView *& perlinNoise);
	void SetWhiteNoise(ID3D11ShaderResourceView *& whiteNoise);
	void SetPerlinNoise(ID3D11ShaderResourceView *& perlinNoise);
	void SetGrainSettings(float intensity, float size, int type, bool hasColor = false);

	void ResetSSAO();
	void ResetBloom();
	void ResetLUT();

protected:
	virtual bool CreateBufferAdditionals(ID3D11Device *&device) override;
	virtual bool SetShaderParameters(ID3D11DeviceContext*, XMMATRIX&, XMMATRIX&, XMMATRIX&) override;

private:
	ID3D11Resource* m_screenBuffer;
	ID3D11ShaderResourceView* m_screenBufferView;
	ID3D11Resource* m_ssaoBuffer;
	ID3D11ShaderResourceView* m_ssaoBufferView;
	ID3D11Resource* m_bloomBuffer;
	ID3D11ShaderResourceView* m_bloomBufferView;
	ID3D11Resource* m_lutBuffer;
	ID3D11ShaderResourceView* m_lutBufferView;
	ID3D11Resource* m_whiteNoiseBuffer{ nullptr };
	ID3D11ShaderResourceView* m_whiteNoiseBufferView{ nullptr };
	ID3D11Resource* m_perlinNoiseBuffer{ nullptr };
	ID3D11ShaderResourceView* m_perlinNoiseBufferView{ nullptr };

	//BUFFERS
	ID3D11Buffer* m_textureBuffer;
	ID3D11Buffer* m_chromaticBuffer;
	ID3D11Buffer* m_grainBuffer;

	//CHROMATIC ABERRATION
	bool m_chromaticAberration = false;
	XMFLOAT3 m_chromaticAberrationOffset{ 0.00215124f, -0.0052312f, 0.0031225f };
	float m_chromaticAberrationIntensity{ 1.0f };

	//GRAIN
	//TODO Use struct for grain
	bool m_useGrain = false;
	float m_grainIntensity{ 0.0f }; //Should use struct
	float m_grainSize{ 0.0f };		//Should use struct
	bool m_hasGrainColor = false;	//Should use struct
	int m_type{ 0 };

	//Interal variables - changed only in PostProcessShader
	ID3D11Resource* m_chromaticAberrationTexture{ nullptr };
	ID3D11ShaderResourceView* m_chromaticAberrationTextureView{ nullptr };
};

#endif // !_POSTPROCESSSHADER_H_