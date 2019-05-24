#pragma once
#ifndef _UILINE_H_
#define _UILINE_H_

#include "UIBase.h"
#include "UIShaderEditorInput.h"
#include "UIShaderEditorOutput.h"

///<summary>Class used for rendering Material Editor input point</summary>
class UILine : public UIBase
{
public:
	UILine();

	///<summary> Initialize line</summary>
	bool Initialize(D3DClass* d3d, UIShaderEditorOutput* startPin, UIShaderEditorInput* endPin);
	
	virtual bool Render(ID3D11DeviceContext *deviceContext) final;
	void SetScale(float scale);

	UIShaderEditorInput* GetInput();
	UIShaderEditorOutput* GetOutput();

private:
	bool CalculateLine();

private:
	D3DClass* m_D3D{ nullptr };
	UIShaderEditorInput* m_endPin;
	UIShaderEditorOutput* m_startPin;
	UIBase::RectangleVertices m_vertices;

	float m_translationX{ 0.0f };
	float m_translationY{ 0.0f };

	float m_xDiff{ 0.0f };
	float m_yDiff{ 0.0f };

	float m_scale{ 1.0f };

private:
	const float lineThickness = 0.008f;
};
#endif // !_UIBASE_H_