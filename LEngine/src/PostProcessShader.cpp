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

void PostProcessShader::SetLUTBuffer(ID3D11ShaderResourceView *& lutBuffer)
{
	m_lutBufferView = lutBuffer;
}

void PostProcessShader::SetChromaticAberrationBuffer(ID3D11ShaderResourceView *& chromaticAberrationBuffer)
{
}

void PostProcessShader::UseChromaticAberration(bool setActive)
{
	m_chromaticAberration = setActive;
}

void PostProcessShader::SetChromaticAberrationOffsets(float red, float green, float blue)
{
	SetChromaticAberrationOffsets(XMFLOAT3{ red, green, blue });
}

void PostProcessShader::SetChromaticAberrationOffsets(XMFLOAT3 offset)
{
	m_chromaticAberrationOffset = offset;
}

void PostProcessShader::SetChromaticAberrationIntensity(float intensity)
{
	m_chromaticAberrationIntensity = intensity;
}

void PostProcessShader::UseGrain(bool setActive)
{
	m_useGrain = setActive;
}

void PostProcessShader::SetNoise(ID3D11ShaderResourceView *& whiteNoise, ID3D11ShaderResourceView *& perlinNoise)
{
	SetWhiteNoise(whiteNoise);
	SetPerlinNoise(perlinNoise);
}

void PostProcessShader::SetWhiteNoise(ID3D11ShaderResourceView *& whiteNoise)
{
	m_whiteNoiseBufferView = whiteNoise;
}

void PostProcessShader::SetPerlinNoise(ID3D11ShaderResourceView *& perlinNoise)
{
	m_perlinNoiseBufferView = perlinNoise;
}

void PostProcessShader::SetGrainSettings(float intensity, float size, int type, bool hasColor)
{
	m_grainIntensity = intensity;
	m_grainSize = size;
	m_type = type;
	m_hasGrainColor = hasColor;
}

void PostProcessShader::ResetSSAO()
{
	m_ssaoBufferView = nullptr;
}

void PostProcessShader::ResetBloom()
{
	m_bloomBufferView = nullptr;
}

void PostProcessShader::ResetLUT()
{
	m_lutBufferView = nullptr;
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

	tempBufferDesc.ByteWidth = sizeof(ChromaticAberrationBuffer);
	if (FAILED(device->CreateBuffer(&tempBufferDesc, NULL, &m_chromaticBuffer)))
		return false;

	tempBufferDesc.ByteWidth = sizeof(GrainSettings);
	if (FAILED(device->CreateBuffer(&tempBufferDesc, NULL, &m_grainBuffer)))
		return false;

	m_buffers = { m_textureBuffer, m_chromaticBuffer, m_grainBuffer };

	//Used as constructor method
	if (m_chromaticAberrationTextureView == nullptr)
	{
		BaseShaderClass::LoadTexture(device, L"RadialGradient.png", m_chromaticAberrationTexture, m_chromaticAberrationTextureView, false);
	}

	if (m_whiteNoiseBufferView == nullptr && m_perlinNoiseBufferView == nullptr)
	{
		LoadTexture(device, L"perlinNoise.png", m_perlinNoiseBuffer, m_perlinNoiseBufferView, false);
		LoadTexture(device, L"WhiteNoiseDitheringBlurred.png", m_whiteNoiseBuffer, m_whiteNoiseBufferView, false);
	}

	return true;
}

bool PostProcessShader::SetShaderParameters(ID3D11DeviceContext *deviceContext, XMMATRIX &worldMatrix, XMMATRIX &viewMatrix, XMMATRIX &projectionMatrix)
{
	if (BaseShaderClass::SetShaderParameters(deviceContext, worldMatrix, viewMatrix, projectionMatrix) == false)
		return false;

	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	TextureBufferType* dataPtr2;
	ChromaticAberrationBuffer* dataPtr3;
	GrainSettings* dataPtr4;
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
	dataPtr2->hasLUT = m_lutBufferView != nullptr;
	dataPtr2->hasChromaticAberration = (float)m_chromaticAberration;
	dataPtr2->hasGrain = (float)m_useGrain;
	dataPtr2->padding = XMFLOAT3{ 0,0,0 };

	deviceContext->Unmap(m_textureBuffer, 0);
	bufferNumber = 0;
	deviceContext->PSSetConstantBuffers(bufferNumber++, 1, &m_textureBuffer);

	//Chromatic aberration buffer
	result = deviceContext->Map(m_chromaticBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
		return false;

	dataPtr3 = (ChromaticAberrationBuffer*)mappedResource.pData;
	dataPtr3->red = m_chromaticAberrationOffset.x;
	dataPtr3->green = m_chromaticAberrationOffset.y;
	dataPtr3->blue = m_chromaticAberrationOffset.z;
	dataPtr3->intensity = m_chromaticAberrationIntensity;

	deviceContext->Unmap(m_chromaticBuffer, 0);
	deviceContext->PSSetConstantBuffers(bufferNumber++, 1, &m_chromaticBuffer);

	//Grain buffer
	result = deviceContext->Map(m_grainBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
		return false;

	dataPtr4 = (GrainSettings*)mappedResource.pData;
	dataPtr4->intensity = m_grainIntensity;
	dataPtr4->size = m_grainSize;
	dataPtr4->hasColor = m_hasGrainColor;
	dataPtr4->type = m_type;

	deviceContext->Unmap(m_grainBuffer, 0);
	deviceContext->PSSetConstantBuffers(bufferNumber++, 1, &m_grainBuffer);
	/////// RESOURCES ///////
	//Pixel shader resources
	bufferNumber = 0;
	deviceContext->PSSetShaderResources(bufferNumber++, 1, &m_screenBufferView);
	deviceContext->PSSetShaderResources(bufferNumber++, 1, &m_ssaoBufferView);
	deviceContext->PSSetShaderResources(bufferNumber++, 1, &m_bloomBufferView);
	deviceContext->PSSetShaderResources(bufferNumber++, 1, &m_lutBufferView);
	deviceContext->PSSetShaderResources(bufferNumber++, 1, &m_chromaticAberrationTextureView);
	deviceContext->PSSetShaderResources(bufferNumber++, 1, &m_whiteNoiseBufferView);
	deviceContext->PSSetShaderResources(bufferNumber++, 1, &m_perlinNoiseBufferView);
	return true;
}