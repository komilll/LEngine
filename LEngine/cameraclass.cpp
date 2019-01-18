////////////////////////////////////////////////////////////////////////////////
// Filename: cameraclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "cameraclass.h"


CameraClass::CameraClass()
{
	m_positionX = 0.0f;
	m_positionY = 0.0f;
	m_positionZ = 0.0f;

	m_rotationX = 0.0f;
	m_rotationY = 0.0f;
	m_rotationZ = 0.0f;

	m_storedLeftRight = 0;
	m_storedBackForward = 0;
	m_storedUpDown = 0;
}


CameraClass::CameraClass(const CameraClass& other)
{
}


CameraClass::~CameraClass()
{
}


void CameraClass::SetPosition(float x, float y, float z)
{
	m_positionX = x;
	m_positionY = y;
	m_positionZ = z;
	return;
}


void CameraClass::SetRotation(float x, float y, float z)
{
	m_rotationX = x;
	m_rotationY = y;
	m_rotationZ = z;
	return;
}

void CameraClass::AddPosition(float rightLeft, float backForward, float upDown)
{
	m_storedLeftRight += rightLeft;
	m_storedBackForward += backForward;
	m_storedUpDown += upDown;
}


XMFLOAT3 CameraClass::GetPosition()
{
	return XMFLOAT3(m_positionX, m_positionY, m_positionZ);
}


XMFLOAT3 CameraClass::GetRotation()
{
	return XMFLOAT3(m_rotationX, m_rotationY, m_rotationZ);
}


void CameraClass::Render()
{
	float yaw, pitch, roll;
	XMMATRIX rotationMatrix;

	XMVECTOR up = { 0.0f, 1.0f, 0.0f, 0.0f };
	XMVECTOR position = { m_positionX, m_positionY, m_positionZ, 0.0f };

	XMVECTOR forward = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
	XMVECTOR right = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
	XMVECTOR camForward = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
	XMVECTOR camRight = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);

	XMVECTOR target{ 0,0,0,0 };

	// Set the yaw (Y axis), pitch (X axis), and roll (Z axis) rotations in radians.
	pitch = m_rotationX * 0.0174532925f;
	yaw = m_rotationY * 0.0174532925f;
	roll = m_rotationZ * 0.0174532925f;

	// Create the rotation matrix from the yaw, pitch, and roll values.
	rotationMatrix = XMMatrixRotationRollPitchYaw(pitch, yaw, roll);
	target = XMVector3TransformCoord(forward, rotationMatrix);
	target = XMVector3Normalize(target);

	XMMATRIX YrotationMatrix;
	YrotationMatrix = XMMatrixRotationY(yaw);

	camRight = XMVector3TransformCoord(right, YrotationMatrix);
	camForward = XMVector3TransformCoord(forward, YrotationMatrix);

	target = { m_positionX + target.m128_f32[0] , m_positionY + target.m128_f32[1], m_positionZ + target.m128_f32[2], 0.0f };

	m_viewMatrix = XMMatrixLookAtLH(position, target, up);

	XMVECTOR addPos = camRight * m_storedLeftRight + camForward * m_storedBackForward;
	m_positionX += addPos.m128_f32[0];
	m_positionY += addPos.m128_f32[1];
	m_positionZ += addPos.m128_f32[2];
	m_positionY += m_storedUpDown;

	m_storedLeftRight = 0;
	m_storedBackForward = 0;
	m_storedUpDown = 0;

	return;
}


void CameraClass::GetViewMatrix(XMMATRIX& viewMatrix)
{
	viewMatrix = m_viewMatrix;
	return;
}