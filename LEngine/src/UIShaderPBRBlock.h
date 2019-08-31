#pragma once
#ifndef _UISHADERPBRBLOCK_H_
#define _UISHADERPBRBLOCK_H_

#include "UIBase.h"
#include "UIShaderEditorInput.h"
#include "TextEngine.h"
#include <array>

///<summary>Class used for rendering Material Editor blocks</summary>
class UIShaderPBRBlock : public UIBase
{
	//TODO Should be sibling of UIShaderEditorBlock
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
	
	void ResetPosition();
	void Move(float x, float y);
	void StartDragging();
	void StopDragging();
	bool IsDragging() const;
	void SetScale(float scale);
	bool TryToMarkBlock(RectangleVertices markingBounds);
	UIShaderPBRBlock::Size GetPosition();

	virtual bool Render(ID3D11DeviceContext *deviceContext) final;

	UIShaderEditorInput * CheckIfMouseOnInputPin(MouseClass * mouse);

private:
	void CalculateBlockVertices();
	bool InitializeInputNodes();
	RectangleVertices CalculateOutlineSize(RectangleVertices blockSize);

private:
	//INPUT - PBR
	//0 - Base Color
	//1 - Metallic
	//2 - Roughness
	//3 - Normal
	//4 - Emissive Color
	static const int m_inputNodesCount{ 5 };

public:
	const std::array<std::string, m_inputNodesCount> m_inputTypes{ "float4", "float", "float", "float3", "float3" };
	const std::array<std::string, m_inputNodesCount> m_inputNames{ "Base Color", "Metalness", "Roughness", "Normal", "Emissive Color" };
	vector<UIShaderEditorInput*> m_inputNodes = {};
	bool m_focused{ false };

private:
	D3DClass* m_D3D{ nullptr };

	bool m_moveAfterInitializing{ false };
	XMFLOAT2 m_movemementAfterInitialization{ 0,0 };

	float m_translationX{ 0 };
	float m_translationY{ 0 };
	bool m_dragged = { false };
	float m_scale{ 1.0f };
	UIBase* m_outlineObject{};

	UIBase::RectangleVertices m_blockVertices;
	TextEngine* m_textEngine{};
	const std::vector<std::pair<float, float>> m_textPositionModifiers{ {-0.04f, 0.295f}, { -0.04f, 0.21f }, { -0.04f, 0.13f }, { -0.04f, 0.05f }, { -0.04f, -0.03f } };

private:
	const Size blockSizeVector = Size{ 0.2f, 0.8f };
	const Margin inOutMargin = Margin{ 0.02f, 0.11f };
	const Size inOutSize = Size{ 0.03f, 0.04f };
	const float paddingBetweenBlocks = 0.08f;
	const XMFLOAT4 blockColor{0.4f, 0.4f, 0.4f, 0.8f};
	//Outline
	const XMFLOAT4 outlineColor{ 0.7f, 0.7f, 0.7f, 0.95f };
	const float outlineMargin = 0.0f;
};
#endif // !_UIBASE_H_