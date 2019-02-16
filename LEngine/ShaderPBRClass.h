#pragma once
#ifndef _SHADERPBRCLASS_H_
#define _SHADERPBRCLASS_H_

#include "BaseShaderClass.h"

class ShaderPBRClass : public BaseShaderClass
{
private:
	struct LightingBufferType
	{
		XMFLOAT3 direction;
		float strength;
	};

	struct CameraBufferType
	{
		XMFLOAT3 cameraDirection;
		float padding;
	};

	struct PBRBufferType 
	{
		float roughness;
		float metalness;
		XMFLOAT2 padding;
	};

	struct ShaderTextureBufferType
	{
		int hasNormalMap;
		int hasRoughnessMap;
		int hasMetalnessMap;
		int paddingShaderTextureBuffer;
	};

public:
	bool LoadIrradianceMap(ID3D11Device *device, const wchar_t* filename);
	void SetRoughness(float roughness);
	void SetMetalness(float metalness);

	//Texture resources
	ID3D11Resource* m_diffuseTexture;
	ID3D11ShaderResourceView* m_diffuseTextureView;
	ID3D11Resource* m_normalTexture;
	ID3D11ShaderResourceView* m_normalTextureView;
	ID3D11Resource* m_roughnessTexture;
	ID3D11ShaderResourceView* m_roughnessTextureView;
	ID3D11Resource* m_metalnessTexture;
	ID3D11ShaderResourceView* m_metalnessTextureView;

	XMFLOAT4 m_lightDirection;
	XMFLOAT3 m_cameraPosition;
	float m_roughness = 0;
	float m_metalness = 0;

protected:
	virtual bool CreateBufferAdditionals(ID3D11Device *&device) override;
	virtual bool SetShaderParameters(ID3D11DeviceContext*, XMMATRIX&, XMMATRIX&, XMMATRIX&) override;

private:
	ID3D11Buffer* m_lightingBuffer;
	ID3D11Buffer* m_cameraBuffer;
	ID3D11Buffer* m_PBRBuffer;
	ID3D11Buffer* m_ShaderTextureBuffer;

	ID3D11Resource* m_irradianceMap;
	ID3D11ShaderResourceView* m_irradianceMapView;
};

#endif // !_SHADERPBRCLASS_H_