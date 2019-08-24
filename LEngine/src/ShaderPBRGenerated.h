#pragma once
#ifndef _SHADERPBRGENERATED_H_
#define _SHADERPBRGENERATED_H_

#include "BaseShaderClass.h"

class ShaderPBRGenerated : public BaseShaderClass
{
private:
	static constexpr int NUM_LIGHTS_DIRECTIONAL = 2;
	static constexpr int NUM_LIGHTS_POINT = 1;

	struct LightingBufferType
	{
		XMFLOAT4 directional_directionStregth[NUM_LIGHTS_DIRECTIONAL];
		XMFLOAT4 directional_color[NUM_LIGHTS_DIRECTIONAL];
	};

	struct PointLightBuffer
	{
		XMFLOAT4 point_positionWithRadius[NUM_LIGHTS_POINT];
		XMFLOAT4 point_colorWithStrength[NUM_LIGHTS_POINT];
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

public:
	struct PointLight
	{
		XMFLOAT4 m_positionWithRadius;
		XMFLOAT4 m_colorWithStrength;
	};

public:
	bool LoadIrradianceMap(ID3D11ShaderResourceView *& shaderResourceView);
	bool LoadIrradianceMap(ID3D11Device *device, const wchar_t* filename);

	bool LoadEnvironmentMap(ID3D11Device *device, const wchar_t* filename);
	bool LoadEnvironmentMap(ID3D11ShaderResourceView *& shaderResourceView);
	bool AddEnvironmentMapLevel(ID3D11ShaderResourceView *& shaderResourceView);
	bool AddEnvironmentMapLevel(ID3D11Device *device, const wchar_t* filename);
	int GetEnvironmentMipLevels();

	bool LoadBrdfLut(ID3D11Device *device, const wchar_t* filename);

	void AddDirectionalLight(XMFLOAT4 directionStrength, XMFLOAT3 color);
	void AddDirectionalLight(XMFLOAT4 directionStrength, float red, float green, float blue);
	void AddDirectionalLight(XMFLOAT3 direction, float strength, float red, float green, float blue);

	static int AddPointLight(XMFLOAT4 positionWithRadius, XMFLOAT4 colorWithStrength);
	static void UpdatePointLight(int index, XMFLOAT4 positionWithRadius, XMFLOAT4 colorWithStrength);
	//void AddPointLight(XMFLOAT4 positionWithRadius, XMFLOAT3 color, float colorStrength);
	//void AddPointLight(XMFLOAT4 positionWithRadius, float red, float green, float blue, float colorStrength);
	//void AddPointLight(XMFLOAT3 position, float radius, float red, float green, float blue, float colorStrength);
	//void AddPointLight(XMFLOAT3 position, float radius, XMFLOAT3 color, float colorStrength);
	//void AddPointLight(PointLight* light);

	void SetRoughness(float roughness);
	void SetMetalness(float metalness);
//GENERATED METHODS
	void LoadGeneratedTextures(ID3D11Device * device);

private:
	bool hasEnding(std::string const & fullString, std::string const & ending);

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

	XMFLOAT3 m_cameraPosition;
	float m_roughness{ 0 };
	float m_metalness{ 0 };
	std::array<float, 3> m_tint { 1,1,1 };
	std::vector<std::string> m_materialNames;

protected:
	virtual bool CreateBufferAdditionals(ID3D11Device *&device) override;
	virtual bool SetShaderParameters(ID3D11DeviceContext*, XMMATRIX&, XMMATRIX&, XMMATRIX&) override;

private:
	std::vector<DirectionalLight> m_directionalLight;
	static std::vector<ShaderPBRGenerated::PointLight> k_pointLight;

	ID3D11Buffer* m_lightingBuffer;
	ID3D11Buffer* m_pointLightBuffer;
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