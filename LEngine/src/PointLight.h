#pragma once
#ifndef _POINTLIGHT_H_
#define _POINTLIGHT_H_

#include "modelClass.h"

class PointLight : public ModelClass
{
public:
	void SetParams(XMFLOAT3 position, float radius, XMFLOAT3 color, float strength);

	void SetRadius(float radius);
	void SetColor(XMFLOAT3 color);
	void SetStrength(float strength);

	XMFLOAT4 GetPositionWithRadius() const;
	XMFLOAT4 GetColorWithStrength() const;

private:
	XMFLOAT4 m_positionWithRadius;
	XMFLOAT4 m_colorWithStrength;
};
#endif