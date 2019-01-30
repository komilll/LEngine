#include "UISlider.h"

bool UISlider::Render(ID3D11DeviceContext * deviceContext, int indexCount, XMMATRIX & worldMatrix, XMMATRIX & viewMatrix, XMMATRIX & projectionMatrix)
{
	if (!UIBase::Render(deviceContext, indexCount, worldMatrix, viewMatrix, projectionMatrix))
		return false;

	XMFLOAT4 colorTemp{ 0,0,0,0 };
	GetColor(colorTemp);
	ChangeColor(1.0f, 1.0f, 1.0f, 1.0f);

	m_modelSlider->Render(deviceContext);
	if (!SetShaderParameters(deviceContext, XMMATRIX({m_modelSlider->GetPosition().x, 0,0,0}, { 0,0,0,0 }, { 0,0,0,0 }, { 0,0,0,0 }), viewMatrix, projectionMatrix))
		return false;

	RenderShader(deviceContext, indexCount);

	ChangeColor(colorTemp);

	return true;
}

bool UISlider::MouseOnArea(MouseClass * mouse)
{
	mouse->GetMouseLocationScreenSpace(m_mousePosX, m_mousePosY);

	return (m_mousePosX >= m_minX && m_mousePosX <= m_maxX &&
		m_mousePosY >= m_minY && m_mousePosY <= m_maxY);
}

bool UISlider::Initialize(D3DClass * d3d, float positionMinX, float positionMaxX, float positionY, float height)
{
	if (!BaseShaderClass::Initialize(d3d->GetDevice(), *d3d->GetHWND(), UI_SHADER_VS, UI_SHADER_PS, BaseShaderClass::vertexInputType(GetInputNames(), GetInputFormats())))
		return false;

	m_D3D = d3d;
	m_minX = positionMinX;
	m_maxX = positionMaxX;
	m_minY = positionY - height / 2 - k_sliderHeight;
	m_maxY = positionY + height / 2 + k_sliderHeight,
		
	m_modelSlider = new ModelClass;
	bool result = m_modelSlider->Initialize(d3d->GetDevice(), ModelClass::ShapeSize::RECTANGLE, positionMinX, positionMinX + k_sliderWidth, m_maxY, m_minY);
	if (!result)
		return false;

	return InitializeModelGeneric(d3d->GetDevice(), ModelClass::ShapeSize::RECTANGLE, positionMinX, positionMaxX, positionY + height / 2, positionY - height / 2);
}

void UISlider::ChangeSliderValue()
{
	m_sliderVal = (m_maxX - m_mousePosX) / (m_maxX - m_minX);
	m_modelSlider->SetPosition(m_mousePosX - m_minX, 0, 0);
}