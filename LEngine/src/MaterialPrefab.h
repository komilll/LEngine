#pragma once
#ifndef _MATERIALPREFAB_H_
#define _MATERIALPREFAB_H_

#include <string>

class MaterialPrefab
{
public:
	MaterialPrefab(std::string name);

private:
	std::string m_name;
};

#endif // !_MATERIALPREFAB_H_