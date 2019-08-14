#include "PointLight.h"

void PointLight::SetParams(XMFLOAT3 position, float radius, XMFLOAT3 color, float strength)
{
	SetPosition(position);
	SetRadius(radius);
	SetColor(color);
	SetStrength(strength);
}

void PointLight::SetRadius(float radius)
{
	m_positionWithRadius.w = radius;
}

void PointLight::SetColor(XMFLOAT3 color)
{
	m_colorWithStrength.x = color.x;
	m_colorWithStrength.y = color.y;
	m_colorWithStrength.z = color.z;
}

void PointLight::SetStrength(float strength)
{
	m_colorWithStrength.w = strength;
}

XMFLOAT4 PointLight::GetPositionWithRadius() const
{
	return m_positionWithRadius;
}

XMFLOAT4 PointLight::GetColorWithStrength() const
{
	return m_colorWithStrength;
}
