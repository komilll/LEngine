#pragma once
#ifndef _UISHADERPBRBLOCK_H_
#define _UISHADERPBRBLOCK_H_

#include "UIBase.h"
#include "UIShaderEditorInput.h"
#include "TextEngine.h"

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

	UIShaderEditorInput * CheckIfMouseOnInputPin(MouseClass * mouse);

private:
	void CalculateBlockVertices();
	bool InitializeInputNodes();

public:
	const std::vector<std::string> m_inputTypes = { "float4", "float", "float", "float3" };
	const std::vector<std::string> m_inputNames = { "BaseColor", "Metalness", "Roughness", "Normal" };
	vector<UIShaderEditorInput*> m_inputNodes = {};

private:
	D3DClass* m_D3D{ nullptr };

	bool m_moveAfterInitializing{ false };
	XMFLOAT2 m_movemementAfterInitialization{ 0,0 };

	float m_translationX{ 0 };
	float m_translationY{ 0 };
	bool m_dragged = { false };

	UIBase::RectangleVertices m_blockVertices;
	TextEngine* m_textEngine{};
	std::vector<pair<float, float>> m_textPositionModifiers{ {-0.01f, 0.27f}, { 0.02f, 0.195f }, { 0.02f, 0.11f }, { 0.0075f-0.01f, 0.03f } };

	//INPUT - PBR
	//0 - Base Color
	//1 - Metallic
	//2 - Roughness
	//3 - Normal
	int m_inputNodesCount = 4;	

private:
	const Size blockSizeVector = Size{ 0.2f, 0.8f };
	const Margin inOutMargin = Margin{ 0.02f, 0.11f };
	const Size inOutSize = Size{ 0.03f, 0.04f };
	const float paddingBetweenBlocks = 0.08f;
	const XMFLOAT4 blockColor{0.4f, 0.4f, 0.4f, 0.8f};
};
#endif // !_UIBASE_H_