#include "ShaderEditorManager.h"

ShaderEditorManager::ShaderEditorManager(D3DClass * d3d, MouseClass * mouse)
{
	m_D3D = d3d;
	m_mouse = mouse;
	m_pbrBlock = new UIShaderPBRBlock();
	m_pbrBlock->Initialize(d3d);
}

void ShaderEditorManager::UpdateBlocks()
{
	if (!RenderBlocks(m_D3D->GetDeviceContext()))
		return;

	//Check pins first - before moving block, interact with pins
	if (!UpdatePinsOfAllBlocks())
		return;

	if (!UpdatePBRBlock())
		return;

	//If user is dragging one block - he won't interact with any other block until freed previous one
	for (const auto& block : m_blocks)
	{
		if (block->IsDragging())
		{
			std::pair<float, float> mouseMov = m_mouse->MouseFrameMovement();
			block->Move(mouseMov.first, mouseMov.second);
			if (m_mouse->GetLMBPressed() == false)
			{
				block->StopDragging();
			}
			return;
		}
	}

	//User can start dragging only one block at the time - dragging multiple blocks is not available
	for (const auto& block : m_blocks)
	{
		if (block->MouseOnArea(m_mouse) && m_mouse->GetLMBPressed())
		{
			block->StartDragging();
			return;
		}
	}
}

bool ShaderEditorManager::UpdatePBRBlock()
{
	for (const auto& block : m_blocks)
	{
		if (block->IsDragging())
			return true;
	}

	if (m_pbrBlock)
	{
		m_pbrBlock->Render(m_D3D->GetDeviceContext());
		if (m_pbrBlock->IsDragging())
		{
			std::pair<float, float> mouseMov = m_mouse->MouseFrameMovement();
			m_pbrBlock->Move(mouseMov.first, mouseMov.second);
			if (m_mouse->GetLMBPressed() == false)
			{
				m_pbrBlock->StopDragging();
			}
			return false;
		}

		if (m_pbrBlock->MouseOnArea(m_mouse) && m_mouse->GetLMBPressed())
		{
			m_pbrBlock->StartDragging();
			return false;
		}
	}

	return true;
}

bool ShaderEditorManager::UpdatePinsOfAllBlocks()
{
	for (const auto& block : m_blocks)
	{
		if (block->IsDragging())
			return true;
	}

	if (m_pbrBlock && m_pbrBlock->IsDragging())
		return true;

	UIShaderEditorOutput* out = nullptr;
	UIShaderEditorBlock* currentBlock = nullptr;

	//If pin is being dragged - consume input
	for (const auto& block : m_blocks)
	{
		if (block->IsPinDragging())
		{
			out = block->DragPins(m_mouse);
			currentBlock = block;
		}
	}

	for (const auto& block : m_blocks)
	{
		if (UIShaderEditorInput* in = block->CheckIfMouseOnInputPin(m_mouse))
		{
			if (out && currentBlock && currentBlock->IsPinDragging() == false)
			{
				out->SetConnectedInput(in);
				in->ChangeColor(0.0f, 0.0f, 1.0f, 1.0f);
				DrawLine(in, out);
				break;
			}
		}
	}

	if (out)
	{
		return false;
	}
	//If no pin is being dragged - check to start dragging
	for (const auto& block : m_blocks)
	{
		if (block->DragPins(m_mouse))
			return false;
	}
	return true;
}

void ShaderEditorManager::DrawLine(UIShaderEditorInput * in, UIShaderEditorOutput * out)
{
	m_line = new UILine();
	m_line->Initialize(m_D3D, out, in);
}

void ShaderEditorManager::AddShaderBlock(UIShaderEditorBlock* && block)
{
	block->Initialize(m_D3D);
	m_blocks.push_back(block);
}

void ShaderEditorManager::AddShaderBlock(UIShaderEditorBlock *& block)
{
	m_blocks.push_back(block);
}

bool ShaderEditorManager::RenderBlocks(ID3D11DeviceContext* deviceContext)
{
	if (m_line)
	{
		if (!m_line->Render(deviceContext))
			return false;
	}

	for (const auto& block : m_blocks)
	{
		if (!block->Render(deviceContext))
			return false;
	}

	if (m_pbrBlock)
	{
		if (!m_pbrBlock->Render(deviceContext))
			return false;
	}

	return true;
}