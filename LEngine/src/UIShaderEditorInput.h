#pragma once
#ifndef _UISHADEREDITORINPUT_H_
#define _UISHADEREDITORINPUT_H_

#include "UIBase.h"
#include "UIShaderEditorOutput.h"

///<summary>Class used for rendering Material Editor input point</summary>
class UIShaderEditorInput : public UIBase
{
public:
	bool MouseOnArea(MouseClass* mouse) override;

	///<summary> Initialize shape: Rectangle/Triangle </summary>
	bool Initialize(D3DClass* d3d, ModelClass::ShapeSize shape, float left, float right, float top, float bottom);
	///<summary> Initialize shape: Square </summary>
	bool Initialize(D3DClass* d3d, float centerX, float centerY, float size);
	
	void Move(float x, float y);
	void GetTranslation(float& x, float& y) const;

	void GetPosition(float & x, float & y) const;

	virtual bool Render(ID3D11DeviceContext *deviceContext) final;
	virtual bool SetShaderParameters(ID3D11DeviceContext* deviceContext, XMMATRIX& worldMatrix, XMMATRIX& viewMatrix, XMMATRIX& projectionMatrix) override;

	void StartDragging();
	void StopDragging();
	bool IsDragging() const;

	void SetScale(float scale);

public:
	UIShaderEditorOutput* m_connectedOutputNode{ nullptr };
	std::string m_returnType{};
	ID3D11Resource* m_pinTexture;
	ID3D11ShaderResourceView* m_pinTextureView;

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