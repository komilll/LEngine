#pragma once
#ifndef _UISHADEREDITOR_H_
#define _UISHADEREDITOR_H_

#include "UIBase.h"
#include "UIShaderEditorInput.h"
#include "UIShaderEditorOutput.h"
#include "TextEngine.h"
#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <cctype>

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
	UIShaderEditorBlock(XMFLOAT2 startPosition);

	bool MouseOnArea(MouseClass* mouse) override;

	bool Initialize(D3DClass* d3d, int inCount, int outCount);
	
	void Move(float x, float y);
	void StartDragging();
	void StopDragging();
	bool IsDragging();
	bool IsPinDragging();

	UIShaderEditorOutput* DragPins(MouseClass* mouse);

	UIShaderEditorInput* CheckIfMouseOnInputPin(MouseClass* mouse);

	virtual bool Render(ID3D11DeviceContext *deviceContext) final;

	std::string GenerateShaderCode(bool skipTabulator = false);
	int GetInputCount();
	void SetOutputPinName(std::string name);
	UIShaderEditorOutput* GetFirstOutputNode();

private:
	void CalculateBlockSize(int inCount, int outCount);
	bool InitializeInputNodes(int inCount);
	bool InitializeOutputNodes(int outCount);

public:
	std::string m_variableName{"test"};
	vector<UIShaderEditorInput*> m_inputNodes = {};
	vector<UIShaderEditorOutput*> m_outputNodes = {};

private:
	D3DClass* m_D3D{ nullptr };

	bool m_moveAfterInitializing{ false };
	XMFLOAT2 m_movemementAfterInitialization{ 0,0 };

	float m_translationX{ 0 };
	float m_translationY{ 0 };
	bool m_dragged = { false };
	bool m_pinDragged = { false };
	RectangleVertices m_blockVertices;
	TextEngine* m_textEngine;

	std::string m_returnType{ "float" };
	std::vector<std::string> m_argumentTypes{ "float", "float" };
	std::string m_functionName{ "add" };
	std::string m_blockName{ "add" };

private:
	const vector<Size> blockSizeVector = { Size{ 0.4f, 0.2f }, Size{ 0.4f, 0.28f }, Size{ 0.4f, 0.35f }, Size{ 0.4f, 0.43f } };
	const Margin inOutMargin = Margin{ 0.02f, 0.11f };
	const Size inOutSize = Size{ 0.03f, 0.04f };
	const float paddingBetweenBlocks = 0.08f;
	const XMFLOAT4 blockColor{0.2f, 0.2f, 0.2f, 0.8f};
};
#endif // !_UIBASE_H_