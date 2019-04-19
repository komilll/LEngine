#include "PostProcessShader.h"

void PostProcessShader::SetScreenBuffer(ID3D11ShaderResourceView*& screenBuffer)
{
	m_screenBufferView = screenBuffer;
}

void PostProcessShader::SetSSAOBuffer(ID3D11ShaderResourceView *&ssaoBuffer)
{
	m_ssaoBufferView = ssaoBuffer;
}

void PostProcessShader::SetBloomBuffer(ID3D11ShaderResourceView *& bloomBuffer)
{
	m_bloomBufferView = bloomBuffer;
}

bool PostProcessShader::CreateBufferAdditionals(ID3D11Device *& device)
{
	BaseShaderClass::CreateBufferAdditionals(device);
	D3D11_BUFFER_DESC tempBufferDesc;

	tempBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	tempBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	tempBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	tempBufferDesc.MiscFlags = 0;
	tempBufferDesc.StructureByteStride = 0;

	tempBufferDesc.ByteWidth = sizeof(TextureBufferType);
	if (FAILED(device->CreateBuffer(&tempBufferDesc, NULL, &m_textureBuffer)))
		return false;

	m_buffers = { m_textureBuffer };

	return true;
}

bool PostProcessShader::SetShaderParameters(ID3D11DeviceContext *deviceContext, XMMATRIX &worldMatrix, XMMATRIX &viewMatrix, XMMATRIX &projectionMatrix)
{
	if (BaseShaderClass::SetShaderParameters(deviceContext, worldMatrix, viewMatrix, projectionMatrix) == false)
		return false;

	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	TextureBufferType* dataPtr2;
	unsigned int bufferNumber;

	/////// VERTEX BUFFERS ///////

	/////// PIXEL BUFFERS ///////
	//Texture buffer
	result = deviceContext->Map(m_textureBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
		return false;

	dataPtr2 = (TextureBufferType*)mappedResource.pData;
	dataPtr2->hasSSAO = m_ssaoBufferView != nullptr;
	dataPtr2->hasBloom = m_bloomBufferView != nullptr;
	dataPtr2->padding = XMFLOAT2{ 0,0 };

	deviceContext->Unmap(m_textureBuffer, 0);
	bufferNumber = 0;
	deviceContext->PSSetConstantBuffers(bufferNumber, 1, &m_textureBuffer);

	/////// RESOURCES ///////
	//Pixel shader resources
	bufferNumber = 0;
	deviceContext->PSSetShaderResources(bufferNumber++, 1, &m_screenBufferView);
	deviceContext->PSSetShaderResources(bufferNumber++, 1, &m_ssaoBufferView);
	deviceContext->PSSetShaderResources(bufferNumber++, 1, &m_bloomBufferView);
	return true;
}