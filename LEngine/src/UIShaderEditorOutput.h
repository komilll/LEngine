#pragma once
#ifndef _UISHADEREDITOROUTPUT_H_
#define _UISHADEREDITOROUTPUT_H_

#include "UIBase.h"

///<summary>Class used for rendering Material Editor input point</summary>
class UIShaderEditorOutput : public UIBase
{
public:
	UIShaderEditorOutput();

	bool MouseOnArea(MouseClass* mouse) override;

	///<summary> Initialize shape: Rectangle/Triangle </summary>
	bool Initialize(D3DClass* d3d, ModelClass::ShapeSize shape, float left, float right, float top, float bottom);
	///<summary> Initialize shape: Square </summary>
	bool Initialize(D3DClass* d3d, float centerX, float centerY, float size);
	
	void Move(float x, float y);
	void GetTranslation(float & x, float & y);
	void GetPosition(float & x, float & y);

	virtual bool Render(ID3D11DeviceContext *deviceContext) final;
	virtual bool SetShaderParameters(ID3D11DeviceContext* deviceContext, XMMATRIX& worldMatrix, XMMATRIX& viewMatrix, XMMATRIX& projectionMatrix) override;

	void StartDragging();
	void StopDragging();
	bool IsDragging();

	void SetScale(float scale);

public:
	float m_value{ 0.0f };
	float m_valueTwo[2]{ 0.0f, 0.0f };
	float m_valueThree[3]{ 0.0f, 0.0f, 0.0f };
	float m_valueFour[4]{ 0.0f, 0.0f, 0.0f, 0.0f };
	std::string m_visibleName{};

	std::string m_variableName{};
	std::string m_returnType{};
	bool m_toDeleteLine{ false };
	ID3D11Resource* m_pinTexture;
	ID3D11ShaderResourceView* m_pinTextureView;

	ID3D11Resource* m_connectedTexture;
	ID3D11ShaderResourceView* m_connectedTextureView;
	std::string m_texturePath{};

private:
	bool m_dragged{ false };

	float m_translationX{ 0 };
	float m_translationY{ 0 };

	float m_scale{ 1.0f };

	D3DClass* m_D3D{};

	float min_X;
	float max_X;
	float min_Y;
	float max_Y;
};
#endif // !_UIBASE_H_