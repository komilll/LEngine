#include "ShadowShader.h"

void ShadowShader::SetShadowMapResource(ID3D11ShaderResourceView * resource)
{
	m_shadowMapResourceView = resource;
}

void ShadowShader::SetFrameResource(ID3D11ShaderResourceView * resource)
{
	m_frameResourceView = resource;
}

void ShadowShader::SetLightViewProjection(XMMATRIX& viewMatrix, XMMATRIX& projectionMatrix)
{
	m_viewMatrix = viewMatrix;
	m_projectionMatrix = projectionMatrix;
}

bool ShadowShader::CreateBufferAdditionals(ID3D11Device *& device)
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

bool ShadowShader::SetShaderParameters(ID3D11DeviceContext *deviceContext, XMMATRIX &worldMatrix, XMMATRIX &viewMatrix, XMMATRIX &projectionMatrix)
{
	if (!BaseShaderClass::SetShaderParameters(deviceContext, worldMatrix, viewMatrix, projectionMatrix))
		return false;

	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	LightBuffer* dataPtr2;
	unsigned int bufferNmber;

	///// VERTEX BUFFERS ///////
	//Lighting buffer
	result = deviceContext->Map(m_lightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
		return false;

	dataPtr2 = (LightBuffer*)mappedResource.pData;
	dataPtr2->viewMatrix = m_viewMatrix;
	dataPtr2->projectionMatrix = m_projectionMatrix;

	deviceContext->Unmap(m_lightBuffer, 0);
	int bufferNumber = 1;
	deviceContext->VSSetConstantBuffers(bufferNumber, 1, &m_lightBuffer);

	/////// PIXEL BUFFERS ///////
	/////// --- EMPTY --- ///////

	/////// RESOURCES ///////
	deviceContext->PSSetShaderResources(0, 1, &m_shadowMapResourceView);
	deviceContext->PSSetShaderResources(1, 1, &m_frameResourceView);

	return true;
}
