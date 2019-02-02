#pragma once
#ifndef _UITEXTUREPREVIEW_H_
#define _UITEXTUREPREVIEW_H_

#include "UIBase.h"
#include "d3dclass.h"

class UITexturePreview : public UIBase
{
public:
	///<summary> Initialize shape: Square </summary>
	bool Initialize(D3DClass* d3d, float centerX, float centerY, float size, wchar_t* texttureFilename = L"");

	virtual bool SetShaderParameters(ID3D11DeviceContext *deviceContext, XMMATRIX &worldMatrix, XMMATRIX &viewMatrix, XMMATRIX &projectionMatrix) override;

private:
	ID3D11Resource* m_texture;
	ID3D11ShaderResourceView* m_textureView;
};

#endif // !_UITEXTUREPREVIEW_H_