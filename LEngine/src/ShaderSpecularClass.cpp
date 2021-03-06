#include "ShaderSpecularClass.h"

bool ShaderSpecularClass::CreateBufferAdditionals(ID3D11Device * &device)
{
	BaseShaderClass::CreateBufferAdditionals(device);
	D3D11_BUFFER_DESC tempBufferDesc;

	tempBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	tempBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	tempBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	tempBufferDesc.MiscFlags = 0;
	tempBufferDesc.StructureByteStride = 0;

	tempBufferDesc.ByteWidth = sizeof(LightingBufferType);
	if (FAILED(device->CreateBuffer(&tempBufferDesc, NULL, &m_lightingBuffer)))
		return false;

	tempBufferDesc.ByteWidth = sizeof(CameraBufferType);
	if (FAILED(device->CreateBuffer(&tempBufferDesc, NULL, &m_cameraBuffer)))
		return false;

	m_buffers = {m_lightingBuffer, m_cameraBuffer};

	return true;
}

bool ShaderSpecularClass::SetShaderParameters(ID3D11DeviceContext *deviceContext, XMMATRIX &worldMatrix, XMMATRIX &viewMatrix, XMMATRIX &projectionMatrix)
{
	if (!BaseShaderClass::SetShaderParameters(deviceContext, worldMatrix, viewMatrix, projectionMatrix))
		return false;

	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	unsigned int bufferNumber;

	/////// VERTEX BUFFERS ///////
	//Camera buffer
	result = deviceContext->Map(m_cameraBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
		return false;

	CameraBufferType* dataPtr3 = static_cast<CameraBufferType*>(mappedResource.pData);
	dataPtr3->cameraDirection = m_cameraPosition;
	dataPtr3->padding = 0;

	deviceContext->Unmap(m_cameraBuffer, 0);
	bufferNumber = 1;
	deviceContext->VSSetConstantBuffers(bufferNumber++, 1, &m_cameraBuffer);

	/////// PIXEL BUFFERS ///////
	//Lighting buffer
	result = deviceContext->Map(m_lightingBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
		return false;

	LightingBufferType* dataPtr2 = static_cast<LightingBufferType*>(mappedResource.pData);
	dataPtr2->direction = m_lightDirection;
	dataPtr2->padding = 0;

	deviceContext->Unmap(m_lightingBuffer, 0);
	bufferNumber = 0;
	deviceContext->PSSetConstantBuffers(bufferNumber++, 1, &m_lightingBuffer);

	/////// RESOURCES ///////
	//Pixel shader resources
	bufferNumber = 0;
	deviceContext->PSSetShaderResources(bufferNumber++, 1, &m_diffuseTextureView);
	deviceContext->PSSetShaderResources(bufferNumber++, 1, &m_normalTextureView);
	return true;
}