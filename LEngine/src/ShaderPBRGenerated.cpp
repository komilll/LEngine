#include "ShaderPBRGenerated.h"

bool ShaderPBRGenerated::LoadIrradianceMap(ID3D11Device *device, const wchar_t * filename)
{
	return LoadTexture(device, filename, m_irradianceMap, m_irradianceMapView);
}

bool ShaderPBRGenerated::LoadIrradianceMap(ID3D11ShaderResourceView *& shaderResourceView)
{
	m_irradianceMapView = shaderResourceView;
	return true;
}

void ShaderPBRGenerated::SetRoughness(float roughness)
{
	m_roughness = roughness;
}

void ShaderPBRGenerated::SetMetalness(float metalness)
{
	m_metalness = metalness;
}

bool ShaderPBRGenerated::LoadEnvironmentMap(ID3D11Device *device, const wchar_t * filename)
{
	return LoadTexture(device, filename, m_environmentMapTexture, m_environmentMapTextureView);
}

bool ShaderPBRGenerated::LoadEnvironmentMap(ID3D11ShaderResourceView *& shaderResourceView)
{
	m_environmentMapTextureView = shaderResourceView;
	return true;
}

bool ShaderPBRGenerated::AddEnvironmentMapLevel(ID3D11ShaderResourceView *& shaderResourceView)
{
	m_environmentMapViews.push_back(shaderResourceView);
	return true;
}

bool ShaderPBRGenerated::AddEnvironmentMapLevel(ID3D11Device * device, const wchar_t * filename)
{
	m_environmentMapViews.push_back(nullptr);
	return LoadTexture(device, filename, m_environmentMapTexture, m_environmentMapViews.at(m_environmentMapViews.size() - 1));
}

int ShaderPBRGenerated::GetEnvironmentMipLevels()
{
	return m_environmentMapViews.size();
}

bool ShaderPBRGenerated::LoadBrdfLut(ID3D11Device *device, const wchar_t * filename)
{
	return LoadTexture(device, filename, m_brdfLut, m_brdfLutView);
}

void ShaderPBRGenerated::AddDirectionalLight(XMFLOAT4 directionStrength, XMFLOAT3 color)
{
	m_directionalLight.push_back(DirectionalLight{ directionStrength,{ color.x, color.y, color.z, 0.0f } });
}

void ShaderPBRGenerated::AddDirectionalLight(XMFLOAT4 directionStrength, float red, float green, float blue)
{
	m_directionalLight.push_back(DirectionalLight{ directionStrength,{ red, green, blue, 0.0f } });
}

void ShaderPBRGenerated::AddDirectionalLight(XMFLOAT3 direction, float strength, float red, float green, float blue)
{
	m_directionalLight.push_back(DirectionalLight{ { direction.x, direction.y, direction.z, strength },{ red, green, blue, 0.0f } });
}

void ShaderPBRGenerated::AddPointLight(XMFLOAT4 positionWithRadius, XMFLOAT4 colorWithStrength)
{
	m_pointLight.push_back(PointLight{ positionWithRadius, colorWithStrength });
}

void ShaderPBRGenerated::AddPointLight(XMFLOAT4 positionWithRadius, XMFLOAT3 color, float colorStrength)
{
	m_pointLight.push_back(PointLight{ positionWithRadius,{ color.x, color.y, color.z, colorStrength } });
}

void ShaderPBRGenerated::AddPointLight(XMFLOAT4 positionWithRadius, float red, float green, float blue, float colorStrength)
{
	m_pointLight.push_back(PointLight{ positionWithRadius,{ red, green, blue, colorStrength } });
}

void ShaderPBRGenerated::AddPointLight(XMFLOAT3 position, float radius, float red, float green, float blue, float colorStrength)
{
	m_pointLight.push_back(PointLight{ { position.x, position.y, position.z, radius },{ red, green, blue, colorStrength } });
}

void ShaderPBRGenerated::AddPointLight(XMFLOAT3 position, float radius, XMFLOAT3 color, float colorStrength)
{
	m_pointLight.push_back(PointLight{ { position.x, position.y, position.z, radius },{ color.x, color.y, color.z, colorStrength } });
}

bool ShaderPBRGenerated::CreateBufferAdditionals(ID3D11Device * &device)
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

	LoadGeneratedTextures(device);

	return true;
}

bool ShaderPBRGenerated::SetShaderParameters(ID3D11DeviceContext *deviceContext, XMMATRIX &worldMatrix, XMMATRIX &viewMatrix, XMMATRIX &projectionMatrix)
{
	if (BaseShaderClass::SetShaderParameters(deviceContext, worldMatrix, viewMatrix, projectionMatrix) == false)
		return false;

	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	unsigned int bufferNumber{ 0 };

	/////// VERTEX BUFFERS ///////
	//Camera buffer
	result = deviceContext->Map(m_cameraBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
		return false;

	CameraBufferType* dataPtr3{ static_cast<CameraBufferType*>(mappedResource.pData) };
	dataPtr3->cameraDirection = m_cameraPosition;
	dataPtr3->padding = 0;

	deviceContext->Unmap(m_cameraBuffer, 0);
	bufferNumber = 1;
	deviceContext->VSSetConstantBuffers(bufferNumber++, 1, &m_cameraBuffer);

	/////// PIXEL BUFFERS ///////
	//Lighting buffer
	result = deviceContext->Map(m_lightingBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
		return false;

	LightingBufferType* dataPtr2{ static_cast<LightingBufferType*>(mappedResource.pData) };
	for (int i = 0; i < NUM_LIGHTS_DIRECTIONAL; i++)
	{
		dataPtr2->directional_directionStregth[i] = m_directionalLight.at(i).direction;
		dataPtr2->directional_color[i] = m_directionalLight.at(i).color;
		//dataPtr2->strength[i] = m_lightDirection.at(i).w;
	}
#ifdef USE_POINT_LIGHTS
	for (int i = 0; i < NUM_LIGHTS_POINT; i++)
	{
		dataPtr2->point_positionWithRadius[i] = m_pointLight.at(i).positionWithRadius;
		//dataPtr2->point_colorWithStrength[i] = XMFLOAT4{ 1, 0, 0, 2 };
		dataPtr2->point_colorWithStrength[i] = m_pointLight.at(i).colorWithStrength;
	}
#endif

	deviceContext->Unmap(m_lightingBuffer, 0);
	bufferNumber = 0;
	deviceContext->PSSetConstantBuffers(bufferNumber++, 1, &m_lightingBuffer);

	//PBR Buffer
	result = deviceContext->Map(m_PBRBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
		return false;

	PBRBufferType* dataPtr4{ static_cast<PBRBufferType*>(mappedResource.pData) };
	dataPtr4->roughness = m_roughness;
	dataPtr4->metalness = m_metalness;
	dataPtr4->albedoTint = XMFLOAT4{ m_tint[0], m_tint[1], m_tint[2], 1.0f };
	dataPtr4->padding = XMFLOAT2{ 0,0 };

	deviceContext->Unmap(m_PBRBuffer, 0);
	deviceContext->PSSetConstantBuffers(bufferNumber++, 1, &m_PBRBuffer);

	//Shader Texture Buffer
	result = deviceContext->Map(m_ShaderTextureBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
		return false;

	ShaderTextureBufferType* dataPtr5{ static_cast<ShaderTextureBufferType*>(mappedResource.pData) };
	dataPtr5->hasNormalMap = m_normalTextureView != nullptr;
	dataPtr5->hasRoughnessMap = m_roughnessTextureView != nullptr;
	dataPtr5->hasMetalnessMap = m_metalnessTextureView != nullptr;
	dataPtr5->hasAlbedoMap = m_diffuseTextureView != nullptr;

	deviceContext->Unmap(m_ShaderTextureBuffer, 0);
	deviceContext->PSSetConstantBuffers(bufferNumber++, 1, &m_ShaderTextureBuffer);

	/////// RESOURCES ///////
	//Pixel shader resources
	bufferNumber = 0;
	deviceContext->PSSetShaderResources(bufferNumber++, 1, &m_irradianceMapView);
	deviceContext->PSSetShaderResources(bufferNumber++, 1, &m_brdfLutView);
	for (const auto& map : m_environmentMapViews)
		deviceContext->PSSetShaderResources(bufferNumber++, 1, &map);
	for (const auto& map : m_additionalMapViews)
		deviceContext->PSSetShaderResources(bufferNumber++, 1, &map);
	return true;
}
void ShaderPBRGenerated::LoadGeneratedTextures(ID3D11Device *device)
{
	ID3D11Resource* resource{nullptr};

	m_additionalMapViews.clear();
	for (const auto& name : m_materialNames)
	{
		m_additionalMapViews.push_back(nullptr);
	}
	for (int i = 0; i < m_materialNames.size(); ++i)
	{
		std::wstring widestr = std::wstring(m_materialNames.at(i).begin(), m_materialNames.at(i).end());
		const wchar_t* widecstr = widestr.c_str();

		if (hasEnding(m_materialNames.at(i), "DDS"))
		{
			LoadTexture(device, widecstr, resource, m_additionalMapViews.at(i), true);
		}
		else
		{
			if (hasEnding(m_materialNames.at(i), "dds"))
			{
				LoadTexture(device, widecstr, resource, m_additionalMapViews.at(i), true);
			}
			else
			{
				LoadTexture(device, widecstr, resource, m_additionalMapViews.at(i), false);
			}
		}
	}
}

bool ShaderPBRGenerated::hasEnding(std::string const &fullString, std::string const &ending)
{
	if (fullString.length() >= ending.length()) 
	{
		return (0 == fullString.compare(fullString.length() - ending.length(), ending.length(), ending));
	}
	else 
	{
		return false;
	}
}
