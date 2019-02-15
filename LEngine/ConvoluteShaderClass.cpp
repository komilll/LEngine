#include "ConvoluteShaderClass.h"

void ConvoluteShaderClass::SetTextureResourceView(ID3D11ShaderResourceView *& shaderResource)
{
	m_resourceView = shaderResource;
}
