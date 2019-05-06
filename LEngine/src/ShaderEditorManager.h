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
	void AddShaderBlock(UIShaderEditorBlock* && block);
	void AddShaderBlock(UIShaderEditorBlock* & block);

private:
	bool RenderBlocks(ID3D11DeviceContext* deviceContext);
	bool UpdatePBRBlock();
	bool UpdatePinsOfAllBlocks();
	void DrawLine(UIShaderEditorInput* in, UIShaderEditorOutput* out);

private:
	D3DClass* m_D3D;
	MouseClass* m_mouse;
	vector<UIShaderEditorBlock*> m_blocks = {};

	UILine* m_line{ nullptr };
	UIShaderPBRBlock* m_pbrBlock;
};

#endif