#pragma once
#ifndef _UI_SLIDER_H_
#define _UI_SLIDER_H_

#include "UIBase.h"

class UISlider : public UIBase
{
public:

	virtual bool Render(ID3D11DeviceContext *deviceContext, int indexCount, XMMATRIX &worldMatrix, XMMATRIX &viewMatrix, XMMATRIX &projectionMatrix) override;
	virtual bool MouseOnArea(MouseClass* mouse) override;

	///<summary> Create slider of given range </summary>
	bool Initialize(D3DClass *d3d, float positionMinX, float positionMaxX, float positionY, float height);
	void ChangeSliderValue();

private:
	ModelClass* m_modelSlider;
	D3DClass* m_D3D;
	float m_sliderVal = 0;

	float m_minX = 0, m_maxX = 0;
	float m_minY = 0, m_maxY = 0;
	float m_mousePosX, m_mousePosY;
	const float k_sliderWidth = 0.01f;
	const float k_sliderHeight = 0.007f;
};

#endif // !_UI_SLIDER_H_