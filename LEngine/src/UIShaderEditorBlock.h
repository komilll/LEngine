#pragma once
#ifndef _UISHADEREDITOR_H_
#define _UISHADEREDITOR_H_

#include "UIBase.h"
#include "UIShaderEditorInput.h"

///<summary>Class used for rendering Material Editor blocks</summary>
class UIShaderEditorBlock : public UIBase
{
private:
	struct Size
	{
		float x;
		float y;

		Size(float x, float y) : x(x), y(y) {}
	};

	struct Margin
	{
		float x;
		float y;

		Margin(float x, float y) : x(x), y(y) {}
	};

public:
	UIShaderEditorBlock();

	bool MouseOnArea(MouseClass* mouse) override;

	bool Initialize(D3DClass* d3d);
	///<summary> Initialize shape: Rectangle/Triangle </summary>
	bool Initialize(D3DClass* d3d, ModelClass::ShapeSize shape, float left, float right, float top, float bottom);
	///<summary> Initialize shape: Square </summary>
	bool Initialize(D3DClass* d3d, float centerX, float centerY, float size);
	
	void Move(float x, float y);
	void StartDragging();
	void StopDragging();
	bool IsDragging();

	virtual bool Render(ID3D11DeviceContext *deviceContext) final;

private:
	void CalculateBlockSize();
	bool InitializeInputNodes();
	bool InitializeOutputNodes();

private:
	D3DClass* m_D3D{ nullptr };

	float m_translationX{ 0 };
	float m_translationY{ 0 };
	bool m_dragged = { false };
	RectangleVertices m_blockVertices;

	int m_inputNodesCount = 1;
	int m_outputNodesCount = 3;
	vector<UIShaderEditorInput*> m_inputNodes = {};
	vector<UIShaderEditorInput*> m_outputNodes = {};

private:
	const vector<Size> blockSizeVector = { Size{ 0.4f, 0.2f }, Size{ 0.4f, 0.28f }, Size{ 0.4f, 0.35f }, Size{ 0.4f, 0.43f } };
	const Margin inOutMargin = Margin{ 0.02f, 0.11f };
	const Size inOutSize = Size{ 0.03f, 0.04f };
	const float paddingBetweenBlocks = 0.08f;
};
#endif // !_UIBASE_H_