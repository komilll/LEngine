#include "ShaderEditorManager.h"

bool ShaderEditorManager::Render(ID3D11DeviceContext * deviceContext)
{
	return RenderBlocks(deviceContext);
}

void ShaderEditorManager::AddShaderBlock(UIShaderEditorBlock* && block)
{
	m_blocks.push_back(block);
}

bool ShaderEditorManager::RenderBlocks(ID3D11DeviceContext* deviceContext)
{
	for (const auto& block : m_blocks)
	{
		if (!block->Render(deviceContext))
			return false;
	}

	return true;
}
