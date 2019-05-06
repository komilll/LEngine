#include "UIShaderPBRBlock.h"

UIShaderPBRBlock::UIShaderPBRBlock()
{
	UIBase::UIBase();
	ChangeColor(blockColor);
}

UIShaderPBRBlock::UIShaderPBRBlock(XMFLOAT2 startPosition)
{
	UIBase::UIBase();
	m_moveAfterInitializing = true;
	m_movemementAfterInitialization = startPosition;
	ChangeColor(blockColor);
}

bool UIShaderPBRBlock::MouseOnArea(MouseClass * mouse)
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

	if (mouseX >(m_blockVertices.minX + m_translationX) && mouseX < (m_blockVertices.maxX + m_translationX) &&
		mouseY >(m_blockVertices.minY + m_translationY) && mouseY < (m_blockVertices.maxY + m_translationY))
	{
		result = true;
	}

	return result;
}

bool UIShaderPBRBlock::Initialize(D3DClass * d3d)
{
	m_D3D = d3d;
	if (!BaseShaderClass::Initialize(d3d->GetDevice(), *d3d->GetHWND(), UI_SHADER_VS, UI_SHADER_PS, BaseShaderClass::vertexInputType(GetInputNames(), GetInputFormats())))
		return false;

	CalculateBlockVertices();
	InitializeInputNodes();
	if (m_moveAfterInitializing)
	{
		Move(m_movemementAfterInitialization.x, m_movemementAfterInitialization.y);
	}

	return InitializeModelGeneric(d3d->GetDevice(), m_blockVertices);
}

void UIShaderPBRBlock::Move(float x, float y)
{
	for (const auto& node : m_inputNodes)
		node->Move(x, y);

	m_translationX += x;
	m_translationY += y;
}

void UIShaderPBRBlock::StartDragging()
{
	m_dragged = true;
}

void UIShaderPBRBlock::StopDragging()
{
	m_dragged = false;
}

bool UIShaderPBRBlock::IsDragging()
{
	return m_dragged;
}

bool UIShaderPBRBlock::Render(ID3D11DeviceContext * deviceContext)
{
	for (const auto& node : m_inputNodes)
		node->Render(deviceContext);

	XMMATRIX tmpMatrix;
	tmpMatrix *= 0;
	tmpMatrix.r[0] = XMVECTOR{ m_translationX, m_translationY, 0, 0 };

	return UIBase::Render(deviceContext, 0, tmpMatrix, tmpMatrix * 0, tmpMatrix * 0);
}

void UIShaderPBRBlock::CalculateBlockVertices()
{
	m_blockVertices = UIBase::RectangleVertices{ -blockSizeVector.x * 0.5f, blockSizeVector.x * 0.5f, -blockSizeVector.y * 0.5f, blockSizeVector.y * 0.5f };
}

bool UIShaderPBRBlock::InitializeInputNodes()
{
	for (int i = 0; i < m_inputNodesCount; ++i)
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
