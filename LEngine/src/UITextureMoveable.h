#pragma once
#ifndef _UITEXTUREMOVEABLE_H_
#define _UITEXTUREMOVEABLE_H_

#include "UIBase.h"
///<summary> Preview texture on block - can be moved around</summary>
class UITextureMoveable : public UIBase
{
public:
	virtual bool SetShaderParameters(ID3D11DeviceContext *deviceContext, XMMATRIX &worldMatrix, XMMATRIX &viewMatrix, XMMATRIX &projectionMatrix) override;
	void LoadTexture(ID3D11Device* device, wchar_t * name, bool isDDS);
	void LoadTexture(ID3D11Device* device, ID3D11ShaderResourceView* resourceView);

private:
	ID3D11Resource* m_texture;
	ID3D11ShaderResourceView* m_textureView;
};
#endif // !_UITEXTURE_H_