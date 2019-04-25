#pragma once
#ifndef _LUTSHADER_H_
#define _LUTSHADER_H_

#include "BaseShaderClass.h"
#include "WICTextureLoader.h"
#include "DDSTextureLoader.h"

class LUTShader : public BaseShaderClass
{
public:
	void SetLUT(ID3D11Device* device, const wchar_t* filename, bool isDDS = false);

protected:
	virtual bool SetShaderParameters(ID3D11DeviceContext *deviceContext, XMMATRIX& worldMatrix, XMMATRIX& viewMatrix, XMMATRIX& projectionMatrix) override;

public:
	ID3D11Resource* m_screenResource;
	ID3D11ShaderResourceView* m_screenResourceView;

private:
	ID3D11Resource* m_lut = nullptr;
	ID3D11ShaderResourceView* m_lutView = nullptr;
};

#endif // !_POSTPROCESSSHADER_H_