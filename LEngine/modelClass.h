////////////////////////////////////////////////////////////////////////////////
// Filename: modelclass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _MODELCLASS_H_
#define _MODELCLASS_H_


//////////////
// INCLUDES //
//////////////
#include <d3d11.h>
#include <DirectXMath.h>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

using namespace DirectX;

////////////////////////////////////////////////////////////////////////////////
// Class name: ModelClass
////////////////////////////////////////////////////////////////////////////////
class ModelClass
{
private:
	struct VertexType
	{
		XMFLOAT3 position;
		XMFLOAT2 tex;
		XMFLOAT3 normal;
		XMFLOAT3 tangent;
		XMFLOAT3 binormal;
	};

public:
	ModelClass();
	ModelClass(const ModelClass&);
	~ModelClass();

	bool Initialize(ID3D11Device* device, const char* modelFilename);
	void Shutdown();
	void Render(ID3D11DeviceContext*);

	int GetIndexCount();

private:
	bool InitializeBuffers(ID3D11Device* device, const char* modelFilename);
	void ShutdownBuffers();
	void RenderBuffers(ID3D11DeviceContext*);
	void SetIndices(std::string input, int &vertexIndex, int &textureIndex, int &normalIndex);
	void CalculateDataForNormalMapping(VertexType* &vertices);
	void CalculateTangentBinormal(VertexType vertex1, VertexType vertex2, VertexType vertex3, XMFLOAT3 &tangent, XMFLOAT3 &binormal);
	void CalculateNormal(XMFLOAT3 &tangent, XMFLOAT3 &binormal, XMFLOAT3 &normal);

private:
	ID3D11Buffer *m_vertexBuffer, *m_indexBuffer;
	int m_vertexCount, m_indexCount;
};

#endif