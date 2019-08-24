#pragma once
#ifndef _POINTLIGHT_H_
#define _POINTLIGHT_H_

#include "modelClass.h"
#include "ShaderPBRGenerated.h"

class PointLight : public ModelClass
{
public:
	PointLight();
	void UpdateShader();
	void SetParams(XMFLOAT3 position, float radius, XMFLOAT3 color, float strength);

	void SetRadius(float radius);
	void SetColor(XMFLOAT3 color);
	void SetStrength(float strength);
	virtual void SetPosition(float x, float y, float z) override final;
	virtual void SetPosition(XMFLOAT3 position) override final;
	virtual std::string GetSaveData() const override final;

	float* GetColorRef();
	float* GetStrengthRef();
	float* GetRadiusRef();

	XMFLOAT4 GetPositionWithRadius() const;
	XMFLOAT4 GetColorWithStrength() const;

private:
	XMFLOAT4 m_positionWithRadius;
	XMFLOAT4 m_colorWithStrength;
	int m_index = -1;
};
#endif