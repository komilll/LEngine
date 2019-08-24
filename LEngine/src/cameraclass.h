#ifndef _CAMERACLASS_H_
#define _CAMERACLASS_H_

#include <DirectXMath.h>
using namespace DirectX;

namespace Camera
{
	constexpr XMVECTOR up{ 0.0f, 1.0f, 0.0f, 0.0f };
	constexpr XMVECTOR right{ 1.0f, 0.0f, 0.0f, 0.0f };
	constexpr XMVECTOR forward{ 0.0f, 0.0f, 1.0f, 0.0f };
};

class CameraClass
{
public:
	void SetPosition(float x, float y, float z);
	void SetPosition(XMFLOAT3 position);
	void AddPosition(float rightLeft, float backForward, float upDown);
	XMFLOAT3 GetPosition() const;

	void SetRotation(float pitch, float yaw, float roll);
	void SetRotation(XMFLOAT3 rotation);
	XMFLOAT3 GetRotation() const;

	void Render();
	void RenderPreview(const XMVECTOR modelPosition);
	void GetViewMatrix(XMMATRIX& viewMatrix);
	void GetViewPreviewMatrix(XMMATRIX& viewPreviewMatrix);

	XMFLOAT3 GetForwardVector() const;
private:
	float m_positionX{ 0.0f };
	float m_positionY{ 0.0f };
	float m_positionZ{ 0.0f };
	float m_rotationX{ 0.0f };
	float m_rotationY{ 0.0f };
	float m_rotationZ{ 0.0f };
	XMMATRIX m_viewMatrix;
	XMMATRIX m_viewPreviewMatrix;

	float m_storedLeftRight{ 0.0f };
	float m_storedBackForward{ 0.0f };
	float m_storedUpDown{ 0.0f };
};

#endif