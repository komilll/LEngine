#include "FXAAShader.h"

void FXAAShader::SetScreenBuffer(ID3D11ShaderResourceView * screenBuffer)
{
	m_screenBufferView = screenBuffer;
}

bool FXAAShader::CreateBufferAdditionals(ID3D11Device *& device)
{
	return BaseShaderClass::CreateBufferAdditionals(device);
}

bool FXAAShader::SetShaderParameters(ID3D11DeviceContext * deviceContext, XMMATRIX & worldMatrix, XMMATRIX & viewMatrix, XMMATRIX & projectionMatrix)
{
	if (!BaseShaderClass::SetShaderParameters(deviceContext, worldMatrix, viewMatrix, projectionMatrix))
		return false;

	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	/////// VERTEX BUFFERS ///////

	/////// PIXEL BUFFERS ///////
	//Texture buffer

	/////// RESOURCES ///////
	//Pixel shader resources
	unsigned int bufferNumber = 0;
	deviceContext->PSSetShaderResources(bufferNumber++, 1, &m_screenBufferView);
	return true;
}
