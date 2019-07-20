////////////////////////////////////////////////////////////////////////////////
// Filename: cameraclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "cameraclass.h"

void CameraClass::SetPosition(float x, float y, float z)
{
	m_positionX = x;
	m_positionY = y;
	m_positionZ = z;
}

void CameraClass::SetPosition(XMFLOAT3 position)
{
	m_positionX = position.x;
	m_positionY = position.y;
	m_positionZ = position.z;
}

void CameraClass::SetRotation(float pitch, float yaw, float roll)
{
	m_rotationX = pitch;
	m_rotationY = yaw;
	m_rotationZ = roll;
}

void CameraClass::SetRotation(XMFLOAT3 rotation)
{
	m_rotationX = rotation.x;
	m_rotationY = rotation.x;
	m_rotationZ = rotation.x;
}

void CameraClass::AddPosition(float rightLeft, float backForward, float upDown)
{
	m_storedLeftRight += rightLeft;
	m_storedBackForward += backForward;
	m_storedUpDown += upDown;
}

XMFLOAT3 CameraClass::GetPosition() const
{
	return XMFLOAT3(m_positionX, m_positionY, m_positionZ);
}

XMFLOAT3 CameraClass::GetRotation() const
{
	return XMFLOAT3(m_rotationX, m_rotationY, m_rotationZ);
}

void CameraClass::Render()
{
	const XMVECTOR position = { m_positionX, m_positionY, m_positionZ, 0.0f };
	constexpr float conv{ 0.0174532925f };
	// Create the rotation matrix from the yaw, pitch, and roll values.
	const XMMATRIX rotationMatrix = XMMatrixRotationRollPitchYaw(m_rotationX * conv, m_rotationY * conv, m_rotationZ * conv);
	XMVECTOR target = XMVector3TransformCoord(Camera::forward, rotationMatrix);
	target = XMVector3Normalize(target);

	const XMMATRIX YrotationMatrix = XMMatrixRotationY(m_rotationY * conv);
	const XMVECTOR camRight = XMVector3TransformCoord(Camera::right, YrotationMatrix);
	const XMVECTOR camForward = XMVector3TransformCoord(Camera::forward, YrotationMatrix);

	target = { m_positionX + target.m128_f32[0] , m_positionY + target.m128_f32[1], m_positionZ + target.m128_f32[2], 0.0f };
	m_viewMatrix = XMMatrixLookAtLH(position, target, Camera::up);

	const XMVECTOR addPos = camRight * m_storedLeftRight + camForward * m_storedBackForward;
	m_positionX += addPos.m128_f32[0];
	m_positionY += addPos.m128_f32[1];
	m_positionZ += addPos.m128_f32[2];
	m_positionY += m_storedUpDown;

	m_storedLeftRight = 0;
	m_storedBackForward = 0;
	m_storedUpDown = 0;
}

void CameraClass::RenderPreview(const XMVECTOR modelPosition)
{
	const XMVECTOR position = { m_positionX, m_positionY, m_positionZ, 0.0f };
	m_viewPreviewMatrix = XMMatrixLookAtLH(position, modelPosition, Camera::up);
}

void CameraClass::GetViewMatrix(XMMATRIX& viewMatrix)
{
	viewMatrix = m_viewMatrix;
}

void CameraClass::GetViewPreviewMatrix(XMMATRIX & viewPreviewMatrix)
{
	viewPreviewMatrix = m_viewPreviewMatrix;
}