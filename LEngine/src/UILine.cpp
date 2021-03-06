#include "UILine.h"

//TODO Use constructor
bool UILine::Initialize(D3DClass * d3d, UIShaderEditorOutput * startPin, UIShaderEditorInput * endPin, float scale)
{
	if (!BaseShaderClass::Initialize(d3d->GetDevice(), *d3d->GetHWND(), L"uiline.vs", L"uiline.ps", BaseShaderClass::vertexInputType(GetInputNames(), GetInputFormats())))
		return false;

	m_D3D = d3d;
	m_startPin = startPin;
	m_endPin = endPin;
	m_scale = scale;

	return CalculateLine();
}

bool UILine::Render(ID3D11DeviceContext * deviceContext)
{
	CalculateLine();

	XMMATRIX worldMatrix = XMMatrixRotationZ(atan(m_yDiff / m_xDiff));
	worldMatrix = XMMatrixMultiply(worldMatrix, XMMatrixTranslation(m_translationX, m_translationY, 0.0f));
	worldMatrix = XMMatrixMultiply(worldMatrix, XMMatrixScaling(m_scale, m_scale, m_scale));

	return UIBase::Render(deviceContext, 0, worldMatrix, worldMatrix * 0, worldMatrix * 0);
}

void UILine::SetScale(float scale)
{
	m_scale = scale;
}

UIShaderEditorInput * UILine::GetInput()
{
	return m_endPin;
}

UIShaderEditorOutput * UILine::GetOutput()
{
	return m_startPin;
}

bool UILine::CalculateLine()
{
	float endPinX;
	float endPinY;
	float startPinX;
	float startPinY;

	m_startPin->GetPosition(startPinX, startPinY);
	m_endPin->GetPosition(endPinX, endPinY);

	m_xDiff = (endPinX - startPinX);
	m_yDiff = (endPinY - startPinY);
	const float lineLength = std::sqrt(m_xDiff * m_xDiff + m_yDiff * m_yDiff);

	m_startPin->GetPosition(m_translationX, m_translationY);
	m_vertices = RectangleVertices{ 0.0f, lineLength, 0.0f, lineThickness };

	return InitializeModelGeneric(m_D3D->GetDevice(), m_vertices);
}
