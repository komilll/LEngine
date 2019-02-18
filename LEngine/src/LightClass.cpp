#include "LightClass.h"

LightClass::LightClass()
{
}

void LightClass::SetPosition(float x, float y, float z)
{
	m_position = { x,y,z };
}

void LightClass::SetPosition(XMFLOAT3 position)
{
	m_position = position;
}

XMFLOAT3 LightClass::GetPosition()
{
	return m_position;
}

void LightClass::SetLookAt(float x, float y, float z)
{
	m_lookAt = { x,y,z };
}

void LightClass::SetLookAt(XMFLOAT3 lookAt)
{
	m_lookAt = lookAt;
}

void LightClass::GenerateViewMatrix()
{
	XMVECTOR eyePos = { m_position.x, m_position.y, m_position.z };
	XMVECTOR focusPos = { m_lookAt.x, m_lookAt.y, m_lookAt.z };
	XMVECTOR upVec = { 0, 1, 0 };

	m_viewMatrix = XMMatrixLookAtLH(eyePos, focusPos, upVec);
}

void LightClass::GenerateProjectionMatrix(float screenDepth, float screenNear)
{
	float fieldOfView = (float)3.14f / 2.0f;
	//float screenAspect = 16.0f / 9.0f;
	float screenAspect = 1.0f;

	m_projectionMatrix = XMMatrixPerspectiveFovLH(fieldOfView, screenAspect, screenNear, screenDepth);
}

void LightClass::GetViewMatrix(XMMATRIX & viewMatrix)
{
	viewMatrix = m_viewMatrix;
}

void LightClass::GetProjectionMatrix(XMMATRIX & projectionMatrix)
{
	m_projectionMatrix = projectionMatrix;
}