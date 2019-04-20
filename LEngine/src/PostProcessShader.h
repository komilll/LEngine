#pragma once
#ifndef _POSTPROCESSSHADER_H_
#define _POSTPROCESSSHADER_H_

#include "BaseShaderClass.h"

class PostProcessShader : public BaseShaderClass
{
private:
	struct TextureBufferType
	{
		int hasSSAO;
		int hasBloom;
		XMFLOAT2 padding;
	};

public:
	void SetScreenBuffer(ID3D11ShaderResourceView *&screenBuffer);
	void SetSSAOBuffer(ID3D11ShaderResourceView *&ssaoBuffer);
	void SetBloomBuffer(ID3D11ShaderResourceView *&bloomBuffer);

	void ResetSSAO();
	void ResetBloom();

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


	ID3D11Buffer* m_textureBuffer;
};

#endif // !_POSTPROCESSSHADER_H_