#include "VignetteShader.h"

bool VignetteShader::SetShaderParameters(ID3D11DeviceContext * deviceContext, XMMATRIX & worldMatrix, XMMATRIX & viewMatrix, XMMATRIX & projectionMatrix)
{
	if (BaseShaderClass::SetShaderParameters(deviceContext, worldMatrix, viewMatrix, projectionMatrix) == false)
		return false;

	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	unsigned int bufferNumber;

	/////// VERTEX BUFFERS ///////

	/////// PIXEL BUFFERS ///////
	//Texture buffer

	/////// RESOURCES ///////
	//Pixel shader resources
	bufferNumber = 0;
	deviceContext->PSSetShaderResources(bufferNumber++, 1, &m_vignetteResourceView);
	return true;
}