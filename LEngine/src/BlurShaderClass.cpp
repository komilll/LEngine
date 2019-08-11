#include "BlurShaderClass.h"

void BlurShaderClass::SetTextureSize(const float size)
{
	m_size = size;
}

void BlurShaderClass::SetTextureResourceView(ID3D11ShaderResourceView *const shaderResource)
{
	m_shaderResource = shaderResource;
}

void BlurShaderClass::SetWeights(std::array<float, k_numberOfWeights> weights)
{
	m_weights = weights;
}

bool BlurShaderClass::CreateBufferAdditionals(ID3D11Device *& device)
{
	BaseShaderClass::CreateBufferAdditionals(device);
	D3D11_BUFFER_DESC tempBufferDesc;

	tempBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	tempBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	tempBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	tempBufferDesc.MiscFlags = 0;
	tempBufferDesc.StructureByteStride = 0;

	tempBufferDesc.ByteWidth = sizeof(ScreenSizeBuffer);
	if (FAILED(device->CreateBuffer(&tempBufferDesc, NULL, &m_screenSizeBuffer)))
		return false;

	tempBufferDesc.ByteWidth = sizeof(BlurWeightsBuffer);
	if (FAILED(device->CreateBuffer(&tempBufferDesc, NULL, &m_weightsBuffer)))
		return false;

	m_buffers = { m_screenSizeBuffer, m_weightsBuffer };

	return true;
}

bool BlurShaderClass::SetShaderParameters(ID3D11DeviceContext *deviceContext, XMMATRIX &worldMatrix, XMMATRIX &viewMatrix, XMMATRIX &projectionMatrix)
{
	//if (BaseShaderClass::SetShaderParameters(deviceContext, worldMatrix, viewMatrix, projectionMatrix) == false)
	//	return false;
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;	

	/////// VERTEX BUFFERS ///////
	//Screen size buffer
	result = deviceContext->Map(m_screenSizeBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
		return false;

	ScreenSizeBuffer* dataPtr2 = static_cast<ScreenSizeBuffer*>(mappedResource.pData);
	dataPtr2->size = m_size;
	dataPtr2->padding_1 = 0.0f;
	dataPtr2->padding_2 = 0.0f;
	dataPtr2->padding_3 = 0.0f;

	deviceContext->Unmap(m_screenSizeBuffer, 0);
	deviceContext->VSSetConstantBuffers(0, 1, &m_screenSizeBuffer);

	/////// PIXEL BUFFERS ///////
	result = deviceContext->Map(m_weightsBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
		return false;

	BlurWeightsBuffer* dataPtr3 = static_cast<BlurWeightsBuffer*>(mappedResource.pData);
	dataPtr3->weights = XMFLOAT4{ m_weights[0], m_weights[1], m_weights[2], m_weights[3] };
	dataPtr3->lastWeightAndpadding = XMFLOAT4{ m_weights[4], -1, -1, -1 };

	deviceContext->Unmap(m_weightsBuffer, 0);
	deviceContext->PSSetConstantBuffers(0, 1, &m_weightsBuffer);

	/////// RESOURCES ///////
	//Pixel shader resources
	deviceContext->PSSetShaderResources(0, 1, &m_shaderResource);
	return true;
}

bool BlurShaderClass::CreateSamplerState(ID3D11Device * device)
{
	D3D11_SAMPLER_DESC samplerDesc;

	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
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
	const HRESULT result = device->CreateSamplerState(&samplerDesc, &m_samplerState);
	if (FAILED(result))
		return false;

	AddSampler(false, 0, 1, m_samplerState);

	return true;
}