#include "UITick.h"

bool UITick::Initialize(D3DClass * d3d, float positionX, float positionY, float size)
{
	if (!BaseShaderClass::Initialize(d3d->GetDevice(), *d3d->GetHWND(), UI_SHADER_VS, UI_SHADER_PS, BaseShaderClass::vertexInputType(GetInputNames(), GetInputFormats())))
		return false;

	m_D3D = d3d;
	m_positionX = positionX;
	m_positionY = positionY;
	m_size = size;
	m_tickState = false;

	//Calculate area bounds for mouse
	float left = positionX - size * 0.5f;
	float right = positionX + size * 0.5f;
	float top = positionY / 2 + size * 0.5f;
	float bottom = positionY / 2 - size * 0.5f;

	float widthLeft = 0.003f;
	float widthTop = widthLeft * 16.0f / 9.0f;

	m_leftMost = left - widthLeft;
	m_rightMost = right + widthLeft;
	m_topMost = top * 2.0f - size * 3.0f;
	m_bottomMost = bottom * 2.0f - size * 3.0f;

	return InitializeSquare(d3d->GetDevice(), positionX, positionY, size, m_tickState);
}

bool UITick::ChangeTick()
{
	m_tickState = !m_tickState;
	return InitializeSquare(m_D3D->GetDevice(), m_positionX, m_positionY, m_size, m_tickState);
}

bool UITick::MouseOnArea(MouseClass * mouse)
{
	float posX, posY;
	mouse->GetMouseLocationScreenSpace(posX, posY);

	return (posX >= m_leftMost && posX <= m_rightMost
		&& posY >= m_bottomMost && posY <= m_topMost);
}