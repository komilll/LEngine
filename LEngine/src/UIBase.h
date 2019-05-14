#pragma once
#ifndef _UIBASE_H_
#define _UIBASE_H_

#include "BaseShaderClass.h"
#include "modelClass.h"
#include "d3dclass.h"
#include "MouseClass.h"

constexpr WCHAR* UI_SHADER_VS = L"uibase.vs";
constexpr WCHAR* UI_SHADER_PS = L"uibase.ps";

///<summary>Class used for rendering objects on screenspace</summary>
class UIBase : public BaseShaderClass
{
public:
	struct RectangleVertices
	{
		float minX;
		float maxX;
		float minY;
		float maxY;

		RectangleVertices()
		{
			RectangleVertices(0, 0, 0, 0);
		}
		RectangleVertices(float minX, float maxX, float minY, float maxY) :
			minX(minX), maxX(maxX), minY(minY), maxY(maxY)
		{}
	};

private:
	struct AppearanceBuffer
	{
		XMFLOAT4 color; //Tint of model
	};

public:
	UIBase();

	virtual bool Render(ID3D11DeviceContext *deviceContext, int indexCount, XMMATRIX &worldMatrix, XMMATRIX &viewMatrix, XMMATRIX &projectionMatrix);
	virtual bool MouseOnArea(MouseClass* mouse);
	
	void ChangeColor(XMFLOAT4 color);
	void ChangeColor(float r, float g, float b, float a);
	void ChangeAlpha(float alpha);

	ModelClass* GetModel();

	///<summary>Initialize rectangle model by specifying its vertices</summary>
	virtual bool InitializeModelGeneric(ID3D11Device* device, RectangleVertices rectangleVertices, bool withTex = true, bool isEmpty = false);
	virtual bool InitializeModelGeneric(ID3D11Device* device, ModelClass::ShapeSize shape, float left, float right, float top, float bottom, bool withTex = true, bool isEmpty = false);
	virtual bool InitializeSquare(ID3D11Device* device, float centerX, float centerY, float size, bool isEmpty = false, bool withTex = false);

protected:
	virtual bool CreateBufferAdditionals(ID3D11Device *&device) override;
	virtual bool CreateSamplerState(ID3D11Device* device) override;
	virtual bool SetShaderParameters(ID3D11DeviceContext *deviceContext, XMMATRIX &worldMatrix, XMMATRIX &viewMatrix, XMMATRIX &projectionMatrix) override;

	virtual std::vector<LPCSTR> GetInputNames() final;
	virtual std::vector<DXGI_FORMAT> GetInputFormats() final;
	virtual void GetColor(XMFLOAT4 &color) final;

protected:
	ModelClass* m_model;
	XMFLOAT4 m_uiColor = XMFLOAT4(1.0, 1.0, 1.0, 1.0);

	ID3D11Buffer* m_appearanceBuffer;
	ID3D11Buffer* m_positionBuffer;
};
#endif // !_UIBASE_H_