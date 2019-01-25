#pragma once
#ifndef _UIBASE_H_
#define _UIBASE_H_

#include "BaseShaderClass.h"
#include "modelClass.h"

class UIBase : public BaseShaderClass
{
public:
	UIBase();

	bool InitializeModel(ID3D11Device* device, ModelClass::ShapeSize shape, float left, float right, float top, float bottom);
	bool Render(ID3D11DeviceContext *deviceContext, int indexCount, XMMATRIX &worldMatrix, XMMATRIX &viewMatrix, XMMATRIX &projectionMatrix);

protected:
	virtual bool CreateBufferAdditionals(ID3D11Device *&device) override;
	virtual bool CreateSamplerState(ID3D11Device* device) override;
	virtual bool SetShaderParameters(ID3D11DeviceContext *deviceContext, XMMATRIX &worldMatrix, XMMATRIX &viewMatrix, XMMATRIX &projectionMatrix) override;

private:
	ModelClass* m_model;
};
#endif // !_UIBASE_H_