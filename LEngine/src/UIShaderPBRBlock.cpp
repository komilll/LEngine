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

	m_textEngine = new TextEngine;
	m_textEngine->Initialize(m_D3D->GetDevice(), L"Fonts/font.spritefont");
	m_textEngine->WriteText(d3d->GetDeviceContext(), d3d->GetWindowSize().x, d3d->GetWindowSize().y, m_translationX, m_translationY, "Color", 0.38f, TextEngine::Align::CENTER);
	m_textEngine->WriteText(d3d->GetDeviceContext(), d3d->GetWindowSize().x, d3d->GetWindowSize().y, m_translationX, m_translationY, "Metalness", 0.38f, TextEngine::Align::CENTER);
	m_textEngine->WriteText(d3d->GetDeviceContext(), d3d->GetWindowSize().x, d3d->GetWindowSize().y, m_translationX, m_translationY, "Roughness", 0.38f, TextEngine::Align::CENTER);
	m_textEngine->WriteText(d3d->GetDeviceContext(), d3d->GetWindowSize().x, d3d->GetWindowSize().y, m_translationX, m_translationY, "Normal", 0.38f, TextEngine::Align::CENTER);

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
	XMMATRIX tmpMatrix;
	tmpMatrix *= 0;
	tmpMatrix.r[0] = XMVECTOR{ m_translationX, m_translationY, 0, 0 };
	
	if (UIBase::Render(deviceContext, 0, tmpMatrix, tmpMatrix * 0, tmpMatrix * 0))
	{
		for (const auto& node : m_inputNodes)
		{
			if (node && !node->Render(deviceContext))
				return false;
		}

		if (m_textEngine)
		{
			for (int i = 0; i < m_textPositionModifiers.size(); ++i)
			{
				m_textEngine->GetData(i)->SetPosition(m_translationX + m_textPositionModifiers.at(i).first, 
					m_translationY + m_textPositionModifiers.at(i).second, m_D3D->GetWindowSize().x, m_D3D->GetWindowSize().y);
				m_textEngine->RenderText(deviceContext, 1, 1);
			}
		}

		return true;
	}
	else
		return false;
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
		inputNode->m_returnType = m_inputTypes.at(i);
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

UIShaderEditorInput* UIShaderPBRBlock::CheckIfMouseOnInputPin(MouseClass* mouse)
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

