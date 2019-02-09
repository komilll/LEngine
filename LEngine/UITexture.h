#pragma once
#ifndef _UITEXTURE_H_
#define _UITEXTURE_H_

#include "UIBase.h"
///<summary> Preview only one texture passed on initialization </summary>
class UITexture : public UIBase
{
public:
	///<summary> Initialize shape: Square </summary>
	bool Initialize(D3DClass* d3d, float centerX, float centerY, float size, wchar_t* textureFilename = L"");
	///<summary> Initialize shape: Rectangle/Triangle </summary>
	bool Initialize(D3DClass* d3d, ModelClass::ShapeSize shape, float left, float right, float top, float bottom);
	void BindTexture(ID3D11ShaderResourceView *& textureView);

	virtual bool SetShaderParameters(ID3D11DeviceContext *deviceContext, XMMATRIX &worldMatrix, XMMATRIX &viewMatrix, XMMATRIX &projectionMatrix) override;
	ModelClass* GetModel();
	ID3D11ShaderResourceView*& GetShaderResourceView();

private:
	ID3D11Resource* m_texture;
	ID3D11ShaderResourceView* m_textureView;
};
#endif // !_UITEXTURE_H_