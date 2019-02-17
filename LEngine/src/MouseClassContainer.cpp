#include "MouseClassContainer.h"

MouseClass * MouseClassContainer::GetMouse()
{
	return m_mouse;
}

void MouseClassContainer::SetMouse(MouseClass * mouse)
{
	m_mouse = mouse;
}

bool MouseClassContainer::InitializeMouse()
{
	if (!BaseShaderClass::Initialize(m_mouse->GetD3D()->GetDevice(), *m_mouse->GetD3D()->GetHWND(), UI_SHADER_VS, UI_SHADER_PS, BaseShaderClass::vertexInputType(GetInputNames(), GetInputFormats())))
		return false;

	return InitializeModelGeneric(m_mouse->GetD3D()->GetDevice(), ModelClass::ShapeSize::TRIANGLE, -0.01f, 0.01f, 0.01f, -0.01f);
}

bool MouseClassContainer::Render(ID3D11DeviceContext * deviceContext, int indexCount, XMMATRIX & worldMatrix, XMMATRIX & viewMatrix, XMMATRIX & projectionMatrix)
{
	float posX = 0;
	float posY = 0;
	m_mouse->GetMouseLocationScreenSpace(posX, posY);

	return UIBase::Render(deviceContext, indexCount, XMMATRIX(XMVECTOR{ posX, posY, 0,0 }, XMVECTOR{ 0, 0, 0, 0 }, XMVECTOR{ 0, 0, 0, 0 }, XMVECTOR{ 0, 0, 0, 0 }), viewMatrix, projectionMatrix);
}
