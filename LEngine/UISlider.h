#pragma once
#ifndef _UI_SLIDER_H_
#define _UI_SLIDER_H_

#include "UIBase.h"
#include "TextEngine.h"

class UISlider : public UIBase
{
public:

	virtual bool Render(ID3D11DeviceContext *deviceContext, int indexCount, XMMATRIX &worldMatrix, XMMATRIX &viewMatrix, XMMATRIX &projectionMatrix) override;
	virtual bool MouseOnArea(MouseClass* mouse) override;

	///<summary> Create slider of given range </summary>
	bool Initialize(D3DClass *d3d, float positionMinX, float positionMaxX, float positionY, float height);
	void CreateTextArea(TextEngine::FontData *text);
	void ChangeSliderValue(MouseClass * mouse);
	bool IsChanging();
	void StartUsing();
	void EndUsing();
	std::function<void(float)> EventOnChangeValue;

private:
	float clamp(float n, float lower, float upper);

private:
	ModelClass* m_modelSlider;
	D3DClass* m_D3D;
	TextEngine* m_textEngine = nullptr;
	int m_textIndex = -1;

	float m_sliderVal = 0;
	bool m_isChanging = false;

	float m_minX = 0, m_maxX = 0;
	float m_minY = 0, m_maxY = 0;
	float m_mousePosX, m_mousePosY;
	const float k_sliderWidth = 0.01f;
	const float k_sliderHeight = 0.007f;
};

#endif // !_UI_SLIDER_H_