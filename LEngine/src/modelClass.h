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
#include <algorithm>
#include <cctype>

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
	enum ShapeSize {
		TRIANGLE, RECTANGLE
	};

public:
	ModelClass();
	ModelClass(const ModelClass&);
	~ModelClass();

	bool Initialize(ID3D11Device* device, const char* modelFilename);
	bool Initialize(ID3D11Device* device, ShapeSize shape, float left, float right, float top, float bottom, bool withTex = true, bool isEmpty = false, float borderWidth = 0.007f);
	bool InitializeSquare(ID3D11Device* device, float centerX, float centerY, float size, bool isEmpty, bool withTex);
	void Shutdown();
	void Render(ID3D11DeviceContext*);

	int GetIndexCount();

	void SetPosition(float x, float y, float z);
	void SetPosition(XMFLOAT3 position);
	XMFLOAT4 GetPosition();

private:
	bool InitializeBuffers(ID3D11Device* device, const char* modelFilename);
	void ShutdownBuffers();
	void RenderBuffers(ID3D11DeviceContext*);
	void SetIndices(std::string input, int &vertexIndex, int &textureIndex, int &normalIndex);
	void CalculateDataForNormalMapping(VertexType* &vertices);
	void CalculateTangentBinormal(VertexType vertex1, VertexType vertex2, VertexType vertex3, XMFLOAT3 &tangent, XMFLOAT3 &binormal);
	void CalculateNormal(XMFLOAT3 &tangent, XMFLOAT3 &binormal, XMFLOAT3 &normal);
	bool CreateBuffers(ID3D11Device* device, VertexType * &vertices, unsigned long * &indices, int vertexCount, int indexCount);
	//Save and Load binary data format
	void SaveBinary(const char* modelFilename, std::vector<VertexType> &vertexType, std::vector<unsigned long> &vertexIndices);
	bool ReadBinary(const char* modelFilename, std::vector<VertexType> &vertexType, std::vector<unsigned long> &vertexIndices);

	//Create shapes
	bool CreateRectangle(ID3D11Device* device, float left, float right, float top, float bottom, bool withTex, bool isEmpty, float borderWidth);
	bool CreateTriangle(ID3D11Device* device, float left, float right, float top, float bottom);
	bool CreateSquare(ID3D11Device* device, float centerX, float centerY, float size, bool isEmpty, bool withTex);

	bool is_number(const std::string& s);
	void LoadNewIndex(std::string line, int& vIndex, int& vtIndex, int& vnIndex);
private:
	ID3D11Buffer *m_vertexBuffer, *m_indexBuffer;
	int m_vertexCount, m_indexCount;
	XMFLOAT4 m_position;
};

#endif