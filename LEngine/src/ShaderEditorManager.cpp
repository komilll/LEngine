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
#pragma region HLSL CODE GENERATOR
	std::string func{};
	for (int i = 0; i < m_blocks.size(); ++i)
	{
		const auto& block = m_blocks.at(i);
		if (block->GetInputCount() == 0) //Variable
		{
			char variableName = i + 65;
			block->m_variableName = variableName;
			block->SetOutputPinName(block->m_variableName);
		}
	}
	for (int i = 0; i < m_blocks.size(); ++i)
	{
		const auto& block = m_blocks.at(i);
		if (block->GetInputCount() > 0) //Function
		{
			std::string variableName{ "out_" };
			variableName += (char)(i + '0');
			block->m_variableName = variableName;
		}
	}
	for (int i = 0; i < m_pbrBlock->m_inputNodes.size(); ++i)
	{
		if (UIShaderEditorOutput* out = m_pbrBlock->m_inputNodes.at(i)->m_connectedOutputNode)
		{
			std::string funcName{ m_pbrBlock->m_inputTypes.at(i) + " Get" + m_pbrBlock->m_inputNames.at(i) + "()" };
			std::string funcBody{ "\n{\n" };

			for (const auto& block : m_blocks)
			{
				if (out == block->GetFirstOutputNode())
				{
					m_originalGeneratorBlock = block;
					funcBody += GenerateBlockCode(block);
					break;
				}
			}

			func += (funcName + funcBody);
			func += "\n}";
			func += "\n"; //New line and empty line
		}
		else
		{
			std::string funcName{ m_pbrBlock->m_inputTypes.at(i) + " Get" + m_pbrBlock->m_inputNames.at(i) + "()" };
			func += funcName;
			func += "\n{\n";

			std::string type = m_pbrBlock->m_inputTypes.at(i);
			if (type == "float")
				func += "\treturn 0.0f;";
			else if (type == "float2")
				func += "\treturn float2(0.0f, 0.0f);";
			else if (type == "float3")
				func += "\treturn float3(0.0f, 0.0f, 0.0f);";
			else if (type == "float4")
				func += "\treturn float4(0.0f, 0.0f, 0.0f, 0.0f);";

			func += "\n}";
			func += "\n"; //New line and empty line
		}
	}

	std::ifstream  src("pbr_generated.ps", std::ios::binary);
	std::ofstream  dst("function.txt", std::ios::binary);

	dst << src.rdbuf();
	dst << func;
#pragma endregion

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

	//If dropped on input pin - connect them
	bool canTryPBR = true;
	for (const auto& block : m_blocks)
	{
		if (UIShaderEditorInput* in = block->CheckIfMouseOnInputPin(m_mouse))
		{
			if (out && currentBlock && currentBlock->IsPinDragging() == false)
			{
				in->m_connectedOutputNode = out;
				in->ChangeColor(0.0f, 0.0f, 1.0f, 1.0f);
				DrawLine(in, out);
				canTryPBR = false;
				break;
			}
		}
	}
	//If dropped on PBR input pin - connect them
	if (canTryPBR && m_pbrBlock)
	{
		if (UIShaderEditorInput* in = m_pbrBlock->CheckIfMouseOnInputPin(m_mouse))
		{
			if (out && currentBlock && currentBlock->IsPinDragging() == false)
			{
				in->m_connectedOutputNode = out;
				in->ChangeColor(0.0f, 0.0f, 1.0f, 1.0f);
				DrawLine(in, out);
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
	for (int i = m_lines.size() - 1; i >= 0; --i)
	{
		if (m_lines.at(i)->GetInput() == in)
		{
			delete m_lines.at(i);
			m_lines.at(i) = nullptr;
			m_lines.erase(m_lines.begin()+i);
		}
	}

	UILine* line = new UILine;
	line->Initialize(m_D3D, out, in);
	m_lines.push_back(line);
}

std::string ShaderEditorManager::GenerateBlockCode(UIShaderEditorBlock * block)
{
	if (block->m_inputNodes.size() == 0)
	{
		if (block->GetFirstOutputNode())
		{
			return block->GenerateShaderCode();
		}
		else
			return "";
	}

	std::string toReturn{};
	for (const auto& in : block->m_inputNodes)
	{
		UIShaderEditorOutput* out = in->m_connectedOutputNode;
		for (const auto& connectedBlock : m_blocks)
		{
			for (const auto& otherOut : connectedBlock->m_outputNodes)
			{
				if (out == otherOut)
				{
					toReturn += { GenerateBlockCode(connectedBlock) };
					goto skipHere;
				}
			}
		}
	skipHere:
		{}
	}

	if (m_originalGeneratorBlock == block)
	{
		toReturn += "\treturn ";
		toReturn += block->GenerateShaderCode(true);
	}
	else
		toReturn += block->GenerateShaderCode();

	return toReturn;
}

void ShaderEditorManager::AddShaderBlock(UIShaderEditorBlock* && block, int inCount, int outCount)
{
	block->Initialize(m_D3D, inCount, outCount);
	m_blocks.push_back(block);
}

void ShaderEditorManager::AddShaderBlock(UIShaderEditorBlock *& block)
{
	m_blocks.push_back(block);
}

bool ShaderEditorManager::RenderBlocks(ID3D11DeviceContext* deviceContext)
{
	for (const auto& line : m_lines)
	{
		if (!line->Render(deviceContext))
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