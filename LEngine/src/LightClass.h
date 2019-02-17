#pragma once
#ifndef _LIGHTCLASS_H_
#define _LIGHTCLASS_H_

#include <DirectXMath.h>

using namespace DirectX;

class LightClass
{
public:
	LightClass();

	void SetPosition(float x, float y, float z);
	void SetPosition(XMFLOAT3 position);
	XMFLOAT3 GetPosition();

	void SetLookAt(float x, float y, float z);
	void SetLookAt(XMFLOAT3 lookAt);

	void GenerateViewMatrix();
	void GenerateProjectionMatrix(float screenDepth, float screenNear);

	void GetViewMatrix(XMMATRIX &viewMatrix);
	void GetProjectionMatrix(XMMATRIX &projectionMatrix);

private:
	XMFLOAT3 m_position{ 0,0,0 };
	XMFLOAT3 m_lookAt{ 0,0,0 };

	XMMATRIX m_viewMatrix;
	XMMATRIX m_projectionMatrix;
};

#endif // !_LIGHTCLASS_H_