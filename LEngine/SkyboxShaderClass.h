#pragma once
#ifndef _SKYBOXSHADERCLASS_H_
#define _SKYBOXSHADERCLASS_H_

#include "BaseShaderClass.h"
#include <ScreenGrab.h>

class SkyboxShaderClass : public BaseShaderClass
{
public:
	ID3D11Resource* m_skyboxTexture;
	ID3D11ShaderResourceView* m_skyboxTextureView;
	virtual bool SetShaderParameters(ID3D11DeviceContext*, XMMATRIX&, XMMATRIX&, XMMATRIX&) override;

	bool CreateColorSkybox(ID3D11Device* device, int width, int height);
};


#endif // !_SKYBOXSHADERCLASS_H_