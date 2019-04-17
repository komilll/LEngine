#include "BloomShaderClass.h"

bool BloomShaderClass::SetShaderParameters(ID3D11DeviceContext * deviceContext, XMMATRIX & worldMatrix, XMMATRIX & viewMatrix, XMMATRIX & projectionMatrix)
{
	if (BaseShaderClass::SetShaderParameters(deviceContext, worldMatrix, viewMatrix, projectionMatrix) == false)
		return false;

	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	unsigned int bufferNmber;

	/////// VERTEX BUFFERS ///////

	/////// PIXEL BUFFERS ///////

	/////// RESOURCES ///////
	deviceContext->PSSetShaderResources(0, 1, &m_bloomTextureView);
	return true;
}