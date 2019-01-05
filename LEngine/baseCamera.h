#pragma once
#ifndef _BASE_CAMERA_H_
#define _BASE_CAMERA_H_

//////////////
// INCLUDES //
//////////////
#include <DirectXMath.h>

using namespace DirectX;
////////////////////////////////////////////////////////////////////////////////
// Class name: CameraClass
////////////////////////////////////////////////////////////////////////////////
class BaseCamera
{
public:
	BaseCamera();
	BaseCamera(const BaseCamera&);
	~BaseCamera();

	void SetPosition(float, float, float);
	void SetRotation(float, float, float);

	XMFLOAT3 GetPosition();
	XMFLOAT3 GetRotation();

	void Render();
	void GetViewMatrix(XMMATRIX&);

private:
	float m_positionX, m_positionY, m_positionZ;
	float m_rotationX, m_rotationY, m_rotationZ;
	XMMATRIX m_viewMatrix;
};
#endif