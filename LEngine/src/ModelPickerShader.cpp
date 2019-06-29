#include "ModelPickerShader.h"

ModelPickerShader::ModelPickerShader()
{
	BaseShaderClass::BaseShaderClass();
}

void ModelPickerShader::GetColor(XMFLOAT4 & color)
{
	color = m_tint;
}

bool ModelPickerShader::Render(ID3D11DeviceContext * deviceContext, int indexCount, XMMATRIX & worldMatrix, XMMATRIX & viewMatrix, XMMATRIX & projectionMatrix)
{
	if (!SetShaderParameters(deviceContext, worldMatrix, viewMatrix, projectionMatrix))
		return false;

	RenderShader(deviceContext, indexCount);
	return true;
}

void ModelPickerShader::ChangeColor(XMFLOAT4 color)
{
	m_tint = color;
}

void ModelPickerShader::ChangeColor(float r, float g, float b, float a)
{
	m_tint = XMFLOAT4(r, g, b, a);
}

void ModelPickerShader::ChangeAlpha(float alpha)
{
	m_tint.w = alpha;
}

bool ModelPickerShader::CreateBufferAdditionals(ID3D11Device *& device)
{
	BaseShaderClass::CreateBufferAdditionals(device);
	D3D11_BUFFER_DESC tempBufferDesc;

	tempBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	tempBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	tempBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	tempBufferDesc.MiscFlags = 0;
	tempBufferDesc.StructureByteStride = 0;

	tempBufferDesc.ByteWidth = sizeof(AppearanceBuffer);
	if (FAILED(device->CreateBuffer(&tempBufferDesc, NULL, &m_appearanceBuffer)))
		return false;

	m_buffers = { m_appearanceBuffer };
}

bool ModelPickerShader::CreateSamplerState(ID3D11Device * device)
{
	return BaseShaderClass::CreateSamplerState(device);
}

bool ModelPickerShader::SetShaderParameters(ID3D11DeviceContext * deviceContext, XMMATRIX & worldMatrix, XMMATRIX & viewMatrix, XMMATRIX & projectionMatrix)
{
	if (BaseShaderClass::SetShaderParameters(deviceContext, worldMatrix, viewMatrix, projectionMatrix) == false)
		return false;

	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	AppearanceBuffer* dataPtr2;
	unsigned int bufferNumber;

	/////// PIXEL BUFFERS ///////
	//Appearance buffer
	result = deviceContext->Map(m_appearanceBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
		return false;

	dataPtr2 = (AppearanceBuffer*)mappedResource.pData;
	dataPtr2->color = m_tint;

	deviceContext->Unmap(m_appearanceBuffer, 0);
	bufferNumber = 0;
	deviceContext->PSSetConstantBuffers(bufferNumber, 1, &m_appearanceBuffer);

	return true;
}