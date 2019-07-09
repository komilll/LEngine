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
#include "json11.hpp"
#include <shlobj.h>
#include <sstream>
#include "ModelPickerShader.h"
#include "d3dclass.h"

using namespace DirectX;

////////////////////////////////////////////////////////////////////////////////
// Class name: ModelClass
////////////////////////////////////////////////////////////////////////////////
class ModelClass;

class Bounds
{
public:
	float minX;
	float maxX;
	float minY;
	float maxY;
	float minZ;
	float maxZ;

	float GetSizeX() const {
		return (maxX - minX);
	}
	float GetSizeY() const {
		return (maxY - minY);
	}
	float GetSizeZ() const {
		return (maxZ - minZ);
	}

	float GetCenterX() const {
		return minX + (maxX - minX) * 0.5f;
	}
	float GetCenterY() const {
		return minY + (maxY - minY) * 0.5f;
	}
	float GetCenterZ() const {
		return minZ + (maxZ - minZ) * 0.5f;
	}

	XMFLOAT3 GetCenter() const {
		return{ GetCenterX(), GetCenterY(), GetCenterZ() };
	}

	XMFLOAT3 BoundingBoxSize(XMMATRIX& worldMatrix, XMMATRIX& viewMatrix, XMMATRIX& projectionMatrix);
	XMFLOAT3 GetMinBounds(ModelClass* const model) const;
	XMFLOAT3 GetMaxBounds(ModelClass* const model) const;
};

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
	enum class Axis {
		X, Y, Z
	};

	enum class ShapeSize : int
	{
		TRIANGLE = 0,
		RECTANGLE = 1,
		SQUARE = 2
	};

public:
	ModelClass();
	ModelClass(const ModelClass&);
	~ModelClass();

	bool Initialize(D3DClass * d3d, const char* modelFilename, bool pickable = true);
	bool Initialize(D3DClass* d3d, XMFLOAT3 origin, XMFLOAT3 destination);
	bool Initialize(ID3D11Device* device, ShapeSize shape, float left, float right, float top, float bottom, bool withTex = true, bool isEmpty = false, float borderWidth = 0.007f);
	bool Initialize(ID3D11Device* device, XMFLOAT3 leftMin, XMFLOAT3 leftMax, XMFLOAT3 rightMin, XMFLOAT3 rightMax);	
	bool InitializeWireframe(ID3D11Device* device, XMFLOAT3 min, XMFLOAT3 max);
	bool InitializeSquare(ID3D11Device* device, float centerX, float centerY, float size, bool isEmpty, bool withTex);
	void Shutdown();
	void Render(ID3D11DeviceContext* deviceContext);
	void CreateWireframe();
	const std::vector<ModelClass*>& GetWireframeList() const;

	int GetIndexCount();

	void SetPosition(float x, float y, float z);
	void SetPosition(XMFLOAT3 position);
	void SetScale(float x, float y, float z);
	void SetScale(XMFLOAT3 scale);
	void SetRotation(float x, float y, float z);

	XMFLOAT4 GetPosition() const;
	XMFLOAT3 GetPositionXYZ() const;
	XMFLOAT3 GetScale() const;
	XMFLOAT3 GetRotation() const;

	float* GetPositionRef();
	float* GetScaleRef();
	float* GetRotationRef();
	ModelClass::Bounds& GetBounds() { return bounds; };

	std::string GetName() const;
	std::string GetModelFilename() const;
	std::string GetSaveData() const;
	void LoadData();
	static ModelClass* LoadModel(D3DClass * d3d);
	void LoadModel();
	std::string LoadModelCalculatePath();
	void SaveVisibleName();

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
	void CalculateAxisBound(float & min, float & max, std::vector<float>& elements) const;

public:
	std::string m_name;
	bool m_selected;

private:
	std::string m_savedName;
	std::string m_modelFilename;
	std::string m_materialName;
	ID3D11Device* m_device;
	ID3D11Buffer *m_vertexBuffer, *m_indexBuffer;
	int m_vertexCount, m_indexCount;
	float m_position[4]{ 0.0f, 0.0f, 0.0f, 0.0f};
	float m_scale[3] { 1.0f, 1.0f, 1.0f };
	float m_rotation[3]{ 0.0f, 0.0f, 0.0f };
	D3DClass* m_D3D;

	Bounds bounds;
	std::vector<ModelClass*> m_wireframeModels;

	struct PrimitiveModel
	{
	public:
		/*ModelClass::ShapeSize*/ int shape;
		float left;
		float right;
		float top; 
		float bottom;
		bool withTex;
		bool isEmpty;
		float borderWidth;

		float centerX; 
		float centerY; 
		float size;

		PrimitiveModel() {};

		void SetRectangle(float Left, float Right, float Top, float Bottom, bool WithTex, bool IsEmpty, float BorderWidth)
		{
			shape = static_cast<int>(ModelClass::ShapeSize::RECTANGLE);
			left = Left;
			right = Right;
			top = Top;
			bottom = Bottom;
			withTex = WithTex;
			isEmpty = IsEmpty;
			borderWidth = BorderWidth;
		}
		void SetSquare(float CenterX, float CenterY, float Size, bool IsEmpty, bool WithTex) 
		{
			shape = static_cast<int>(ModelClass::ShapeSize::SQUARE);
			centerX = CenterX;
			centerY = CenterY;
			size = Size;
			isEmpty = IsEmpty;
			withTex = WithTex;
		}

		json11::Json to_json() const { 
			return json11::Json::array { shape, left, right, top, bottom, withTex, isEmpty, borderWidth, centerX, centerY, size }; }
	};
	PrimitiveModel m_primitiveModel;
};
#endif