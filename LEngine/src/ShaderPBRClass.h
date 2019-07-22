#pragma once
#ifndef _SHADERPBRCLASS_H_
#define _SHADERPBRCLASS_H_

#include "BaseShaderClass.h"

///<summary> Basically unused due to usage of GeneratedPBRClass; Delete in future </summary>
class ShaderPBRClass : public BaseShaderClass
{
private:
	static const int NUM_LIGHTS_DIRECTIONAL = 1;
	static const int NUM_LIGHTS_POINT = 0;
	//#define USE_POINT_LIGHTS

	struct LightingBufferType
	{
		XMFLOAT4 directional_directionStregth[NUM_LIGHTS_DIRECTIONAL];
		XMFLOAT4 directional_color[NUM_LIGHTS_DIRECTIONAL];
#ifdef USE_POINT_LIGHTS
		XMFLOAT4 point_positionWithRadius[NUM_LIGHTS_POINT];
		XMFLOAT4 point_colorWithStrength[NUM_LIGHTS_POINT];
#endif
		//float strength[NUM_LIGHTS];
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
		XMFLOAT4 albedoTint;
	};

	struct ShaderTextureBufferType
	{
		int hasNormalMap;
		int hasRoughnessMap;
		int hasMetalnessMap;
		int hasAlbedoMap;
	};

	struct DirectionalLight 
	{
		XMFLOAT4 direction;
		XMFLOAT4 color;

		DirectionalLight() = default;
		DirectionalLight(XMFLOAT4 direction_, XMFLOAT4 color_) : direction(direction_), color(color_) {}
	};

	struct PointLight
	{
		XMFLOAT4 positionWithRadius;
		XMFLOAT4 colorWithStrength;

		PointLight() = default;
		PointLight(XMFLOAT4 positionWithRadius_, XMFLOAT4 colorWithStrength_) : positionWithRadius(positionWithRadius_), colorWithStrength(colorWithStrength_) {}
	};

public:
	bool LoadIrradianceMap(ID3D11ShaderResourceView *& shaderResourceView);
	bool LoadIrradianceMap(ID3D11Device *device, const wchar_t* filename);

	bool LoadEnvironmentMap(ID3D11Device *device, const wchar_t* filename);
	bool LoadEnvironmentMap(ID3D11ShaderResourceView *& shaderResourceView);
	bool AddEnvironmentMapLevel(ID3D11ShaderResourceView *& shaderResourceView);
	bool AddEnvironmentMapLevel(ID3D11Device *device, const wchar_t* filename);
	int GetEnvironmentMipLevels() const;

	bool LoadBrdfLut(ID3D11Device *device, const wchar_t* filename);

	void AddDirectionalLight(XMFLOAT4 directionStrength, XMFLOAT3 color);
	void AddDirectionalLight(XMFLOAT4 directionStrength, float red, float green, float blue);
	void AddDirectionalLight(XMFLOAT3 direction, float strength, float red, float green, float blue);

	void AddPointLight(XMFLOAT4 positionWithRadius, XMFLOAT4 colorWithStrength);
	void AddPointLight(XMFLOAT4 positionWithRadius, XMFLOAT3 color, float colorStrength);
	void AddPointLight(XMFLOAT4 positionWithRadius, float red, float green, float blue, float colorStrength);
	void AddPointLight(XMFLOAT3 position, float radius, float red, float green, float blue, float colorStrength);
	void AddPointLight(XMFLOAT3 position, float radius, XMFLOAT3 color, float colorStrength);

	void SetRoughness(float roughness);
	void SetMetalness(float metalness);

public:
	//Texture resources
	ID3D11Resource* m_diffuseTexture;
	ID3D11ShaderResourceView* m_diffuseTextureView;
	ID3D11Resource* m_normalTexture;
	ID3D11ShaderResourceView* m_normalTextureView;
	ID3D11Resource* m_roughnessTexture;
	ID3D11ShaderResourceView* m_roughnessTextureView;
	ID3D11Resource* m_metalnessTexture;
	ID3D11ShaderResourceView* m_metalnessTextureView;
	ID3D11Resource* m_brdfLut;
	ID3D11ShaderResourceView* m_brdfLutView;

	XMFLOAT3 m_cameraPosition{ 0,0,0 };
	float m_roughness{ 0 };
	float m_metalness{ 0 };
	std::array<float, 3> m_tint{ 1,1,1 };

protected:
	virtual bool CreateBufferAdditionals(ID3D11Device *&device) override;
	virtual bool SetShaderParameters(ID3D11DeviceContext*, XMMATRIX&, XMMATRIX&, XMMATRIX&) override;

protected:
	std::vector<DirectionalLight> m_directionalLight;
	std::vector<PointLight> m_pointLight;

	ID3D11Buffer* m_lightingBuffer;
	ID3D11Buffer* m_cameraBuffer;
	ID3D11Buffer* m_PBRBuffer;
	ID3D11Buffer* m_ShaderTextureBuffer;

	ID3D11Resource* m_irradianceMap;
	ID3D11ShaderResourceView* m_irradianceMapView;
	ID3D11Resource* m_environmentMapTexture;
	ID3D11ShaderResourceView* m_environmentMapTextureView;

	std::vector<ID3D11ShaderResourceView*> m_environmentMapViews;
	std::vector<ID3D11ShaderResourceView*> m_additionalMapViews;
};

#endif // !_SHADERPBRCLASS_H_