#include "BloomShaderClass.h"

bool BloomShaderClass::CreateBufferAdditionals(ID3D11Device *& device)
{
	BaseShaderClass::CreateBufferAdditionals(device);
	D3D11_BUFFER_DESC tempBufferDesc;

	tempBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	tempBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	tempBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	tempBufferDesc.MiscFlags = 0;
	tempBufferDesc.StructureByteStride = 0;

	tempBufferDesc.ByteWidth = sizeof(BloomIntensity);
	if (FAILED(device->CreateBuffer(&tempBufferDesc, NULL, &m_bloomIntensityBuffer)))
		return false;

	m_buffers = { m_bloomIntensityBuffer };

	return true;
}

bool BloomShaderClass::SetShaderParameters(ID3D11DeviceContext * deviceContext, XMMATRIX & worldMatrix, XMMATRIX & viewMatrix, XMMATRIX & projectionMatrix)
{
	if (BaseShaderClass::SetShaderParameters(deviceContext, worldMatrix, viewMatrix, projectionMatrix) == false)
		return false;

	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	BloomIntensity* dataPtr2;
	unsigned int bufferNmber;

	/////// VERTEX BUFFERS ///////

	/////// PIXEL BUFFERS ///////
	result = deviceContext->Map(m_bloomIntensityBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
		return false;

	dataPtr2 = (BloomIntensity*)mappedResource.pData;
	dataPtr2->intensity = m_bloomIntensity;
	dataPtr2->padding = 0.0f;

	deviceContext->Unmap(m_bloomIntensityBuffer, 0);
	deviceContext->PSSetConstantBuffers(0, 1, &m_bloomIntensityBuffer);
	/////// RESOURCES ///////
	deviceContext->PSSetShaderResources(0, 1, &m_bloomTextureView);
	return true;
}

void BloomShaderClass::SetBloomIntensity(XMFLOAT3 bloomIntensity)
{
	m_bloomIntensity = bloomIntensity;
}
