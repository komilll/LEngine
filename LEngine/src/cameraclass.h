////////////////////////////////////////////////////////////////////////////////
// Filename: cameraclass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _CAMERACLASS_H_
#define _CAMERACLASS_H_


//////////////
// INCLUDES //
//////////////
#include <DirectXMath.h>
using namespace DirectX;

////////////////////////////////////////////////////////////////////////////////
// Class name: CameraClass
////////////////////////////////////////////////////////////////////////////////
class CameraClass
{
public:
	CameraClass();
	CameraClass(const CameraClass&);
	~CameraClass();

	void SetPosition(float, float, float);
	void SetRotation(float, float, float);

	void AddPosition(float rightLeft, float backForward, float upDown);

	XMFLOAT3 GetPosition();
	XMFLOAT3 GetRotation();

	void Render();
	void RenderPreview(const XMVECTOR modelPosition);
	void GetViewMatrix(XMMATRIX& viewMatrix);
	void GetViewPreviewMatrix(XMMATRIX& viewPreviewMatrix);

private:
	float m_positionX, m_positionY, m_positionZ;
	float m_rotationX, m_rotationY, m_rotationZ;
	XMMATRIX m_viewMatrix;
	XMMATRIX m_viewPreviewMatrix;

	float m_storedLeftRight, m_storedBackForward, m_storedUpDown;
};

#endif