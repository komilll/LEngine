#include "UIShaderEditorBlock.h"

UIShaderEditorBlock::UIShaderEditorBlock()
{
	UIBase::UIBase();
	ChangeColor(blockColor);
}

UIShaderEditorBlock::UIShaderEditorBlock(XMFLOAT2 startPosition)
{
	UIBase::UIBase();
	m_moveAfterInitializing = true;
	m_movemementAfterInitialization = startPosition;
	ChangeColor(blockColor);
}

bool UIShaderEditorBlock::MouseOnArea(MouseClass * mouse)
{
	bool result = false;
	float mouseX{ 0 };
	float mouseY{ 0 };
	POINT p = mouse->CurrentMouseLocation();

	//Calculate mouse X
	mouseX = (float)p.x / (float)m_D3D->GetWindowSize().x;
	mouseX = mouseX * 2.0f - 1.0f;
	if (mouseX > 1.0f)
		mouseX = 1.0f;
	else if (mouseX < -1.0f)
		mouseX = -1.0f;

	//Calculate mouse Y
	mouseY = (float)p.y / (float)m_D3D->GetWindowSize().y;
	mouseY = mouseY * 2.0f - 1.0f;
	if (mouseY > 1.0f)
		mouseY = 1.0f;
	else if (mouseY < -1.0f)
		mouseY = -1.0f;

	mouseY *= -1.0f;

	if (mouseX > (m_blockVertices.minX + m_translationX) && mouseX < (m_blockVertices.maxX + m_translationX) &&
		mouseY > (m_blockVertices.minY + m_translationY) && mouseY < (m_blockVertices.maxY + m_translationY) )
	{
		result = true;
	}

	return result;
}

bool UIShaderEditorBlock::Initialize(D3DClass * d3d, int inCount, int outCount)
{
	m_D3D = d3d;
	if (!BaseShaderClass::Initialize(d3d->GetDevice(), *d3d->GetHWND(), UI_SHADER_VS, UI_SHADER_PS, BaseShaderClass::vertexInputType(GetInputNames(), GetInputFormats())))
		return false;

	CalculateBlockSize(inCount, outCount);
	InitializeInputNodes(inCount);
	InitializeOutputNodes(outCount);
	if (m_moveAfterInitializing)
	{
		Move(m_movemementAfterInitialization.x, m_movemementAfterInitialization.y);
	}

	return InitializeModelGeneric(d3d->GetDevice(), m_blockVertices);
}

void UIShaderEditorBlock::Move(float x, float y)
{
	for (const auto& node : m_inputNodes)
		node->Move(x, y);

	for (const auto& node : m_outputNodes)
		node->Move(x, y);

	m_translationX += x;
	m_translationY += y;
}

void UIShaderEditorBlock::StartDragging()
{
	m_dragged = true;
}

void UIShaderEditorBlock::StopDragging()
{
	m_dragged = false;
}

bool UIShaderEditorBlock::IsDragging()
{
	return m_dragged;
}

bool UIShaderEditorBlock::IsPinDragging()
{
	return m_pinDragged;
}

UIShaderEditorOutput* UIShaderEditorBlock::DragPins(MouseClass * mouse)
{
	for (const auto& pin : m_outputNodes)
	{
		if (pin->IsDragging())
		{
			if (mouse->GetLMBPressed() == false)
			{
				pin->StopDragging();
				m_pinDragged = false;
			}
			return pin;
		}
	}

	for (const auto& pin : m_outputNodes)
	{
		if (pin->MouseOnArea(mouse) && mouse->GetLMBPressed())
		{
			pin->StartDragging();
			m_pinDragged = true;
			return pin;
		}
	}

	return nullptr;
}

bool UIShaderEditorBlock::Render(ID3D11DeviceContext * deviceContext)
{
	for (const auto& node : m_inputNodes)
		node->Render(deviceContext);

	for (const auto& node : m_outputNodes)
		node->Render(deviceContext);

	XMMATRIX tmpMatrix;
	tmpMatrix *= 0;
	tmpMatrix.r[0] = XMVECTOR{ m_translationX, m_translationY, 0, 0 };

	return UIBase::Render(deviceContext, 0, tmpMatrix, tmpMatrix * 0, tmpMatrix * 0);
}

std::string UIShaderEditorBlock::GenerateShaderCode()
{	
	std::string func{};
	if (GetInputCount() == 0) //Variable
	{
		func = { m_variableName + " = "};
		if (m_outputNodes.size() > 0)
		{
			float val = m_outputNodes.at(0)->m_value;
			std::ostringstream ss;
			ss << val;
			func += ss.str();
			func += ";\n";
		}
		else
		{
			func = "";
		}
	}
	else
	{
		func = { m_variableName + " = " + m_functionName + "(" };
		std::vector<float> args = {};
		for (const auto& node : m_inputNodes)
		{
			if (node->m_connectedOutputNode != nullptr)
			{
				args.push_back(node->m_connectedOutputNode->m_value);
			}
		}
		for (int i = 0; i < args.size(); ++i)
		{
			std::ostringstream ss;
			ss << args.at(i);
			func += ss.str();
			if (i < args.size() - 1)
			{
				func += ", ";
			}
		}
		func += ");";
	}

	return func;
}

int UIShaderEditorBlock::GetInputCount()
{
	return m_inputNodes.size();
}

void UIShaderEditorBlock::CalculateBlockSize(int inCount, int outCount)
{
	int inOutCount = inCount > outCount ? inCount : outCount;
	Size blockSize = blockSizeVector[inOutCount - 1];
	m_blockVertices.minX = -blockSize.x * 0.5f;
	m_blockVertices.maxX = blockSize.x * 0.5f;
	m_blockVertices.minY = -blockSize.y * 0.5f;
	m_blockVertices.maxY= blockSize.y * 0.5f;
}

bool UIShaderEditorBlock::InitializeInputNodes(int count)
{
	for (int i = 0; i < count; ++i)
	{
		UIShaderEditorInput* inputNode = new UIShaderEditorInput;
		if (!inputNode->Initialize(m_D3D, ModelClass::ShapeSize::RECTANGLE,
			m_blockVertices.minX + inOutMargin.x, m_blockVertices.minX + inOutMargin.x + inOutSize.x,
			m_blockVertices.maxY - inOutMargin.y, m_blockVertices.maxY - inOutMargin.y - inOutSize.y))
		{
			return false;
		}
		inputNode->ChangeColor(1.0f, 1.0f, 1.0f, 1.0f);
		inputNode->Move(0.0f, -(float)i * paddingBetweenBlocks);

		m_inputNodes.push_back(inputNode);
	}

	return true;
}

bool UIShaderEditorBlock::InitializeOutputNodes(int count)
{
	for (int i = 0; i < count; ++i)
	{
		UIShaderEditorOutput* outputNode = new UIShaderEditorOutput;
		if (!outputNode->Initialize(m_D3D, ModelClass::ShapeSize::RECTANGLE,
			m_blockVertices.maxX - inOutMargin.x - inOutSize.x, m_blockVertices.maxX - inOutMargin.x,
			m_blockVertices.maxY - inOutMargin.y, m_blockVertices.maxY - inOutMargin.y - inOutSize.y))
		{
			return false;
		}
		outputNode->ChangeColor(1.0f, 1.0f, 1.0f, 1.0f);
		outputNode->Move(0.0f, -(float)i * paddingBetweenBlocks);

		m_outputNodes.push_back(outputNode);
	}

	return true;
}

UIShaderEditorInput* UIShaderEditorBlock::CheckIfMouseOnInputPin(MouseClass* mouse)
{
	for (const auto& pin : m_inputNodes)
	{
		if (pin->MouseOnArea(mouse))
		{
			return pin;
		}
	}
	return nullptr;
}
