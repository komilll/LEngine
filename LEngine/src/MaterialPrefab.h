#pragma once
#ifndef _MATERIALPREFAB_H_
#define _MATERIALPREFAB_H_

#include <string>
#include <fstream>
#include "ShaderPBRGenerated.h"
#include "d3dclass.h"

class MaterialPrefab
{
public:
	MaterialPrefab(const std::string name, D3DClass* d3d);
	ShaderPBRGenerated* const GetShader() const { return m_shader; };
	std::string GetName() const { return m_name; };

private:
	std::vector<std::string> GetTextureNames(const std::string materialFilename) const;

private:
	std::string m_name;
	ShaderPBRGenerated* m_shader;
	D3DClass* m_D3D;
};

#endif // !_MATERIALPREFAB_H_