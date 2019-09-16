////////////////////////////////////////////////////////////////////////////////
// Filename: modelclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "modelclass.h"
#include <fbxsdk.h>

ModelClass::ModelClass(D3DClass * d3d, const char * modelFilename, XMFLOAT3 position, bool pickable)
{
	m_vertexBuffer = 0;
	m_indexBuffer = 0;
	Initialize(d3d, modelFilename, pickable);
	SetPosition(position);
}

bool ModelClass::Initialize(D3DClass* d3d, const char * modelFilename, bool pickable)
{
	m_D3D = d3d;
	m_device = d3d->GetDevice();
	m_modelFilename = modelFilename;
	if (!InitializeBuffers(m_device, modelFilename))
		return false;

	return true;
}

bool ModelClass::InitializeFBX(D3DClass * d3d, const char * modelFilename, bool pickable)
{
	m_D3D = d3d;
	m_device = d3d->GetDevice();
	m_modelFilename = modelFilename;
	if (LoadModelFBX(modelFilename) == E_FAIL)
		return false;

	return true;
}

bool ModelClass::Initialize(D3DClass * d3d, XMFLOAT3 origin, XMFLOAT3 destination)
{
	if (!d3d)
	{
		return false;
	}
	m_D3D = d3d;
	m_device = d3d->GetDevice();
	Initialize(m_D3D->GetDevice(), origin, destination, origin, destination);
	return true;
}

bool ModelClass::Initialize(ID3D11Device * device, ShapeSize shape, float left, float right, float top, float bottom, bool withTex, bool isEmpty, float borderWidth)
{
	m_device = device;
	m_primitiveModel.SetRectangle(left, right, top, bottom, withTex, isEmpty, borderWidth);
	switch (shape)
	{
		case ModelClass::ShapeSize::RECTANGLE:
			return CreateRectangle(device, left, right, top, bottom, withTex, isEmpty, borderWidth);
		case ModelClass::ShapeSize::TRIANGLE:
			return CreateTriangle(device, left, right, top, bottom);
		default:
			return false;
	}

	return false;
}

bool ModelClass::Initialize(ID3D11Device * device, XMFLOAT3 leftMin, XMFLOAT3 leftMax, XMFLOAT3 rightMin, XMFLOAT3 rightMax)
{
	m_device = device;
	VertexType* vertices;
	unsigned long* indices;

	int verticesCount = 6;

	float widthLeft = 0.007f;
	float widthTop = widthLeft * 16.0f / 9.0f;
	float zWidth = 0.00f;

	vertices = new VertexType[verticesCount];
	//First triangle
	vertices[0].position = leftMax;  // Top left.	
	vertices[1].position = rightMin;  // Bottom right.
	vertices[2].position = leftMin;  // Bottom left.
	//Second triangle
	vertices[3].position = leftMax;  // Top left.
	vertices[4].position = rightMax;  // Top right.
	vertices[5].position = rightMin;  // Bottom right.

	indices = new unsigned long[verticesCount];
	for (int i = 0; i < verticesCount; i++)
		indices[i] = i;

	m_indexCount = verticesCount;

	if (indices != 0)
	{
		if (CreateBuffers(device, vertices, indices, verticesCount, verticesCount) == false)
			return false;
	}

	return true;
}

bool ModelClass::InitializeWireframe(ID3D11Device * device, XMFLOAT3 min, XMFLOAT3 max)
{
	m_device = device;

	const float left = min.x;
	const float right = max.x;
	const float top = max.y;
	const float bottom = min.y;
	const float zMin = min.z;
	const float zMax = max.z;

	constexpr float widthLeft = 0.003f;
	constexpr float widthTop = widthLeft * 16.0f / 9.0f;

	constexpr int verticesCount = 6;
	m_indexCount = verticesCount;
	constexpr std::array<unsigned long, 6> indices{ 0, 1, 2, 3, 4, 5 };
	std::array<VertexType, 6> vertices;

	//First triangle
	vertices[0].position = XMFLOAT3(left - widthLeft, top + widthTop, zMax);  // Top left.	
	vertices[1].position = XMFLOAT3(right, bottom - widthTop, zMin);  // Bottom right.
	vertices[2].position = XMFLOAT3(left - widthLeft, bottom - widthTop, zMin);  // Bottom left.
	//Second triangle
	vertices[3].position = XMFLOAT3(left - widthLeft, top + widthTop, zMax);  // Top left.
	vertices[4].position = XMFLOAT3(right, top + widthTop, zMax);  // Top right.
	vertices[5].position = XMFLOAT3(right, bottom - widthTop, zMin);  // Bottom right.

	return CreateBuffers(device, &vertices[0], &indices[0], verticesCount, verticesCount);
}

bool ModelClass::InitializeSquare(ID3D11Device * device, float centerX, float centerY, float size, bool isEmpty, bool withTex)
{
	m_device = device;
	m_primitiveModel.SetSquare(centerX, centerY, size, isEmpty, withTex);
	return CreateSquare(device, centerX, centerY, size, isEmpty, withTex);
}

void ModelClass::Shutdown()
{
	ShutdownBuffers();
}


void ModelClass::Render(ID3D11DeviceContext* deviceContext)
{
	RenderBuffers(deviceContext);
}

void ModelClass::CreateWireframe()
{
	//Front
	{
		ModelClass* model = new ModelClass;
		model->Initialize(m_D3D->GetDevice(), { bounds.minX, bounds.minY, bounds.minZ }, { bounds.minX, bounds.maxY, bounds.minZ }, { bounds.maxX, bounds.minY, bounds.minZ },
		{ bounds.maxX, bounds.maxY, bounds.minZ });
		m_wireframeModels.push_back(std::move(model));
	}
	//Back
	{
		ModelClass* model = new ModelClass;
		model->Initialize(m_D3D->GetDevice(), { bounds.minX, bounds.minY, bounds.maxZ }, { bounds.minX, bounds.maxY, bounds.maxZ }, { bounds.maxX, bounds.minY, bounds.maxZ },
		{ bounds.maxX, bounds.maxY, bounds.maxZ });
		m_wireframeModels.push_back(std::move(model));
	}
	//Right
	{
		ModelClass* model = new ModelClass;
		model->Initialize(m_D3D->GetDevice(), { bounds.maxX, bounds.minY, bounds.minZ }, { bounds.maxX, bounds.maxY, bounds.minZ }, { bounds.maxX, bounds.minY, bounds.maxZ },
		{ bounds.maxX, bounds.maxY, bounds.maxZ });
		m_wireframeModels.push_back(std::move(model));
	}
	//Left
	{
		ModelClass* model = new ModelClass;
		model->Initialize(m_D3D->GetDevice(), { bounds.minX, bounds.minY, bounds.minZ }, { bounds.minX, bounds.maxY, bounds.minZ }, { bounds.minX, bounds.minY, bounds.maxZ },
		{ bounds.minX, bounds.maxY, bounds.maxZ });
		m_wireframeModels.push_back(std::move(model));
	}
	//Top
	{
		ModelClass* model = new ModelClass;
		model->Initialize(m_D3D->GetDevice(), { bounds.minX, bounds.maxY, bounds.minZ }, { bounds.minX, bounds.maxY, bounds.maxZ }, { bounds.maxX, bounds.maxY, bounds.minZ },
		{ bounds.maxX, bounds.maxY, bounds.maxZ });
		m_wireframeModels.push_back(std::move(model));
	}
	//Bottom
	{
		ModelClass* model = new ModelClass;
		model->Initialize(m_D3D->GetDevice(), { bounds.minX, bounds.minY, bounds.minZ }, { bounds.minX, bounds.minY, bounds.maxZ }, { bounds.maxX, bounds.minY, bounds.minZ },
		{ bounds.maxX, bounds.minY, bounds.maxZ });
		m_wireframeModels.push_back(std::move(model));
	}
}

const std::vector<ModelClass*>& ModelClass::GetWireframeList() const
{
	return m_wireframeModels;
}

int ModelClass::GetIndexCount() const
{
	return m_indexCount;
}

void ModelClass::SetPosition(const float x, const float y, const float z)
{
	SetPosition(XMFLOAT3{ x, y, z });
}

void ModelClass::SetPosition(const XMFLOAT3 position)
{
	m_position.x = position.x;
	m_position.y = position.y;
	m_position.z = position.z;
	m_position.w = 1.0f;
}

void ModelClass::SetScale(float x, float y, float z)
{
	m_scale = { x, y, z };
}

void ModelClass::SetScale(XMFLOAT3 scale)
{
	m_scale = scale;
}

void ModelClass::SetRotation(XMFLOAT3 rotation)
{
	m_rotation = rotation;
}

void ModelClass::SetRotation(float x, float y, float z)
{
	SetRotation(XMFLOAT3{ x, y, z });
}

XMFLOAT4 ModelClass::GetPosition() const
{
	return{ m_position.x, m_position.y, m_position.z, m_position.w };
}

XMFLOAT3 ModelClass::GetPositionXYZ() const
{
	return{ m_position.x, m_position.y, m_position.z };
}

XMFLOAT3 ModelClass::GetScale() const
{
	return m_scale;
}

XMFLOAT3 ModelClass::GetRotation() const
{
	return{ m_rotation.x + (m_isFBX ? 90.0f : 0.0f), m_rotation.y + (m_isFBX ? 180.0f : 0.0f), m_rotation.z + (m_isFBX ? 180.0f : 0.0f) };
}

float* ModelClass::GetPositionRef()
{
	return &m_position.x;
}

float* ModelClass::GetScaleRef()
{
	return &m_scale.x;
}

float* ModelClass::GetRotationRef()
{
	return &m_rotation.x;
}

std::string ModelClass::GetName() const
{
	return m_savedName;
}

std::string ModelClass::GetModelFilename() const
{
	return m_modelFilename;
}

std::string ModelClass::GetSaveData() const
{
	json11::Json obj = json11::Json::object{
		{"modelName", m_modelFilename},
		{"sceneName", m_savedName}, 
		{"position", json11::Json::array{m_position.x, m_position.y, m_position.z}},
		{"scale", json11::Json::array{m_scale.x, m_scale.y, m_scale.z}},
		{"rotation", json11::Json::array{m_rotation.x, m_rotation.y, m_rotation.z}},
		{"material", m_materialName}
	};
	return obj.dump();
}

ModelClass* ModelClass::LoadModel(D3DClass * d3d)
{
	ModelClass* model = new ModelClass();
	model->Initialize(d3d, model->LoadModelCalculatePath().c_str());
	model->m_name = "Model";
	return model;
}

void ModelClass::LoadModel()
{
	assert(m_D3D);
	Initialize(m_D3D, LoadModelCalculatePath().c_str());
}

HRESULT ModelClass::LoadModelFBX(const char* modelFilename)
{
	//Rotate to LH later on
	m_isFBX = true;

	//std::unique_ptr<FbxManager> fbxManager = make_unique<FbxManager>(FbxManager::Create());
	static FbxManager* fbxManager;
	if (!fbxManager)
	{
		fbxManager = FbxManager::Create();
		static FbxIOSettings* ioSettings = FbxIOSettings::Create(fbxManager, IOSROOT);
		fbxManager->SetIOSettings(ioSettings);
	}

	FbxImporter* importer = FbxImporter::Create(fbxManager, "");
	FbxScene* fbxScene = FbxScene::Create(fbxManager, "");

	if (!importer->Initialize(modelFilename, -1, fbxManager->GetIOSettings()))
		return E_FAIL;

	if (!importer->Import(fbxScene))
		return E_FAIL;

	importer->Destroy();

	VertexType* vertices{ nullptr };
	unsigned long* indices;
	std::vector<VertexType> verticesVector(0);
	std::vector<unsigned long> indicesVec;
	int m_vertexCount = 0;

	if (!ReadBinary(modelFilename, verticesVector, indicesVec))
	{
		FbxNode* fbxRootNode = fbxScene->GetRootNode();
		if (fbxRootNode)
		{
			for (int i = 0; i < fbxRootNode->GetChildCount(); ++i)
			{
				FbxNode* fbxChildNode = fbxRootNode->GetChild(i);

				if (fbxChildNode->GetNodeAttribute() == NULL)
					continue;

				FbxNodeAttribute::EType attributeType = fbxChildNode->GetNodeAttribute()->GetAttributeType();

				if (attributeType != FbxNodeAttribute::eMesh)
					continue;

				FbxGeometryConverter converter(fbxChildNode->GetFbxManager());
				converter.Triangulate(fbxScene, true);

				FbxMesh* mesh = static_cast<FbxMesh*>(fbxChildNode->GetNodeAttribute());
				FbxVector4* fbxVertices = mesh->GetControlPoints();

				std::vector<DirectX::XMFLOAT3> vertexPosition;
				std::vector<DirectX::XMFLOAT3> normalPosition;
				std::vector<DirectX::XMFLOAT2> texPosition;

				std::vector<float> positionAxis_X;
				std::vector<float> positionAxis_Y;
				std::vector<float> positionAxis_Z;

				for (int j = 0; j < mesh->GetPolygonCount(); ++j)
				{
					int numVertices = mesh->GetPolygonSize(j);
					//assert(numVertices == 3);

					for (int k = 0; k < numVertices; ++k)
					{
						const int controlPointIndex = mesh->GetPolygonVertex(j, k);

						const float xPos = static_cast<float>(fbxVertices[controlPointIndex].mData[0]);
						const float yPos = static_cast<float>(fbxVertices[controlPointIndex].mData[1]);
						const float zPos = static_cast<float>(fbxVertices[controlPointIndex].mData[2]);

						vertexPosition.push_back(DirectX::XMFLOAT3(xPos, yPos, zPos));
						positionAxis_X.emplace_back(xPos);
						positionAxis_Y.emplace_back(yPos);
						positionAxis_Z.emplace_back(zPos);
					}
				}

				const auto normal = mesh->GetElementNormal();
				if (normal)
				{
					if (normal->GetMappingMode() == FbxGeometryElement::eByControlPoint)
					{
						//Let's get normals of each vertex, since the mapping mode of normal element is by control point
						for (int lVertexIndex = 0; lVertexIndex < mesh->GetControlPointsCount(); lVertexIndex++)
						{
							int lNormalIndex = 0;
							if (normal->GetReferenceMode() == FbxGeometryElement::eDirect)
								lNormalIndex = lVertexIndex;

							//reference mode is index-to-direct, get normals by the index-to-direct
							if (normal->GetReferenceMode() == FbxGeometryElement::eIndexToDirect)
								lNormalIndex = normal->GetIndexArray().GetAt(lVertexIndex);

							//Got normals of each vertex.
							FbxVector4 lNormal = normal->GetDirectArray().GetAt(lNormalIndex);
							normalPosition.push_back({ static_cast<float>(lNormal.mData[0]), static_cast<float>(lNormal.mData[1]), static_cast<float>(lNormal.mData[2]) });
						}
					}
					else if (normal->GetMappingMode() == FbxGeometryElement::eByPolygonVertex)
					{
						int lIndexByPolygonVertex = 0;
						//Let's get normals of each polygon, since the mapping mode of normal element is by polygon-vertex.
						for (int lPolygonIndex = 0; lPolygonIndex < mesh->GetPolygonCount(); lPolygonIndex++)
						{
							//get polygon size, you know how many vertices in current polygon.
							int lPolygonSize = mesh->GetPolygonSize(lPolygonIndex);
							//retrieve each vertex of current polygon.
							for (int i = 0; i < lPolygonSize; i++)
							{
								int lNormalIndex = 0;
								//reference mode is direct, the normal index is same as lIndexByPolygonVertex.
								if (normal->GetReferenceMode() == FbxGeometryElement::eDirect)
									lNormalIndex = lIndexByPolygonVertex;

								//reference mode is index-to-direct, get normals by the index-to-direct
								if (normal->GetReferenceMode() == FbxGeometryElement::eIndexToDirect)
									lNormalIndex = normal->GetIndexArray().GetAt(lIndexByPolygonVertex);

								//Got normals of each polygon-vertex.
								FbxVector4 lNormal = normal->GetDirectArray().GetAt(lNormalIndex);
								normalPosition.push_back({ static_cast<float>(lNormal.mData[0]), static_cast<float>(lNormal.mData[1]), static_cast<float>(lNormal.mData[2]) });

								lIndexByPolygonVertex++;
							}
						}
					}
				}

				//get all UV set names
				FbxStringList lUVSetNameList;
				mesh->GetUVSetNames(lUVSetNameList);

				//iterating over all uv sets
				for (int lUVSetIndex = 0; lUVSetIndex < lUVSetNameList.GetCount(); lUVSetIndex++)
				{
					//get lUVSetIndex-th uv set
					const char* lUVSetName = lUVSetNameList.GetStringAt(lUVSetIndex);
					const FbxGeometryElementUV* lUVElement = mesh->GetElementUV(lUVSetName);

					if (!lUVElement)
						continue;

					// only support mapping mode eByPolygonVertex and eByControlPoint
					if (lUVElement->GetMappingMode() != FbxGeometryElement::eByPolygonVertex &&
						lUVElement->GetMappingMode() != FbxGeometryElement::eByControlPoint)
						return false;

					//index array, where holds the index referenced to the uv data
					const bool lUseIndex = lUVElement->GetReferenceMode() != FbxGeometryElement::eDirect;
					const int lIndexCount = (lUseIndex) ? lUVElement->GetIndexArray().GetCount() : 0;

					//iterating through the data by polygon
					const int lPolyCount = mesh->GetPolygonCount();

					if (lUVElement->GetMappingMode() == FbxGeometryElement::eByControlPoint)
					{
						for (int lPolyIndex = 0; lPolyIndex < lPolyCount; ++lPolyIndex)
						{
							// build the max index array that we need to pass into MakePoly
							const int lPolySize = mesh->GetPolygonSize(lPolyIndex);
							for (int lVertIndex = 0; lVertIndex < lPolySize; ++lVertIndex)
							{
								FbxVector2 lUVValue;

								//get the index of the current vertex in control points array
								int lPolyVertIndex = mesh->GetPolygonVertex(lPolyIndex, lVertIndex);

								//the UV index depends on the reference mode
								int lUVIndex = lUseIndex ? lUVElement->GetIndexArray().GetAt(lPolyVertIndex) : lPolyVertIndex;

								lUVValue = lUVElement->GetDirectArray().GetAt(lUVIndex);

								texPosition.push_back({ static_cast<float>(lUVValue.mData[0]), static_cast<float>(lUVValue.mData[1]) });
							}
						}
					}
					else if (lUVElement->GetMappingMode() == FbxGeometryElement::eByPolygonVertex)
					{
						int lPolyIndexCounter = 0;
						for (int lPolyIndex = 0; lPolyIndex < lPolyCount; ++lPolyIndex)
						{
							// build the max index array that we need to pass into MakePoly
							const int lPolySize = mesh->GetPolygonSize(lPolyIndex);
							for (int lVertIndex = 0; lVertIndex < lPolySize; ++lVertIndex)
							{
								if (lPolyIndexCounter < lIndexCount)
								{
									FbxVector2 lUVValue;

									//the UV index depends on the reference mode
									int lUVIndex = lUseIndex ? lUVElement->GetIndexArray().GetAt(lPolyIndexCounter) : lPolyIndexCounter;

									lUVValue = lUVElement->GetDirectArray().GetAt(lUVIndex);

									texPosition.push_back({ static_cast<float>(lUVValue.mData[0]), static_cast<float>(lUVValue.mData[1]) });

									lPolyIndexCounter++;
								}
							}
						}
					}
				}

				CalculateAxisBound(bounds.minX, bounds.maxX, positionAxis_X);
				CalculateAxisBound(bounds.minY, bounds.maxY, positionAxis_Y);
				CalculateAxisBound(bounds.minZ, bounds.maxZ, positionAxis_Z);

				vertices = new VertexType[m_vertexCount = vertexPosition.size()];
				for (int i = 0; i < m_vertexCount; i++)
				{
					vertices[i].position = vertexPosition.at(i);
					vertices[i].normal = normalPosition.at(i);
					vertices[i].tex = texPosition.at(i);
				}
			}
		}
		fbxRootNode->Destroy();
	}
	else
	{
		m_vertexCount = verticesVector.size();
	}

	auto m_indicesCount = m_vertexCount;
	m_indexCount = m_indicesCount;
	indices = new unsigned long[m_indexCount];
	for (int i = 0; i < m_indexCount; i++)
	{
		indices[i] = i;
		indicesVec.push_back(i);
	}

	if (CreateBuffers(m_D3D->GetDevice(), vertices, indices, m_vertexCount, m_indexCount) == false)
		return false;

	if (verticesVector.size() == 0)
	{
		for (int i = 0; i < m_vertexCount; i++)
			verticesVector.push_back(vertices[i]);

		SaveBinary(modelFilename, verticesVector, indicesVec);
	}

	//Release unused data
	delete[] vertices;
	vertices = 0;

	delete[] indices;
	indices = 0;

	return S_OK;
}

bool ModelClass::InitializeBuffers(ID3D11Device* device, const char* modelFilename)
{
	//TODO Optimize whole method to improve model loading times
	std::vector<VertexType> verticesVector(0);
	VertexType* vertices{ nullptr };
	unsigned long* indices;

	/////////// GET VERTEX COUNT ////////
	int m_vertexCount = 0;

	///////// CORRECT VERTICES LOADING ////////
	std::vector<DirectX::XMFLOAT3> vertexPosition;
	std::vector<DirectX::XMFLOAT2> texPosition;
	std::vector<DirectX::XMFLOAT3> normalPosition;
	std::vector<int> texIndices;
	std::vector<int> normalIndices;
	std::vector<unsigned long> indicesVec;

	std::string filename = modelFilename;
	const std::size_t lastPos = filename.find(".obj");
	filename.erase(lastPos + 4);
	modelFilename = filename.c_str();
	m_modelFilename = filename.c_str();

#pragma region Read and save data from file
	if (!ReadBinary(modelFilename, verticesVector, indicesVec))
	{
		std::vector<float> positionAxis_X;
		std::vector<float> positionAxis_Y;
		std::vector<float> positionAxis_Z;

		if (std::ifstream input{ modelFilename, std::ios::binary })
		{
			std::string x, y, z;
			std::string type = "";
			while (input >> type)
			{
				if (type == "")
				{
					continue;
				}

				if (type == "v")
				{
					input >> x >> y >> z;
					const float xPos = std::stof(x);
					const float yPos = std::stof(y);
					const float zPos = std::stof(z);

					vertexPosition.push_back(DirectX::XMFLOAT3(xPos, yPos, zPos));
					positionAxis_X.emplace_back(xPos);
					positionAxis_Y.emplace_back(yPos);
					positionAxis_Z.emplace_back(zPos);
				}
				else if (type == "vt")
				{
					input >> x >> y;
					texPosition.push_back(DirectX::XMFLOAT2(std::stof(x), std::stof(y)));
				}
				else if (type == "vn")
				{
					input >> x >> y >> z;
					normalPosition.push_back(DirectX::XMFLOAT3(std::stof(x), std::stof(y), std::stof(z)));
				}
			}

			vertices = new VertexType[m_vertexCount = vertexPosition.size()];
			for (int i = 0; i < m_vertexCount; i++)
			{
				vertices[i].position = vertexPosition.at(i);
			}
		}

		CalculateAxisBound(bounds.minX, bounds.maxX, positionAxis_X);
		CalculateAxisBound(bounds.minY, bounds.maxY, positionAxis_Y);
		CalculateAxisBound(bounds.minZ, bounds.maxZ, positionAxis_Z);

			////////// FEEDING INDICES WITH DATA /////////
		if (std::ifstream input{ modelFilename, std::ios::binary })
		{
			std::string line = "";
			int left = 0;
			
			int vIndex = 0;
			int vtIndex = 0;
			int vnIndex = 0;
			//while (std::getline(input, line))
			while (input >> line)
			{
				//std::istringstream iss(line);
				//iss >> type;

				if (line == "f")
				{
					left = 3;
					if (texPosition.size() == 0)
						vtIndex = -1;
				}
				else if (left > 0)
				{
					left--;
					SetIndices(line, vIndex, vtIndex, vnIndex);
					indicesVec.push_back(vIndex);
					if (texPosition.size() > 0 && m_vertexCount > vIndex && texPosition.size() > vtIndex)
					{
						vertices[vIndex].tex = texPosition.at(vtIndex);
						texIndices.push_back(vtIndex);
					}
					if (normalPosition.size() > 0 && m_vertexCount > vIndex && normalPosition.size() > vnIndex)
					{
						vertices[vIndex].normal = normalPosition.at(vnIndex);
						normalIndices.push_back(vnIndex);
					}
				}
			}
		}
	}
	else
	{
		vertices = new VertexType[m_vertexCount = verticesVector.size()];
		for (int i = 0 ; i < m_vertexCount; i++)
			vertices[i] = verticesVector.at(i);
	}
#pragma endregion

	auto m_indicesCount = indicesVec.size();
	if (m_indicesCount == 0)
		m_indicesCount = verticesVector.size();

	m_indexCount = m_indicesCount;
	indices = new unsigned long[m_indexCount];
	if (indicesVec.size() > 0)
	{
		for (int i = 0; i < m_indexCount; i++)
			indices[i] = indicesVec.at(i);
	}
	else
	{
		for (int i = 0; i < m_indexCount; i++)
			indices[i] = i;
	}

	//CalculateDataForNormalMapping(vertices);

	if (CreateBuffers(device, vertices, indices, m_vertexCount, m_indexCount) == false)
		return false;

	if (verticesVector.size() == 0)
	{
		for (int i = 0; i < m_vertexCount; i++)
			verticesVector.push_back(vertices[i]);

		//SaveBinary(vertexPosition, texPosition, normalPosition, indicesVec, texIndices, normalIndices);
		SaveBinary(modelFilename, verticesVector, indicesVec);
	}

	//Release unused data
	delete[] vertices;
	vertices = 0;

	delete[] indices;
	indices = 0;

	return true;
}


void ModelClass::ShutdownBuffers()
{
	//TODO Upgrade or remove completely

	// Release the index buffer.
	if(m_indexBuffer)
	{
		m_indexBuffer->Release();
		m_indexBuffer = 0;
	}

	// Release the vertex buffer.
	if(m_vertexBuffer)
	{
		m_vertexBuffer->Release();
		m_vertexBuffer = 0;
	}

	return;
}


void ModelClass::RenderBuffers(ID3D11DeviceContext* deviceContext)
{
	constexpr unsigned int stride = sizeof(VertexType);
	constexpr unsigned int offset = 0;

	deviceContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);
	deviceContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void ModelClass::SetIndices(std::string input, int & vertexIndex, int & textureIndex, int & normalIndex)
{
	std::string curOut = "";
	std::stringstream newStream(input);
	int curIndex = 0;
	while (std::getline(newStream, curOut, '/'))
	{
		int index = -1;
		if (curOut != "" && is_number(curOut))
		{
			index += stoi(curOut);
		}
		else
		{
			return;
		}
		
		if (curIndex == 0)
			vertexIndex = index;
		else if (curIndex == 1 && textureIndex != -1)
			textureIndex = index;
		else
			normalIndex = index;

		curIndex++;
	}
}

bool ModelClass::CreateBuffers(ID3D11Device* device, const VertexType *vertices, const unsigned long *indices, int vertexCount, int indexCount)
{
	D3D11_BUFFER_DESC vertexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData;
	//Create vertex buffer description
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(VertexType) * vertexCount;
 	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;
	//Fill subresource data with vertices
	vertexData.pSysMem = vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;
	//Try to create vertex buffer and store it in varaible
	if (FAILED(device->CreateBuffer(&vertexBufferDesc, &vertexData, &m_vertexBuffer)))
	{
		return false;
	}

	D3D11_BUFFER_DESC indexBufferDesc;
	D3D11_SUBRESOURCE_DATA indexData;
	//Create index buffer description
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(unsigned long) * indexCount;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;
	//Fill subresource data with indices
	indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;
	//Try to create index buffer and store it in varaible
	if (FAILED(device->CreateBuffer(&indexBufferDesc, &indexData, &m_indexBuffer)))
	{
		return false;
	}

	return true;
}

/////// BINARY FILES MANAGMENT ///////
void ModelClass::SaveBinary(const char* modelFilename, std::vector<VertexType> &vertexType, std::vector<unsigned long> &vertexIndices)
{
	std::string newFilename = modelFilename;
	newFilename = newFilename.substr(0, newFilename.find("."));
	newFilename += ".modelclass";

	if (std::ofstream output{ "Models/" + newFilename, std::ios::binary })
	{
		const int vertSize = vertexType.size();
		output.write(reinterpret_cast<const char*>(&vertSize), sizeof(vertSize));
		const int indSize = vertexIndices.size();
		output.write(reinterpret_cast<const char*>(&indSize), sizeof(indSize));

		for (const auto& val : vertexType)
			output.write(reinterpret_cast<const char*>(&val), sizeof(val));
		for (const auto& val : vertexIndices)
			output.write(reinterpret_cast<const char*>(&val), sizeof(val));

		//Save bounds AABB
		output.write(reinterpret_cast<const char*>(&bounds.maxX), sizeof(bounds.maxX));
		output.write(reinterpret_cast<const char*>(&bounds.maxY), sizeof(bounds.maxY));
		output.write(reinterpret_cast<const char*>(&bounds.maxZ), sizeof(bounds.maxZ));
		output.write(reinterpret_cast<const char*>(&bounds.minX), sizeof(bounds.minX));
		output.write(reinterpret_cast<const char*>(&bounds.minZ), sizeof(bounds.minY));
		output.write(reinterpret_cast<const char*>(&bounds.minY), sizeof(bounds.minZ));
	}
}

bool ModelClass::ReadBinary(const char* modelFilename, std::vector<VertexType> &vertexType, std::vector<unsigned long> &vertexIndices)
{
	std::string newFilename = modelFilename;
	newFilename = newFilename.substr(0, newFilename.find("."));
	newFilename += ".modelclass";

	if (std::ifstream input{ "Models/" + newFilename, std::ios::binary })
	{
		int sizeVertices;
		int sizeIndices;
		input.read(reinterpret_cast<char*>(&sizeVertices), sizeof(sizeVertices));
		input.read(reinterpret_cast<char*>(&sizeIndices), sizeof(sizeIndices));

		{
			VertexType valVertex;
			for (int i = 0; i < sizeVertices; i++)
			{
				input.read(reinterpret_cast<char*>(&valVertex), sizeof(valVertex));
				vertexType.push_back(valVertex);
			}
		}
		{
			long valIndices{ 0 };
			for (int i = 0; i < sizeIndices; i++)
			{
				input.read(reinterpret_cast<char*>(&valIndices), sizeof(valIndices));
				vertexIndices.push_back(valIndices);
			}
		}

		//Load bounds AABB
		input.read(reinterpret_cast<char*>(&bounds.maxX), sizeof(bounds.maxX));
		input.read(reinterpret_cast<char*>(&bounds.maxY), sizeof(bounds.maxY));
		input.read(reinterpret_cast<char*>(&bounds.maxZ), sizeof(bounds.maxZ));
		input.read(reinterpret_cast<char*>(&bounds.minX), sizeof(bounds.minX));
		input.read(reinterpret_cast<char*>(&bounds.minZ), sizeof(bounds.minY));
		input.read(reinterpret_cast<char*>(&bounds.minY), sizeof(bounds.minZ));
		return true;
	}
	return false;
}

/////// SHAPES DRAWING ///////
bool ModelClass::CreateRectangle(ID3D11Device* device, float left, float right, float top, float bottom, bool withTex, bool isEmpty, float borderWidth)
{
	VertexType* vertices;
	unsigned long* indices;

	int verticesCount = 6;

	float widthLeft = borderWidth;
	float widthTop = widthLeft * 16.0f / 9.0f;

	vertices = new VertexType[verticesCount];
	if (isEmpty == false)
	{
		//First triangle
		vertices[0].position = XMFLOAT3(left, top, 0.0f);  // Top left.	
		vertices[1].position = XMFLOAT3(right, bottom, 0.0f);  // Bottom right.
		vertices[2].position = XMFLOAT3(left, bottom, 0.0f);  // Bottom left.
		//Second triangle
		vertices[3].position = XMFLOAT3(left, top, 0.0f);  // Top left.
		vertices[4].position = XMFLOAT3(right, top, 0.0f);  // Top right.
		vertices[5].position = XMFLOAT3(right, bottom, 0.0f);  // Bottom right.

		if (withTex) //Create tex values for vertices
		{
			//First triangle	
			vertices[0].tex = XMFLOAT2(0.0, 0.0);  // Top left.	
			vertices[1].tex = XMFLOAT2(1.0, 1.0);  // Bottom right.
			vertices[2].tex = XMFLOAT2(0.0, 1.0);  // Bottom left.
			//Second triangle
			vertices[3].tex = XMFLOAT2(0.0, 0.0);  // Top left.
			vertices[4].tex = XMFLOAT2(1.0, 0.0);  // Top right.
			vertices[5].tex = XMFLOAT2(1.0, 1.0);  // Bottom right.
		}
	}
	else
	{
		verticesCount = 24;
		vertices = new VertexType[verticesCount];
		////// LEFT //////
		//First triangle
		vertices[0].position = XMFLOAT3(left - widthLeft, top + widthTop, 0.0f);  // Top left.	
		vertices[1].position = XMFLOAT3(left, bottom - widthTop, 0.0f);  // Bottom right.
		vertices[2].position = XMFLOAT3(left - widthLeft, bottom - widthTop, 0.0f);  // Bottom left.
		//Second triangle
		vertices[3].position = XMFLOAT3(left - widthLeft, top + widthTop, 0.0f);  // Top left.
		vertices[4].position = XMFLOAT3(left, top + widthTop, 0.0f);  // Top right.
		vertices[5].position = XMFLOAT3(left, bottom - widthTop, 0.0f);  // Bottom right.
		////// TOP //////
		//First triangle
		vertices[6].position = XMFLOAT3(left - widthLeft, top + widthTop, 0.0f);  // Top left.	
		vertices[7].position = XMFLOAT3(right + widthLeft, top, 0.0f);  // Bottom right.
		vertices[8].position = XMFLOAT3(left - widthLeft, top, 0.0f);  // Bottom left.
		//Second triangle
		vertices[9].position = XMFLOAT3(left - widthLeft, top + widthTop, 0.0f);  // Top left.
		vertices[10].position = XMFLOAT3(right + widthLeft, top + widthTop, 0.0f);  // Top right.
		vertices[11].position = XMFLOAT3(right + widthLeft, top, 0.0f);  // Bottom right.
		////// RIGHT //////
		//First triangle
		vertices[12].position = XMFLOAT3(right, top + widthTop, 0.0f);  // Top left.	
		vertices[13].position = XMFLOAT3(right + widthLeft, bottom - widthTop, 0.0f);  // Bottom right.
		vertices[14].position = XMFLOAT3(right, bottom - widthTop, 0.0f);  // Bottom left.
		//Second triangle
		vertices[15].position = XMFLOAT3(right, top + widthTop, 0.0f);  // Top left.
		vertices[16].position = XMFLOAT3(right + widthLeft, top + widthTop, 0.0f);  // Top right.
		vertices[17].position = XMFLOAT3(right + widthLeft, bottom - widthTop, 0.0f);  // Bottom right.
		////// BOTTOM //////
		//First triangle
		vertices[18].position = XMFLOAT3(left - widthLeft, bottom, 0.0f);  // Top left.	
		vertices[19].position = XMFLOAT3(right + widthLeft, bottom - widthTop, 0.0f);  // Bottom right.
		vertices[20].position = XMFLOAT3(left - widthLeft, bottom - widthTop, 0.0f);  // Bottom left.
		//Second triangle
		vertices[21].position = XMFLOAT3(left - widthLeft, bottom, 0.0f);  // Top left.
		vertices[22].position = XMFLOAT3(right + widthLeft, bottom, 0.0f);  // Top right.
		vertices[23].position = XMFLOAT3(right + widthLeft, bottom - widthTop, 0.0f);  // Bottom right.
	}

	indices = new unsigned long[verticesCount];
	for (int i = 0; i < verticesCount; i++)
		indices[i] = i;

	m_indexCount = verticesCount;

	if (indices != 0)
	{
		if (CreateBuffers(device, vertices, indices, verticesCount, verticesCount) == false)
			return false;
	}

	return true;
}

bool ModelClass::CreateTriangle(ID3D11Device * device, float left, float right, float top, float bottom)
{
	constexpr int verticesCount = 3;
	m_indexCount = verticesCount;

	constexpr std::array<unsigned long, 3> indices{ 0, 1, 2 };
	std::array<VertexType, 3> vertices;

	//Only triangle
	vertices[0].position = XMFLOAT3((left + right) / 2, top, 0.0f);  // Top	
	vertices[1].position = XMFLOAT3(right, bottom, 0.0f);  // Bottom right.
	vertices[2].position = XMFLOAT3(left, bottom, 0.0f);  // Bottom left.

	return CreateBuffers(device, &vertices[0], &indices[0], verticesCount, verticesCount);
}

bool ModelClass::CreateSquare(ID3D11Device * device, float centerX, float centerY, float size, bool isEmpty, bool withTex)
{
	constexpr int verticesCountEmpty = 24;
	constexpr int verticesCountFilled = 6;

	const float left = centerX - size * 0.5f;
	const float right = centerX + size * 0.5f;
	const float top = centerY / 2 + size * 0.5f * (16.0f / 9.0f);
	const float bottom = centerY / 2 - size * 0.5f * (16.0f / 9.0f);

	constexpr float widthLeft = 0.003f;
	constexpr float widthTop = widthLeft * 16.0f / 9.0f;

	if (isEmpty == false)
	{
		std::array<VertexType, verticesCountEmpty> vertices;
		std::array<unsigned long, verticesCountEmpty> indices;
		for (int i = 0; i < verticesCountEmpty; i++)
			indices[i] = i;

		//First triangle	
		vertices[0].position = XMFLOAT3(left, top, 0.0f);  // Top left.	
		vertices[1].position = XMFLOAT3(right, bottom, 0.0f);  // Bottom right.
		vertices[2].position = XMFLOAT3(left, bottom, 0.0f);  // Bottom left.
		//Second triangle
		vertices[3].position = XMFLOAT3(left, top, 0.0f);  // Top left.
		vertices[4].position = XMFLOAT3(right, top, 0.0f);  // Top right.
		vertices[5].position = XMFLOAT3(right, bottom, 0.0f);  // Bottom right.

		if (withTex) //Create tex values for vertices
		{
			//First triangle	
			vertices[0].tex = XMFLOAT2(0.0, 0.0);  // Top left.	
			vertices[1].tex = XMFLOAT2(1.0, 1.0);  // Bottom right.
			vertices[2].tex = XMFLOAT2(0.0, 1.0);  // Bottom left.
			//Second triangle
			vertices[3].tex = XMFLOAT2(0.0, 0.0);  // Top left.
			vertices[4].tex = XMFLOAT2(1.0, 0.0);  // Top right.
			vertices[5].tex = XMFLOAT2(1.0, 1.0);  // Bottom right.
		}
		return CreateBuffers(device, &vertices[0], &indices[0], verticesCountEmpty, verticesCountEmpty);
	}
	else
	{
		std::array<VertexType, verticesCountFilled> vertices;
		std::array<unsigned long, verticesCountFilled> indices;
		for (int i = 0; i < verticesCountFilled; i++)
			indices[i] = i;

		////// LEFT //////
		//First triangle
		vertices[0].position = XMFLOAT3(left - widthLeft, top + widthTop, 0.0f);  // Top left.	
		vertices[1].position = XMFLOAT3(left, bottom - widthTop, 0.0f);  // Bottom right.
		vertices[2].position = XMFLOAT3(left - widthLeft, bottom - widthTop, 0.0f);  // Bottom left.
		//Second triangle
		vertices[3].position = XMFLOAT3(left - widthLeft, top + widthTop, 0.0f);  // Top left.
		vertices[4].position = XMFLOAT3(left, top + widthTop, 0.0f);  // Top right.
		vertices[5].position = XMFLOAT3(left, bottom - widthTop, 0.0f);  // Bottom right.
		////// TOP //////
		//First triangle
		vertices[6].position = XMFLOAT3(left - widthLeft, top + widthTop, 0.0f);  // Top left.	
		vertices[7].position = XMFLOAT3(right + widthLeft, top, 0.0f);  // Bottom right.
		vertices[8].position = XMFLOAT3(left - widthLeft, top, 0.0f);  // Bottom left.
		//Second triangle
		vertices[9].position = XMFLOAT3(left - widthLeft, top + widthTop, 0.0f);  // Top left.
		vertices[10].position = XMFLOAT3(right + widthLeft, top + widthTop, 0.0f);  // Top right.
		vertices[11].position = XMFLOAT3(right + widthLeft, top, 0.0f);  // Bottom right.
		////// RIGHT //////
		//First triangle
		vertices[12].position = XMFLOAT3(right, top + widthTop, 0.0f);  // Top left.	
		vertices[13].position = XMFLOAT3(right + widthLeft, bottom - widthTop, 0.0f);  // Bottom right.
		vertices[14].position = XMFLOAT3(right, bottom - widthTop, 0.0f);  // Bottom left.
		//Second triangle
		vertices[15].position = XMFLOAT3(right, top + widthTop, 0.0f);  // Top left.
		vertices[16].position = XMFLOAT3(right + widthLeft, top + widthTop, 0.0f);  // Top right.
		vertices[17].position = XMFLOAT3(right + widthLeft, bottom - widthTop, 0.0f);  // Bottom right.
		////// BOTTOM //////
		//First triangle
		vertices[18].position = XMFLOAT3(left - widthLeft, bottom, 0.0f);  // Top left.	
		vertices[19].position = XMFLOAT3(right + widthLeft, bottom - widthTop, 0.0f);  // Bottom right.
		vertices[20].position = XMFLOAT3(left - widthLeft, bottom - widthTop, 0.0f);  // Bottom left.
		//Second triangle
		vertices[21].position = XMFLOAT3(left - widthLeft, bottom, 0.0f);  // Top left.
		vertices[22].position = XMFLOAT3(right + widthLeft, bottom, 0.0f);  // Top right.
		vertices[23].position = XMFLOAT3(right + widthLeft, bottom - widthTop, 0.0f);  // Bottom right.

		return CreateBuffers(device, &vertices[0], &indices[0], verticesCountFilled, verticesCountFilled);
	}
}

bool ModelClass::is_number(const std::string & s)
{
	return !s.empty() && std::find_if(s.begin(),
		s.end(), [](char c) { return !std::isdigit(c); }) == s.end();
}

void ModelClass::CalculateAxisBound(float & min, float & max, std::vector<float>& elements) const
{
	if (elements.size() > 0)
	{
		std::vector<float>::iterator maxIt = std::max_element(elements.begin(), elements.end());
		std::vector<float>::iterator minIt = std::min_element(elements.begin(), elements.end());		
		max = elements.at(std::distance(elements.begin(), maxIt));
		min = elements.at(std::distance(elements.begin(), minIt));
	}
}

std::string ModelClass::LoadModelCalculatePath(bool fbx)
{
	PWSTR pszFilePath;
	wchar_t* wFilePath = 0;
	std::wstringstream ss;
	IFileOpenDialog *pFileOpen;
	COMDLG_FILTERSPEC objSpec = { L"OBJ (Wavefront .obj)", L"*.obj" };
	if (fbx)
		objSpec = { L"FBX (.fbx)", L"*.fbx" };
	const COMDLG_FILTERSPEC rgSpec[] = { objSpec };

	// Create the FileOpenDialog object.
	HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL,
		IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));

	if (SUCCEEDED(hr))
		hr = pFileOpen->SetFileTypes(1, rgSpec);

	if (SUCCEEDED(hr))
	{
		// Show the Open dialog box.
		hr = pFileOpen->Show(NULL);

		// Get the file name from the dialog box.
		if (SUCCEEDED(hr))
		{
			IShellItem *pItem;
			hr = pFileOpen->GetResult(&pItem);
			if (SUCCEEDED(hr))
			{
				hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

				// Display the file name to the user.
				if (SUCCEEDED(hr))
				{
					ss << pszFilePath;
					CoTaskMemFree(pszFilePath);
				}
				pItem->Release();
			}
		}
		pFileOpen->Release();
	}

	if (ss)
	{
		const std::wstring ws = ss.str();
		const std::string str(ws.begin(), ws.end());

		const std::size_t found = str.find_last_of("/\\");
		return str.substr(found + 1);;
	}

	return "";
}

void ModelClass::SaveVisibleName()
{
	//Really bad but needed due to using string.data() in ImGui in graphicsclass.cpp
	m_savedName = "";
	m_name.resize(30);
	for (int i = 0; i < strlen(m_name.data()); ++i)
	{
		m_savedName += m_name.data()[i];
	}
}

void ModelClass::SetSavedName(std::string name)
{
	m_savedName = name;
	m_name = name;
}

void ModelClass::SetMaterial(MaterialPrefab * const material)
{
	m_material = material;
	m_materialName = m_material->GetName();
}

XMFLOAT3 Bounds::BoundingBoxSize(XMMATRIX & worldMatrix, XMMATRIX & viewMatrix, XMMATRIX & projectionMatrix)
{
	//TODO Test if there is need to create local matrices for every scope

	XMFLOAT3 min{};
	XMFLOAT3 max{};
	//X axis
	{
		XMMATRIX w = worldMatrix;
		XMMATRIX v = viewMatrix;
		XMMATRIX p = projectionMatrix;
		XMVECTOR clipSpacePos = XMVector4Transform({ minX, GetCenterY(), GetCenterZ(), 1.0f }, w);
		clipSpacePos = XMVector4Transform(clipSpacePos, v);
		clipSpacePos = XMVector4Transform(clipSpacePos, p);
		clipSpacePos.m128_f32[0] /= clipSpacePos.m128_f32[3];

		min.x = (clipSpacePos.m128_f32[0] + 1.0f) * 0.5f;
	}
	{
		XMMATRIX w = worldMatrix;
		XMMATRIX v = viewMatrix;
		XMMATRIX p = projectionMatrix;
		XMVECTOR clipSpacePos = XMVector4Transform({ maxX, GetCenterY(), GetCenterZ(), 1.0f }, w);
		clipSpacePos = XMVector4Transform(clipSpacePos, v);
		clipSpacePos = XMVector4Transform(clipSpacePos, p);
		clipSpacePos.m128_f32[0] /= clipSpacePos.m128_f32[3];

		max.x = (clipSpacePos.m128_f32[0] + 1.0f) * 0.5f;
	}
	//Y axis
	{
		XMMATRIX w = worldMatrix;
		XMMATRIX v = viewMatrix;
		XMMATRIX p = projectionMatrix;
		XMVECTOR clipSpacePos = XMVector4Transform({ GetCenterX(), minY, GetCenterZ(), 1.0f }, w);
		clipSpacePos = XMVector4Transform(clipSpacePos, v);
		clipSpacePos = XMVector4Transform(clipSpacePos, p);
		clipSpacePos.m128_f32[1] /= clipSpacePos.m128_f32[3];

		min.y = (clipSpacePos.m128_f32[1] + 1.0f) * 0.5f;
	}
	{
		XMMATRIX w = worldMatrix;
		XMMATRIX v = viewMatrix;
		XMMATRIX p = projectionMatrix;
		XMVECTOR clipSpacePos = XMVector4Transform({ GetCenterX(), maxY, GetCenterZ(), 1.0f }, w);
		clipSpacePos = XMVector4Transform(clipSpacePos, v);
		clipSpacePos = XMVector4Transform(clipSpacePos, p);
		clipSpacePos.m128_f32[1] /= clipSpacePos.m128_f32[3];

		max.y = (clipSpacePos.m128_f32[1] + 1.0f) * 0.5f;
	}
	//Z Axis
	{
		XMMATRIX w = worldMatrix;
		XMMATRIX v = viewMatrix;
		XMMATRIX p = projectionMatrix;
		XMVECTOR clipSpacePos = XMVector4Transform({ GetCenterX(), GetCenterY(), minZ, 1.0f }, w);
		clipSpacePos = XMVector4Transform(clipSpacePos, v);
		clipSpacePos = XMVector4Transform(clipSpacePos, p);
		clipSpacePos.m128_f32[2] /= clipSpacePos.m128_f32[3];

		//min.z = (clipSpacePos.m128_f32[2] + 1.0f) * 0.5f;
		min.z = clipSpacePos.m128_f32[2];
	}
	{
		XMMATRIX w = worldMatrix;
		XMMATRIX v = viewMatrix;
		XMMATRIX p = projectionMatrix;
		XMVECTOR clipSpacePos = XMVector4Transform({ GetCenterX(), GetCenterY(), maxZ, 1.0f }, w);
		clipSpacePos = XMVector4Transform(clipSpacePos, v);
		clipSpacePos = XMVector4Transform(clipSpacePos, p);
		clipSpacePos.m128_f32[2] /= clipSpacePos.m128_f32[3];

		//max.z = (clipSpacePos.m128_f32[2] + 1.0f) * 0.5f;
		max.z = clipSpacePos.m128_f32[2];
	}
	return{ max.x - min.x, max.y - min.y, max.z - min.z };
}

XMFLOAT3 Bounds::GetMinBounds(ModelClass * const model) const
{
	//return{ model->GetBounds().minX + model->GetPositionXYZ().x, model->GetBounds().minY + model->GetPositionXYZ().y, model->GetBounds().minZ + model->GetPositionXYZ().z };;
	return{ model->GetBounds().minX, model->GetBounds().minY, model->GetBounds().minZ };
}

XMFLOAT3 Bounds::GetMaxBounds(ModelClass * const model) const
{
	//return{ model->GetBounds().maxX + model->GetPositionXYZ().x, model->GetBounds().maxY + model->GetPositionXYZ().y, model->GetBounds().maxZ + model->GetPositionXYZ().z };;
	return{ model->GetBounds().maxX, model->GetBounds().maxY, model->GetBounds().maxZ };
}
