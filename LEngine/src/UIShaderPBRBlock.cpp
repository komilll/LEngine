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

	if (mouseX >(m_blockVertices.minX + m_translationX) * m_scale && mouseX < (m_blockVertices.maxX + m_translationX) * m_scale &&
		mouseY >(m_blockVertices.minY + m_translationY) * m_scale && mouseY < (m_blockVertices.maxY + m_translationY) * m_scale)
	{
		result = true;
	}

	return result;
}

bool UIShaderPBRBlock::Initialize(D3DClass * d3d)
{
	m_D3D = d3d;
	if (!BaseShaderClass::Initialize(d3d->GetDevice(), *d3d->GetHWND(), L"uiline.vs", L"uiline.ps", BaseShaderClass::vertexInputType(GetInputNames(), GetInputFormats())))
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

	m_outlineObject = new UIBase;
	if (!m_outlineObject->Initialize(d3d->GetDevice(), *d3d->GetHWND(), L"uiline.vs", L"uiline.ps", BaseShaderClass::vertexInputType(GetInputNames(), GetInputFormats())))
		return false;
	if (!m_outlineObject->InitializeModelGeneric(d3d->GetDevice(), CalculateOutlineSize(m_blockVertices), false, true))
		return false;
	m_outlineObject->ChangeColor(outlineColor);

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

void UIShaderPBRBlock::SetScale(float scale)
{
	m_scale = scale;
}

bool UIShaderPBRBlock::TryToMarkBlock(RectangleVertices markingBounds)
{
	int correctPos = 0; //Need to get 2 points
	float minX = m_blockVertices.minX + m_translationX;
	float maxX = m_blockVertices.maxX + m_translationX;
	float minY = m_blockVertices.minY + m_translationY;
	float maxY = m_blockVertices.maxY + m_translationY;

	//TEST X POSITION
	//Block in middle X
	if (markingBounds.minX < minX && markingBounds.maxX > maxX)
	{
		correctPos++;
	}
	//Block on left crossed X
	else if (markingBounds.minX > minX && markingBounds.maxX > maxX && markingBounds.minX < maxX)
	{
		correctPos++;
	}
	//Block on right crossed X
	else if (markingBounds.maxX < maxX && markingBounds.minX < minX && markingBounds.maxX > minX)
	{
		correctPos++;
	}
	//Crossed through middle X
	else if (minX < markingBounds.minX && maxX > markingBounds.maxX)
	{
		correctPos++;
	}
	//TEXT Y POSITION
	//Block in middle Y
	if (markingBounds.minY < minY && markingBounds.maxY > maxY)
	{
		correctPos++;
	}
	//Block on top crossed Y
	else if (markingBounds.maxY < maxY && markingBounds.minY < minY && markingBounds.maxY > minY)
	{
		correctPos++;
	}
	//Block on bottom crossed Y
	else if (markingBounds.maxY > maxY && markingBounds.minY > minY && markingBounds.minY < maxY)
	{
		correctPos++;
	}
	//Crossed through middle Y
	else if (markingBounds.maxY < maxY && markingBounds.minY > minY && markingBounds.maxY > minY)
	{
		correctPos++;
	}

	return correctPos >= 2;
}

bool UIShaderPBRBlock::Render(ID3D11DeviceContext * deviceContext)
{
	//XMMATRIX tmpMatrix;
	//tmpMatrix *= 0;
	//tmpMatrix.r[0] = XMVECTOR{ m_translationX, m_translationY, 0, 0 };

	XMMATRIX worldMatrix = XMMatrixIdentity();
	worldMatrix = XMMatrixMultiply(worldMatrix, XMMatrixTranslation(m_translationX, m_translationY, 0.0f));
	worldMatrix = XMMatrixMultiply(worldMatrix, XMMatrixScaling(m_scale, m_scale, m_scale));
	
	if (m_focused && m_outlineObject)
	{
		//Render outline
		if (m_outlineObject && !m_outlineObject->Render(deviceContext, 0, worldMatrix, worldMatrix * 0, worldMatrix * 0))
			return false;
	}

	worldMatrix = XMMatrixIdentity();
	worldMatrix = XMMatrixMultiply(worldMatrix, XMMatrixTranslation(m_translationX, m_translationY, 0.0f));
	worldMatrix = XMMatrixMultiply(worldMatrix, XMMatrixScaling(m_scale, m_scale, m_scale));

	if (UIBase::Render(deviceContext, 0, worldMatrix, worldMatrix * 0, worldMatrix * 0))
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
				m_textEngine->GetData(i)->SetPosition((m_translationX + m_textPositionModifiers.at(i).first) * m_scale,
					(m_translationY + m_textPositionModifiers.at(i).second) * m_scale, m_D3D->GetWindowSize().x, m_D3D->GetWindowSize().y);
				m_textEngine->GetData(i)->scale = 0.38f * m_scale;
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

UIBase::RectangleVertices UIShaderPBRBlock::CalculateOutlineSize(RectangleVertices blockSize)
{
	return RectangleVertices{ blockSize.minX - outlineMargin, blockSize.maxX + outlineMargin, blockSize.minY - outlineMargin, blockSize.maxY + outlineMargin };
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

