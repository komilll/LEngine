#pragma once
#ifndef _MODELPCIKERSHADER_H_
#define _MODELPCIKERSHADER_H_

#include "BaseShaderClass.h"
#include "modelClass.h"
#include "d3dclass.h"
#include "MouseClass.h"

///<summary>Class used for rendering model object picker</summary>
class ModelPickerShader : public BaseShaderClass
{
private:
	struct AppearanceBuffer
	{
		XMFLOAT4 color{ 0.0f, 0.0f, 0.0f, 1.0f }; //Tint of model
	};

public:
	virtual bool Render(ID3D11DeviceContext *deviceContext, int indexCount, XMMATRIX &worldMatrix, XMMATRIX &viewMatrix, XMMATRIX &projectionMatrix);
	
	void ChangeColor(XMFLOAT4 color);
	void ChangeColor(float r, float g, float b, float a = 1.0f);
	void ChangeAlpha(float alpha);

	virtual void GetColor(XMFLOAT4 &color) const final;

protected:
	virtual bool CreateBufferAdditionals(ID3D11Device *&device) override;
	virtual bool CreateSamplerState(ID3D11Device* device) override;
	virtual bool SetShaderParameters(ID3D11DeviceContext *deviceContext, XMMATRIX &worldMatrix, XMMATRIX &viewMatrix, XMMATRIX &projectionMatrix) override;

protected:
	XMFLOAT4 m_tint{ 1.0, 1.0, 1.0, 1.0 }; //White tint of picker

	ID3D11Buffer* m_appearanceBuffer;
	ID3D11Buffer* m_positionBuffer;
};
#endif // !_UIBASE_H_