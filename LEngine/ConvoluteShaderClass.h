#pragma once
#ifndef _CONVOLUTESHADERCLASS_H_
#define _CONVOLUTESHADERCLASS_H_

#include "BaseShaderClass.h"

class ConvoluteShaderClass : public BaseShaderClass
{
public:
	ID3D11Resource *m_resource;
	ID3D11ShaderResourceView* m_resourceView;

	void SetTextureResourceView(ID3D11ShaderResourceView *& shaderResource);
};

#endif // !_CONVOLUTESHADERCLASS_H_