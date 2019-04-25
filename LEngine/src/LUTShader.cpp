#include "LUTShader.h"

bool LUTShader::SetShaderParameters(ID3D11DeviceContext * deviceContext, XMMATRIX & worldMatrix, XMMATRIX & viewMatrix, XMMATRIX & projectionMatrix)
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
	deviceContext->PSSetShaderResources(bufferNumber++, 1, &m_screenResourceView);
	deviceContext->PSSetShaderResources(bufferNumber++, 1, &m_lutView);
	return true;
}

void LUTShader::SetLUT(ID3D11Device* device, const wchar_t * filename, bool isDDS)
{
	BaseShaderClass::LoadTexture(device, filename, m_lut, m_lutView, isDDS);
}