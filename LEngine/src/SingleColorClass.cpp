#include "SingleColorClass.h"

void SingleColorClass::SetLightPosition(XMFLOAT3 lightPosition)
{
	m_lightPosition = lightPosition;
}

void SingleColorClass::SetLightViewProjection(XMMATRIX & viewMatrix, XMMATRIX & projectionMatrix)
{
	m_lightViewMatrix = viewMatrix;
	m_lightProjectionMatrix = projectionMatrix;
}

bool SingleColorClass::CreateBufferAdditionals(ID3D11Device *& device)
{
	BaseShaderClass::CreateBufferAdditionals(device);
	D3D11_BUFFER_DESC tempBufferDesc;

	tempBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	tempBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	tempBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	tempBufferDesc.MiscFlags = 0;
	tempBufferDesc.StructureByteStride = 0;

	tempBufferDesc.ByteWidth = sizeof(LightBuffer);
	if (FAILED(device->CreateBuffer(&tempBufferDesc, NULL, &m_lightBuffer)))
		return false;

	m_buffers = { m_lightBuffer };

	return true;
}

bool SingleColorClass::SetShaderParameters(ID3D11DeviceContext *deviceContext, XMMATRIX &worldMatrix, XMMATRIX &viewMatrix, XMMATRIX &projectionMatrix)
{
	if (BaseShaderClass::SetShaderParameters(deviceContext, worldMatrix, viewMatrix, projectionMatrix) == false)
		return false;

	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	/////// VERTEX BUFFERS ///////
	//Lighting buffer
	result = deviceContext->Map(m_lightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
		return false;

	LightBuffer* dataPtr2 = static_cast<LightBuffer*>(mappedResource.pData);
	dataPtr2->lightPosition = m_lightPosition;
	dataPtr2->lightViewMatrix = m_lightViewMatrix;
	dataPtr2->lightProjectionMatrix = m_lightProjectionMatrix;
	dataPtr2->padding = 0;

	deviceContext->Unmap(m_lightBuffer, 0);
	unsigned int bufferNumber = 1;
	deviceContext->VSSetConstantBuffers(bufferNumber++, 1, &m_lightBuffer);

	/////// PIXEL BUFFERS ///////
	/////// --- EMPTY --- ///////

	/////// RESOURCES ///////
	deviceContext->PSSetShaderResources(0, 1, &m_shadowMapResourceView);
	return true;
}

bool SingleColorClass::CreateSamplerState(ID3D11Device * device)
{
	//Base sampler state that is used as general-purpose is creating here
	D3D11_SAMPLER_DESC samplerDesc;

	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.BorderColor[0] = 0;
	samplerDesc.BorderColor[1] = 0;
	samplerDesc.BorderColor[2] = 0;
	samplerDesc.BorderColor[3] = 0;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	// Create the texture sampler state.
	HRESULT result = device->CreateSamplerState(&samplerDesc, &m_samplerState);
	if (FAILED(result))
		return false;

	//This sampler holds first slot (usually) in samplers list
	AddSampler(false, 0, 1, m_samplerState);

	return true;
}