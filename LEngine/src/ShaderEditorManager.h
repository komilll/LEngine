#pragma once

#ifndef _SHADEREDITORMANAGER_H_
#define _SHADEREDITORMANAGER_H_

#include "UIShaderEditorBlock.h"
#include <vector>

class ShaderEditorManager
{
public:
	bool Render(ID3D11DeviceContext* deviceContext);
	void AddShaderBlock(UIShaderEditorBlock* && block);

private:
	bool RenderBlocks(ID3D11DeviceContext* deviceContext);

private:
	vector<UIShaderEditorBlock*> m_blocks = {};
};

#endif