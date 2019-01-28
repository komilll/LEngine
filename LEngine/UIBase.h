#pragma once
#ifndef _UIBASE_H_
#define _UIBASE_H_

#include "BaseShaderClass.h"
#include "modelClass.h"

class UIBase : public BaseShaderClass
{
private:
	struct AppearanceBuffer 
	{
		XMFLOAT4 color;
	};

public:
	UIBase();

	bool InitializeModel(ID3D11Device* device, ModelClass::ShapeSize shape, float left, float right, float top, float bottom);
	bool InitializeSquare(ID3D11Device* device, float centerX, float centerY, float size);
	bool Render(ID3D11DeviceContext *deviceContext, int indexCount, XMMATRIX &worldMatrix, XMMATRIX &viewMatrix, XMMATRIX &projectionMatrix);
	void ChangeColor(XMFLOAT4 color);
	void ChangeColor(float r, float g, float b, float a);
	void ChangeAlpha(float alpha);

protected:
	virtual bool CreateBufferAdditionals(ID3D11Device *&device) override;
	virtual bool CreateSamplerState(ID3D11Device* device) override;
	virtual bool SetShaderParameters(ID3D11DeviceContext *deviceContext, XMMATRIX &worldMatrix, XMMATRIX &viewMatrix, XMMATRIX &projectionMatrix) override;

private:
	ModelClass* m_model;
	XMFLOAT4 m_uiColor = XMFLOAT4(1.0, 1.0, 1.0, 1.0);

	ID3D11Buffer* m_appearanceBuffer;
};
#endif // !_UIBASE_H_