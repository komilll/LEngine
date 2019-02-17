#pragma once
#ifndef _MOUSE_CLASS_CONTAINER_H_
#define _MOUSE_CLASS_CONTAINER_H_

#include "UIBase.h"
#include "MouseClass.h"

///<summary>Contains all info about mouse; used for rendering and passing data info</summary>
class MouseClassContainer : public UIBase
{
public:
	
	MouseClass* GetMouse();
	void SetMouse(MouseClass* mouse);
	bool InitializeMouse();
	virtual bool Render(ID3D11DeviceContext *deviceContext, int indexCount, XMMATRIX &worldMatrix, XMMATRIX &viewMatrix, XMMATRIX &projectionMatrix) override;

private:
	MouseClass* m_mouse;
};
#endif // !_MOUSE_CLASS_CONTAINER_H_