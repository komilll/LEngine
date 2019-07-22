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

bool SkyboxShaderClass::CreateSamplerState(ID3D11Device * device)
{
	//Base sampler state that is used as general-purpose is creating here
	D3D11_SAMPLER_DESC samplerDesc;

	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
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

bool SkyboxShaderClass::SetShaderParameters(ID3D11DeviceContext *deviceContext, XMMATRIX &worldMatrix, XMMATRIX &viewMatrix, XMMATRIX &projectionMatrix)
{
	if (!BaseShaderClass::SetShaderParameters(deviceContext, worldMatrix, viewMatrix, projectionMatrix))
		return false;

	/////// ADDITIONAL BUFFERS ///////
	if (m_skyboxType == SkyboxType::CONV_DIFFUSE)
	{
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		unsigned int bufferNumber;

		HRESULT result = deviceContext->Map(m_upVectorBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		if (FAILED(result))
			return false;

		UpVectorBuffer* dataPtr = static_cast<UpVectorBuffer*>(mappedResource.pData);
		dataPtr->upVector = m_upVector;
		dataPtr->rightVectorDirection = m_rightVectorDirection;

		deviceContext->Unmap(m_upVectorBuffer, 0);
		bufferNumber = 0;
		deviceContext->PSSetConstantBuffers(bufferNumber, 1, &m_upVectorBuffer);
	}
	else if (m_skyboxType == SkyboxType::ENVIRO)
	{
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		unsigned int bufferNumber;

		HRESULT result = deviceContext->Map(m_upVectorBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		if (FAILED(result))
			return false;

		UpVectorBuffer* dataPtr = static_cast<UpVectorBuffer*>(mappedResource.pData);
		dataPtr->upVector = m_upVector;
		dataPtr->rightVectorDirection = m_roughness;

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

void SkyboxShaderClass::SetRightVector(float rightVectorSign)
{
	m_rightVectorDirection = rightVectorSign >= 0 ? 1 : -1;
}

void SkyboxShaderClass::SetRoughness(float roughness)
{
	m_roughness = roughness;
}