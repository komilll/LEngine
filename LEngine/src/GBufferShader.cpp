#include "GBufferShader.h"

void GBufferShader::ChangeTextureType(BufferType newType)
{
	m_bufferType = newType;
}

void GBufferShader::SetKernelValues(std::array<XMFLOAT3, 64> kernelVal)
{
	m_kernelValues = kernelVal;
}

void GBufferShader::SetNoiseValues(std::array<XMFLOAT2, 16> noiseVal)
{
	m_noiseValues = noiseVal;
}

void GBufferShader::SetRadiusSize(float radiusSize)
{
	m_radiusSize = radiusSize;
}

float *GBufferShader::GetRadiusSizeRef()
{
	return &m_radiusSize;
}

void GBufferShader::LoadPositionTexture(ID3D11ShaderResourceView * view)
{
	m_positionView = view;
}

void GBufferShader::LoadNormalTexture(ID3D11ShaderResourceView * view)
{
	m_normalView = view;
}

void GBufferShader::LoadNoiseTexture(ID3D11ShaderResourceView * view)
{
	m_noiseView = view;
}

bool GBufferShader::Initialize(ID3D11Device * device, HWND hwnd, WCHAR * vsFilename, WCHAR * psFilename, vertexInputType vertexInput, BufferType type)
{
	ChangeTextureType(type);
	return BaseShaderClass::Initialize(device, hwnd, vsFilename, psFilename, vertexInput);
}

bool GBufferShader::CreateBufferAdditionals(ID3D11Device *& device)
{
	BaseShaderClass::CreateBufferAdditionals(device);
	D3D11_BUFFER_DESC tempBufferDesc;

	tempBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	tempBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	tempBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	tempBufferDesc.MiscFlags = 0;
	tempBufferDesc.StructureByteStride = 0;

	switch (m_bufferType)
	{
		case GBufferShader::BufferType::SSAO_NOISE:
			tempBufferDesc.ByteWidth = sizeof(SSAONoiseBuffer);
			if (FAILED(device->CreateBuffer(&tempBufferDesc, NULL, &m_ssaoNoiseBuffer)))
				return false;

			m_buffers = { m_ssaoNoiseBuffer };
			break;
		case GBufferShader::BufferType::SSAO:
			if (m_ssaoBuffer == nullptr)
			{
				tempBufferDesc.ByteWidth = sizeof(SSAOBuffer);
				if (FAILED(device->CreateBuffer(&tempBufferDesc, NULL, &m_ssaoBuffer)))
					return false;
			m_buffers = { m_ssaoBuffer };
			}
			break;
	}

	return true;
}

bool GBufferShader::SetShaderParameters(ID3D11DeviceContext * deviceContext, XMMATRIX & worldMatrix, XMMATRIX & viewMatrix, XMMATRIX & projectionMatrix)
{
	if (!BaseShaderClass::SetShaderParameters(deviceContext, worldMatrix, viewMatrix, projectionMatrix))
		return false;

	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	
	/////// VERTEX BUFFERS ///////
	//SSAO noise buffer
	switch (m_bufferType)
	{
		case GBufferShader::BufferType::SSAO_NOISE:
			result = deviceContext->Map(m_ssaoNoiseBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
			if (FAILED(result))
				return false;

			SSAONoiseBuffer* ssaoNoiseBuffer = static_cast<SSAONoiseBuffer*>(mappedResource.pData);
			ssaoNoiseBuffer->noise = XMFLOAT2{ 0, 0 };
			ssaoNoiseBuffer->padding = XMFLOAT2{ 0, 0 };

			deviceContext->Unmap(m_ssaoNoiseBuffer, 0);
			deviceContext->VSSetConstantBuffers(1, 1, &m_ssaoNoiseBuffer);
			break;

	}

	/////// PIXEL BUFFERS ///////
	switch (m_bufferType)
	{
		case GBufferShader::BufferType::SSAO:
			result = deviceContext->Map(m_ssaoBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
			if (FAILED(result))
				return false;

			SSAOBuffer* ssaoBuffer = static_cast<SSAOBuffer*>(mappedResource.pData);
			ssaoBuffer->radiusSize = m_radiusSize;
			ssaoBuffer->bias = m_bias;
			ssaoBuffer->padding_2 = 0;
			ssaoBuffer->padding_3 = 0;
			ssaoBuffer->kernelValues = m_kernelValues;

			deviceContext->Unmap(m_ssaoBuffer, 0);
			deviceContext->PSSetConstantBuffers(0, 1, &m_ssaoBuffer);
			break;
	}

	/////// RESOURCES ///////
	switch (m_bufferType)
	{
		case GBufferShader::BufferType::SSAO:
			deviceContext->PSSetShaderResources(0, 1, &m_positionView);
			deviceContext->PSSetShaderResources(1, 1, &m_normalView);
			deviceContext->PSSetShaderResources(2, 1, &m_noiseView);
			break;
	}
	//deviceContext->PSSetShaderResources(0, 1, &m_shadowMapResourceView);
	return true;
}