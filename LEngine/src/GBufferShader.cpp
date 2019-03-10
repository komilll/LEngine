#include "GBufferShader.h"

void GBufferShader::ChangeTextureType(BufferType newType)
{
	m_bufferType = newType;
}

void GBufferShader::SetKernelValues(XMFLOAT3 kernelVal[64])
{
	for (int i = 0; i < 64; i++)
		kernelValues[i] = kernelVal[i];
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
		//case GBufferShader::BufferType::POSITION:
		//	tempBufferDesc.ByteWidth = sizeof(PositionBuffer);
		//	if (FAILED(device->CreateBuffer(&tempBufferDesc, NULL, &m_positionBuffer)))
		//		return false;

		//	m_buffers = { m_positionBuffer };
		//	break;

		//case GBufferShader::BufferType::NORMAL:
		//	tempBufferDesc.ByteWidth = sizeof(NormalBuffer);
		//	if (FAILED(device->CreateBuffer(&tempBufferDesc, NULL, &m_normalBuffer)))
		//		return false;

		//	m_buffers = { m_normalBuffer };
		//	break;
		case GBufferShader::BufferType::SSAO_NOISE:
			tempBufferDesc.ByteWidth = sizeof(SSAONoiseBuffer);
			if (FAILED(device->CreateBuffer(&tempBufferDesc, NULL, &m_ssaoNoiseBuffer)))
				return false;

			m_buffers = { m_ssaoNoiseBuffer };
			break;
		case GBufferShader::BufferType::SSAO:
			tempBufferDesc.ByteWidth = sizeof(SSAOBuffer);
			if (FAILED(device->CreateBuffer(&tempBufferDesc, NULL, &m_ssaoBuffer)))
				return false;

			m_buffers = { m_ssaoBuffer };
			break;
	}

	return true;
}

bool GBufferShader::SetShaderParameters(ID3D11DeviceContext * deviceContext, XMMATRIX & worldMatrix, XMMATRIX & viewMatrix, XMMATRIX & projectionMatrix)
{
	if (BaseShaderClass::SetShaderParameters(deviceContext, worldMatrix, viewMatrix, projectionMatrix) == false)
		return false;

	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	PositionBuffer* positionBuffer;
	NormalBuffer* normalBuffer;
	SSAONoiseBuffer* ssaoNoiseBuffer;
	SSAOBuffer* ssaoBuffer;
	unsigned int bufferNmber;

	/////// VERTEX BUFFERS ///////
	//SSAO noise buffer
	int bufferNumber = 1;
	switch (m_bufferType)
	{
		case GBufferShader::BufferType::SSAO_NOISE:
			result = deviceContext->Map(m_ssaoNoiseBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
			if (FAILED(result))
				return false;

			ssaoNoiseBuffer = (SSAONoiseBuffer*)mappedResource.pData;
			ssaoNoiseBuffer->noise = XMFLOAT2{ 0, 0 };
			ssaoNoiseBuffer->padding = XMFLOAT2{ 0, 0 };

			deviceContext->Unmap(m_ssaoNoiseBuffer, 0);
			deviceContext->VSSetConstantBuffers(bufferNumber, 1, &m_ssaoNoiseBuffer);
			break;

	}


	/////// PIXEL BUFFERS ///////
	switch (m_bufferType)
	{
		case GBufferShader::BufferType::SSAO:
			result = deviceContext->Map(m_ssaoBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
			if (FAILED(result))
				return false;

			ssaoBuffer = (SSAOBuffer*)mappedResource.pData;
			for (int i = 0; i < 64; i++)
				ssaoBuffer->kernelValues[i] = kernelValues[i];

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