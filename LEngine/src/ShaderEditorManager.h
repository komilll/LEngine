#pragma once

#ifndef _SHADEREDITORMANAGER_H_
#define _SHADEREDITORMANAGER_H_

#include "UIShaderEditorBlock.h"
#include "UIShaderPBRBlock.h"
#include <vector>
#include "MouseClass.h"
#include "d3dclass.h"
#include "UILine.h"

static const char* ChoosingWindowItems[] = { "ADD", "SUBSTRACT" };
class ShaderEditorManager
{
public:
	ShaderEditorManager(D3DClass* d3d, MouseClass* mouse);

	void UpdateBlocks(bool mouseOnly = false);
	void AddShaderBlock(UIShaderEditorBlock* && block, int inCount, int outCount);
	void AddShaderBlock(UIShaderEditorBlock* & block);
	void GenerateCodeToFile();
	
	bool WillRenderChoosingWindow();
	int* GetChoosingWindowHandler();

private:
	bool RenderBlocks(ID3D11DeviceContext* deviceContext);
	bool UpdatePBRBlock(bool mouseOnly = false);
	bool UpdatePinsOfAllBlocks();
	void DrawLine(UIShaderEditorInput* in, UIShaderEditorOutput* out);

	//GENERATING SHADER CODE
	std::string GenerateBlockCode(UIShaderEditorBlock* block);
	std::string GetFunctionDeclarations();
	std::string GetFunctionDefinitions();
	vector<std::string> GetFilenamesInDirectory(std::string dir, bool withDir = true);

	void ShowFunctionChoosingWindow();
	void LoadFunctionsFromDirectory();

private:
	D3DClass* m_D3D;
	MouseClass* m_mouse;
	vector<UIShaderEditorBlock*> m_blocks = {};
	UIShaderEditorBlock* m_originalGeneratorBlock{};

	std::vector<UILine*> m_lines = {};
	UIShaderPBRBlock* m_pbrBlock;

	bool m_choosingWindow{ false };
	int m_choosingWindowHandler{ 0 };
};

#endif