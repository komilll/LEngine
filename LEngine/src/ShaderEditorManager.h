#pragma once

#ifndef _SHADEREDITORMANAGER_H_
#define _SHADEREDITORMANAGER_H_

#include "UIShaderEditorBlock.h"
#include "UIShaderPBRBlock.h"
#include <vector>
#include "MouseClass.h"
#include "d3dclass.h"
#include "UILine.h"

class ShaderEditorManager
{
public:
	ShaderEditorManager(D3DClass* d3d, MouseClass* mouse);

	void UpdateBlocks(bool mouseOnly = false);
	void AddShaderBlock(UIShaderEditorBlock* block, int inCount, int outCount);
	void AddShaderBlock(UIShaderEditorBlock* block);
	void GenerateCodeToFile();
	
	bool WillRenderChoosingWindow();
	int* GetChoosingWindowHandler();
	void CreateBlock(std::string name);
	float GetWindowPositionX();
	float GetWindowPositionY();
	void PressedOutsideOfChoosingWindow();
	void DeleteCurrentShaderBlock();

private:
	bool RenderBlocks(ID3D11DeviceContext* deviceContext);
	bool UpdatePBRBlock(bool mouseOnly = false);
	bool UpdatePinsOfAllBlocks();
	void DrawLine(UIShaderEditorInput* in, UIShaderEditorOutput* out);
	void ResetFocusOnAllBlocks();
	void CreateChoosingWindowItemsArray();

	//GENERATING SHADER CODE
	std::string GenerateBlockCode(UIShaderEditorBlock* block);
	std::string GetFunctionDeclarations();
	std::string GetFunctionDefinitions();
	vector<std::string> GetFilenamesInDirectory(std::string dir, bool withDir = true);

	void ShowFunctionChoosingWindow();
	void LoadFunctionsFromDirectory();

public:
	std::vector<const char*> ChoosingWindowItems{};
	std::vector<char> m_choosingWindowSearch{ ' ' };
	const int k_choosingWindowSearchSize{ 10 };

private:
	D3DClass* m_D3D;
	MouseClass* m_mouse;
	vector<UIShaderEditorBlock*> m_blocks = {};
	UIShaderEditorBlock* m_originalGeneratorBlock{};

	std::vector<UILine*> m_lines = {};
	UIShaderPBRBlock* m_pbrBlock;

	bool m_choosingWindow{ false };
	int m_choosingWindowHandler{ 0 };
	int m_choosingWindowPosX{ 0 };
	int m_choosingWindowPosY{ 0 };
	float m_choosingWindowPosXScreenspace{ 0 };
	float m_choosingWindowPosYScreenspace{ 0 };
};

#endif