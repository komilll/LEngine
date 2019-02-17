#include "SkyboxShaderClass.h"

bool SkyboxShaderClass::CreateBufferAdditionals(ID3D11Device * &device)
{
	BaseShaderClass::CreateBufferAdditionals(device);
	D3D11_BUFFER_DESC tempBufferDesc;

	tempBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	tempBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	tempBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	tempBufferDesc.MiscFlags = 0;
	tempBufferDesc.StructureByteStride = 0;

	tempBufferDesc.ByteWidth = sizeof(UpVectorBuffer);
	if (FAILED(device->CreateBuffer(&tempBufferDesc, NULL, &m_upVectorBuffer)))
		return false;

	m_buffers = { m_upVectorBuffer };

	return true;
}

bool SkyboxShaderClass::SetShaderParameters(ID3D11DeviceContext *deviceContext, XMMATRIX &worldMatrix, XMMATRIX &viewMatrix, XMMATRIX &projectionMatrix)
{
	if (BaseShaderClass::SetShaderParameters(deviceContext, worldMatrix, viewMatrix, projectionMatrix) == false)
		return false;

	/////// ADDITIONAL BUFFERS ///////
	if (m_skyboxType == SkyboxType::CONV_DIFFUSE)
	{
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		UpVectorBuffer* dataPtr;
		unsigned int bufferNumber;

		HRESULT result = deviceContext->Map(m_upVectorBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		if (FAILED(result))
			return false;

		dataPtr = (UpVectorBuffer*)mappedResource.pData;
		dataPtr->upVector = m_upVector;
		dataPtr->padding = 0;

		deviceContext->Unmap(m_upVectorBuffer, 0);
		bufferNumber = 0;
		deviceContext->PSSetConstantBuffers(bufferNumber, 1, &m_upVectorBuffer);
	}

	/////// RESOURCES ///////
	//Pixel shader resources
	deviceContext->PSSetShaderResources(0, 1, &m_skyboxTextureView);
	return true;
}

void SkyboxShaderClass::SetType(SkyboxType type)
{
	m_skyboxType = type;
}

void SkyboxShaderClass::SetUpVector(XMFLOAT3 vector)
{
	m_upVector = vector;
}
