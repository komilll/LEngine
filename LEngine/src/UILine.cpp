#include "UILine.h"

UILine::UILine()
{
	UIBase::UIBase();
}

bool UILine::Initialize(D3DClass * d3d, UIShaderEditorOutput * startPin, UIShaderEditorInput * endPin)
{
	if (!BaseShaderClass::Initialize(d3d->GetDevice(), *d3d->GetHWND(), UI_SHADER_VS, UI_SHADER_PS, BaseShaderClass::vertexInputType(GetInputNames(), GetInputFormats())))
		return false;

	m_startPin = startPin;
	m_endPin = endPin;

	float endPinX;
	float endPinY;
	float startPinX;
	float startPinY;

	m_startPin->GetPosition(startPinX, startPinY);
	m_endPin->GetPosition(endPinX, endPinY);

	float xDiff = (endPinX - startPinX);
	float yDiff = (endPinY - startPinY);
	//float lineLength = std::sqrt(xDiff * xDiff + yDiff * yDiff);
	float lineLength = std::abs(xDiff);

	startPin->GetPosition(m_translationX, m_translationY);
	//m_translationX += (lineLength * 0.5f);

	m_vertices = RectangleVertices{0.0f, lineLength, 0.0f, lineThickness};

	return InitializeModelGeneric(d3d->GetDevice(), m_vertices);
}

bool UILine::Render(ID3D11DeviceContext * deviceContext)
{
	XMMATRIX tmpMatrix;
	tmpMatrix *= 0;
	tmpMatrix.r[0] = XMVECTOR{ m_translationX, m_translationY, 0, 0 };

	return UIBase::Render(deviceContext, 0, tmpMatrix, tmpMatrix * 0, tmpMatrix * 0);
}
