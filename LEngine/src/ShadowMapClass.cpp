#include "ShadowMapClass.h"

void ShadowMapClass::SetLightPosition(XMFLOAT3 lightPosition)
{
	m_lightPosition = lightPosition;
}

void ShadowMapClass::SetLightViewProjection(XMMATRIX& viewMatrix, XMMATRIX& projectionMatrix)
{
	m_viewMatrix = viewMatrix;
	m_projectionMatrix = projectionMatrix;
}

bool ShadowMapClass::CreateBufferAdditionals(ID3D11Device *& device)
{
	BaseShaderClass::CreateBufferAdditionals(device);
	//D3D11_BUFFER_DESC tempBufferDesc;

	//tempBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	//tempBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	//tempBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	//tempBufferDesc.MiscFlags = 0;
	//tempBufferDesc.StructureByteStride = 0;

	//tempBufferDesc.ByteWidth = sizeof(LightBuffer);
	//if (FAILED(device->CreateBuffer(&tempBufferDesc, NULL, &m_lightBuffer)))
	//	return false;

	//tempBufferDesc.ByteWidth = sizeof(LightBuffer);
	//if (FAILED(device->CreateBuffer(&tempBufferDesc, NULL, &m_lightMatrixBuffer)))
	//	return false;
	//
	//m_buffers = { m_lightBuffer, m_lightMatrixBuffer };

	return true;
}

bool ShadowMapClass::SetShaderParameters(ID3D11DeviceContext *deviceContext, XMMATRIX &worldMatrix, XMMATRIX &viewMatrix, XMMATRIX &projectionMatrix)
{
	if (!BaseShaderClass::SetShaderParameters(deviceContext, worldMatrix, viewMatrix, projectionMatrix))
		return false;

	//HRESULT result;
	//D3D11_MAPPED_SUBRESOURCE mappedResource;
	//LightBuffer* dataPtr2;
	//LightMatrixBuffer* dataPtr3;
	//unsigned int bufferNmber;

	/////// VERTEX BUFFERS ///////
	//Lighting buffer
	//result = deviceContext->Map(m_lightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	//if (FAILED(result))
	//	return false;

	//dataPtr2 = (LightBuffer*)mappedResource.pData;
	//dataPtr2->lightPosition = m_lightPosition;
	//dataPtr2->padding = 0;

	//deviceContext->Unmap(m_lightBuffer, 0);
	//int bufferNumber = 1;
	//deviceContext->VSSetConstantBuffers(bufferNumber, 1, &m_lightBuffer);
	//
	////Lighting matrix buffer
	//result = deviceContext->Map(m_lightMatrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	//if (FAILED(result))
	//	return false;

	//dataPtr3 = (LightMatrixBuffer*)mappedResource.pData;
	//dataPtr3->viewMatrix = m_viewMatrix;
	//dataPtr3->projectionMatrix = m_projectionMatrix;

	//deviceContext->Unmap(m_lightMatrixBuffer, 0);
	//bufferNumber = 2;
	//deviceContext->VSSetConstantBuffers(bufferNumber, 1, &m_lightMatrixBuffer);

	/////// PIXEL BUFFERS ///////
	/////// --- EMPTY --- ///////

	/////// RESOURCES ///////
	///// --- EMPTY --- /////

	return true;
}
