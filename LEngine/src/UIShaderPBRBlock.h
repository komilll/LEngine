#pragma once
#ifndef _UISHADERPBRBLOCK_H_
#define _UISHADERPBRBLOCK_H_

#include "UIBase.h"
#include "UIShaderEditorInput.h"

///<summary>Class used for rendering Material Editor blocks</summary>
class UIShaderPBRBlock : public UIBase
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
	UIShaderPBRBlock();
	UIShaderPBRBlock(XMFLOAT2 startPosition);

	bool MouseOnArea(MouseClass* mouse) override;

	bool Initialize(D3DClass* d3d);
	
	void Move(float x, float y);
	void StartDragging();
	void StopDragging();
	bool IsDragging();

	virtual bool Render(ID3D11DeviceContext *deviceContext) final;

private:
	void CalculateBlockVertices();
	bool InitializeInputNodes();

private:
	D3DClass* m_D3D{ nullptr };

	bool m_moveAfterInitializing{ false };
	XMFLOAT2 m_movemementAfterInitialization{ 0,0 };

	float m_translationX{ 0 };
	float m_translationY{ 0 };
	bool m_dragged = { false };

	int m_inputNodesCount = 1;
	vector<UIShaderEditorInput*> m_inputNodes = {};
	UIBase::RectangleVertices m_blockVertices;

private:
	const Size blockSizeVector = Size{ 0.2f, 0.8f };
	const Margin inOutMargin = Margin{ 0.02f, 0.11f };
	const Size inOutSize = Size{ 0.03f, 0.04f };
	const float paddingBetweenBlocks = 0.08f;
	const XMFLOAT4 blockColor{0.4f, 0.4f, 0.4f, 0.8f};
};
#endif // !_UIBASE_H_