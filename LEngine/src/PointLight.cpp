#include "PointLight.h"

PointLight::PointLight() 
{
	ModelClass::ModelClass();
	m_index = ShaderPBRGenerated::AddPointLight(GetPositionWithRadius(), GetColorWithStrength());
}

void PointLight::UpdateShader()
{
	ShaderPBRGenerated::UpdatePointLight(m_index, GetPositionWithRadius(), GetColorWithStrength());
}

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

void PointLight::SetPosition(float x, float y, float z)
{
	SetPosition(XMFLOAT3{ x, y, z });
}

void PointLight::SetPosition(XMFLOAT3 position)
{
	m_position.x = position.x;
	m_position.y = position.y;
	m_position.z = position.z;
	m_position.w = 1.0f;

	m_positionWithRadius.x = position.x;
	m_positionWithRadius.y = position.y;
	m_positionWithRadius.z = position.z;

	UpdateShader();
}

std::string PointLight::GetSaveData() const
{
	json11::Json obj = json11::Json::object{
		{ "modelName", m_modelFilename },
		{ "sceneName", m_savedName },
		{ "position", json11::Json::array{m_position.x, m_position.y, m_position.z} },
		{ "scale", json11::Json::array{m_scale.x, m_scale.y, m_scale.z} },
		{ "rotation", json11::Json::array{m_rotation.x, m_rotation.y, m_rotation.z} },
		{ "material", m_materialName },
	//Point light specific
		{ "radius", m_positionWithRadius .w },
		{ "color", json11::Json::array{m_colorWithStrength.x, m_colorWithStrength.y, m_colorWithStrength.z} },
		{ "strength", m_colorWithStrength.w }
	};
	return obj.dump();
}

float* PointLight::GetColorRef()
{
	return &m_colorWithStrength.x;
}

float* PointLight::GetStrengthRef()
{
	return &m_colorWithStrength.w;
}

float* PointLight::GetRadiusRef()
{
	return &m_positionWithRadius.w;
}

XMFLOAT4 PointLight::GetPositionWithRadius() const
{
	return m_positionWithRadius;
}

XMFLOAT4 PointLight::GetColorWithStrength() const
{
	return m_colorWithStrength;
}
