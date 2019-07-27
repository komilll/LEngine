#pragma once
#ifndef _FXAASHADER_H_
#define _FXAASHADER_H_

#include "BaseShaderClass.h"

class FXAAShader : public BaseShaderClass
{
	//Empty - option may be added in future (e.g. user settings)
public:
	void SetScreenBuffer(ID3D11ShaderResourceView* screenBuffer);

protected:
	virtual bool CreateBufferAdditionals(ID3D11Device *&device) override;
	virtual bool SetShaderParameters(ID3D11DeviceContext* deviceContext, XMMATRIX& worldMatrix, XMMATRIX& viewMatrix, XMMATRIX& projectionMatrix) override;

private:
	ID3D11Resource* m_screenBuffer;
	ID3D11ShaderResourceView* m_screenBufferView;
};

#endif // !_FXAASHADER_H_