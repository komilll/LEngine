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

	void UpdateBlocks();
	void AddShaderBlock(UIShaderEditorBlock* && block, int inCount, int outCount);
	void AddShaderBlock(UIShaderEditorBlock* & block);

private:
	bool RenderBlocks(ID3D11DeviceContext* deviceContext);
	bool UpdatePBRBlock();
	bool UpdatePinsOfAllBlocks();
	void DrawLine(UIShaderEditorInput* in, UIShaderEditorOutput* out);
	std::string GenerateBlockCode(UIShaderEditorBlock* block);

private:
	D3DClass* m_D3D;
	MouseClass* m_mouse;
	vector<UIShaderEditorBlock*> m_blocks = {};
	UIShaderEditorBlock* m_originalGeneratorBlock{};

	std::vector<UILine*> m_lines = {};
	UIShaderPBRBlock* m_pbrBlock;
};

#endif