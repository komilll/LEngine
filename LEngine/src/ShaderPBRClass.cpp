#include "ShaderPBRClass.h"

bool ShaderPBRClass::LoadIrradianceMap(ID3D11Device *device, const wchar_t * filename)
{
	return LoadTexture(device, filename, m_irradianceMap, m_irradianceMapView);
}

bool ShaderPBRClass::LoadIrradianceMap(ID3D11ShaderResourceView *& shaderResourceView)
{
	m_irradianceMapView = shaderResourceView;
	return true;
}

void ShaderPBRClass::SetRoughness(float roughness)
{
	m_roughness = roughness;
}

void ShaderPBRClass::SetMetalness(float metalness)
{
	m_metalness = metalness;
}

bool ShaderPBRClass::LoadEnvironmentMap(ID3D11Device *device, const wchar_t * filename)
{
	return LoadTexture(device, filename, m_environmentMapTexture, m_environmentMapTextureView);
}

bool ShaderPBRClass::LoadEnvironmentMap(ID3D11ShaderResourceView *& shaderResourceView)
{
	m_environmentMapTextureView = shaderResourceView;
	return true;
}

bool ShaderPBRClass::AddEnvironmentMapLevel(ID3D11ShaderResourceView *& shaderResourceView)
{
	m_environmentMapViews.push_back(shaderResourceView);
	return true;
}

bool ShaderPBRClass::AddEnvironmentMapLevel(ID3D11Device * device, const wchar_t * filename)
{
	m_environmentMapViews.push_back(nullptr);
	if (!LoadTexture(device, filename, m_environmentMapTexture, m_environmentMapViews.at(m_environmentMapViews.size() - 1)))
		return false;
	//m_environmentMapViews.push_back(m_environmentMapTextureView);
	return true;
}

int ShaderPBRClass::GetEnvironmentMipLevels()
{
	return m_environmentMapViews.size();
}

bool ShaderPBRClass::LoadBrdfLut(ID3D11Device *device, const wchar_t * filename)
{
	return LoadTexture(device, filename, m_brdfLut, m_brdfLutView);
}

void ShaderPBRClass::AddLights(XMFLOAT4 directionStrength)
{
	m_lightDirection.push_back(directionStrength);
}

void ShaderPBRClass::AddLights(XMFLOAT3 direction, float strength)
{
	m_lightDirection.push_back(XMFLOAT4{ direction.x, direction.y, direction.z, strength });
}

bool ShaderPBRClass::CreateBufferAdditionals(ID3D11Device * &device)
{
	BaseShaderClass::CreateBufferAdditionals(device);
	D3D11_BUFFER_DESC tempBufferDesc;

	tempBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	tempBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	tempBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	tempBufferDesc.MiscFlags = 0;
	tempBufferDesc.StructureByteStride = 0;

	tempBufferDesc.ByteWidth = sizeof(LightingBufferType);
	if (FAILED(device->CreateBuffer(&tempBufferDesc, NULL, &m_lightingBuffer)))
		return false;

	tempBufferDesc.ByteWidth = sizeof(CameraBufferType);
	if (FAILED(device->CreateBuffer(&tempBufferDesc, NULL, &m_cameraBuffer)))
		return false;

	tempBufferDesc.ByteWidth = sizeof(PBRBufferType);
	if (FAILED(device->CreateBuffer(&tempBufferDesc, NULL, &m_PBRBuffer)))
		return false;

	tempBufferDesc.ByteWidth = sizeof(PBRBufferType);
	if (FAILED(device->CreateBuffer(&tempBufferDesc, NULL, &m_ShaderTextureBuffer)))
		return false;

	m_buffers = { m_lightingBuffer, m_cameraBuffer, m_PBRBuffer, m_ShaderTextureBuffer };

	return true;
}

bool ShaderPBRClass::SetShaderParameters(ID3D11DeviceContext *deviceContext, XMMATRIX &worldMatrix, XMMATRIX &viewMatrix, XMMATRIX &projectionMatrix)
{
	if (BaseShaderClass::SetShaderParameters(deviceContext, worldMatrix, viewMatrix, projectionMatrix) == false)
		return false;

	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	LightingBufferType* dataPtr2;
	CameraBufferType* dataPtr3;
	PBRBufferType* dataPtr4;
	ShaderTextureBufferType* dataPtr5;
	unsigned int bufferNumber;

	/////// VERTEX BUFFERS ///////
	//Camera buffer
	result = deviceContext->Map(m_cameraBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
		return false;

	dataPtr3 = (CameraBufferType*)mappedResource.pData;
	dataPtr3->cameraDirection = m_cameraPosition;
	dataPtr3->padding = 0;

	deviceContext->Unmap(m_cameraBuffer, 0);
	bufferNumber = 1;
	deviceContext->VSSetConstantBuffers(bufferNumber, 1, &m_cameraBuffer);

	/////// PIXEL BUFFERS ///////
	//Lighting buffer
	result = deviceContext->Map(m_lightingBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
		return false;

	dataPtr2 = (LightingBufferType*)mappedResource.pData;
	for (int i = 0; i < NUM_LIGHTS; i++)
	{
		dataPtr2->direction[i] = XMFLOAT3{ m_lightDirection.at(i).x, m_lightDirection.at(i).y, m_lightDirection.at(i).z };
		dataPtr2->strength[i] = m_lightDirection.at(i).w;
	}

	deviceContext->Unmap(m_lightingBuffer, 0);
	bufferNumber = 0;
	deviceContext->PSSetConstantBuffers(bufferNumber, 1, &m_lightingBuffer);

	//PBR Buffer
	result = deviceContext->Map(m_PBRBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
		return false;

	dataPtr4 = (PBRBufferType*)mappedResource.pData;
	dataPtr4->roughness = m_roughness;
	dataPtr4->metalness = m_metalness;
	dataPtr4->padding = XMFLOAT2{ 0,0 };

	deviceContext->Unmap(m_PBRBuffer, 0);
	bufferNumber = 1;
	deviceContext->PSSetConstantBuffers(bufferNumber, 1, &m_PBRBuffer);

	//Shader Texture Buffer
	result = deviceContext->Map(m_ShaderTextureBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
		return false;

	dataPtr5 = (ShaderTextureBufferType*)mappedResource.pData;
	dataPtr5->hasNormalMap = m_normalTextureView != nullptr;
	dataPtr5->hasRoughnessMap = m_roughnessTextureView != nullptr;
	dataPtr5->hasMetalnessMap = m_metalnessTextureView != nullptr;
	dataPtr5->paddingShaderTextureBuffer = 0;

	deviceContext->Unmap(m_ShaderTextureBuffer, 0);
	bufferNumber = 2;
	deviceContext->PSSetConstantBuffers(bufferNumber, 1, &m_ShaderTextureBuffer);

	/////// RESOURCES ///////
	//Pixel shader resources
	bufferNumber = 0;
	deviceContext->PSSetShaderResources(bufferNumber++, 1, &m_diffuseTextureView);
	deviceContext->PSSetShaderResources(bufferNumber++, 1, &m_normalTextureView);
	deviceContext->PSSetShaderResources(bufferNumber++, 1, &m_roughnessTextureView);
	deviceContext->PSSetShaderResources(bufferNumber++, 1, &m_metalnessTextureView);
	deviceContext->PSSetShaderResources(bufferNumber++, 1, &m_irradianceMapView);
	deviceContext->PSSetShaderResources(bufferNumber++, 1, &m_brdfLutView);
	for (int i = 0; i < m_environmentMapViews.size(); i++)
		deviceContext->PSSetShaderResources(bufferNumber++, 1, &m_environmentMapViews[i]);
	return true;
}