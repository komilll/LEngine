#include "MaterialPrefab.h"

MaterialPrefab::MaterialPrefab(const std::string name, D3DClass* d3d)
{
	if (!d3d)
	{
		return;
	}
	m_D3D = d3d;
	m_name = name;

	//Generate PS file
	const std::string psString = "GeneratedShaders/" + m_name + ".ps";
	const std::wstring psWideString = std::wstring(psString.begin(), psString.end());
	const WCHAR* psWCHAR = psWideString.c_str();

	//Create and initialize shader
	if (!(m_shader = new ShaderPBRGenerated))
		return;
	m_shader->m_materialNames = GetTextureNames("Materials/" + m_name + ".material");
	if (!m_shader->Initialize(m_D3D->GetDevice(), *m_D3D->GetHWND(), L"pbr_used.vs", const_cast<WCHAR*>(psWCHAR), m_D3D->GetBaseInputType()))
		return;

	//Base light
	m_shader->AddDirectionalLight(XMFLOAT3{ 0.0f, 1.0f, -5.0f }, 1.0f, 1.0f, 1.0f, 1.0f);
	m_shader->AddDirectionalLight(XMFLOAT3{ 0.0f, 1.0f, 5.0f }, 0.5f, 1.0f, 0.5f, 0.75f);
	//Additional lights
	m_shader->AddDirectionalLight(XMFLOAT3{ 0.0f, 0.0f, -1.0f }, 0.4f, 1.0f, 0.0f, 0.0f);
	m_shader->AddDirectionalLight(XMFLOAT3{ 0.0f, 2.0f, 6.0f }, 0.7f, 0.0f, 1.0f, 0.0f);
	m_shader->AddDirectionalLight(XMFLOAT3{ 0.0f, 10.f, 3.0f }, 1.25f, 0.0f, 0.0f, 1.0f);
	m_shader->AddDirectionalLight(XMFLOAT3{ 5.0f , -3.0f, 0.0f }, 1.0f, 1.0f, 1.0f, 0.4f);

	//Load diffuse and specular IBL
	if (m_shader->LoadIrradianceMap(m_D3D->GetDevice(), L"Skyboxes/conv_cubemap.dds"))
	{
		m_shader->AddEnvironmentMapLevel(m_D3D->GetDevice(), L"Skyboxes/enviro_cubemap_0.dds");
		m_shader->AddEnvironmentMapLevel(m_D3D->GetDevice(), L"Skyboxes/enviro_cubemap_1.dds");
		m_shader->AddEnvironmentMapLevel(m_D3D->GetDevice(), L"Skyboxes/enviro_cubemap_2.dds");
		m_shader->AddEnvironmentMapLevel(m_D3D->GetDevice(), L"Skyboxes/enviro_cubemap_3.dds");
		m_shader->AddEnvironmentMapLevel(m_D3D->GetDevice(), L"Skyboxes/enviro_cubemap_4.dds");
		if (!m_shader->LoadBrdfLut(m_D3D->GetDevice(), L"Skyboxes/LutBRDF.dds"))
			return;
	}

	//Load additional settings
	if (std::ifstream input{ "Materials/" + name + ".settings" })
	{
		std::string line;

		getline(input, line);
		m_isEmissive = (line == "1");
	}
	m_shader->m_isEmissive = m_isEmissive;
}

std::vector<std::string> MaterialPrefab::GetTextureNames(const std::string materialFilename) const
{
	//Find all textures in ".material" file and pass their names
	std::vector<std::string> textures;
	if (ifstream in{ materialFilename })
	{
		std::string line;
		while (in >> line)
		{
			if (line.find(".png") != std::string::npos || line.find(".PNG") != std::string::npos || 
				line.find(".dds") != std::string::npos || line.find(".DDS") != std::string::npos)
			{
				textures.push_back(line);
			}
		}
	}

	return textures;
}
