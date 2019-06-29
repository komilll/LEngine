////////////////////////////////////////////////////////////////////////////////
// Filename: modelclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "modelclass.h"


ModelClass::ModelClass()
{
	m_vertexBuffer = 0;
	m_indexBuffer = 0;
}


ModelClass::ModelClass(const ModelClass& other)
{
}


ModelClass::~ModelClass()
{
}


bool ModelClass::Initialize(ID3D11Device* device, const char* modelFilename)
{
	m_device = device;
	m_modelFilename = modelFilename;
	if(!InitializeBuffers(device, modelFilename))
		return false;

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

bool ModelClass::InitializeSquare(ID3D11Device * device, float centerX, float centerY, float size, bool isEmpty, bool withTex)
{
	m_device = device;
	m_primitiveModel.SetSquare(centerX, centerY, size, isEmpty, withTex);
	return CreateSquare(device, centerX, centerY, size, isEmpty, withTex);
}

void ModelClass::Shutdown()
{
	// Shutdown the vertex and index buffers.
	ShutdownBuffers();

	return;
}


void ModelClass::Render(ID3D11DeviceContext* deviceContext)
{
	// Put the vertex and index buffers on the graphics pipeline to prepare them for drawing.
	RenderBuffers(deviceContext);

	return;
}

int ModelClass::GetIndexCount()
{
	return m_indexCount;
}

void ModelClass::SetPosition(float x, float y, float z)
{
	SetPosition(XMFLOAT3{ x, y, z });
}

void ModelClass::SetPosition(XMFLOAT3 position)
{
	m_position[0] = position.x;
	m_position[1] = position.y;
	m_position[2] = position.z;
	m_position[3] = 1.0f;
}

void ModelClass::SetScale(float x, float y, float z)
{
	m_scale[0] = x;
	m_scale[1] = y;
	m_scale[2] = z;
}

void ModelClass::SetScale(XMFLOAT3 scale)
{
	SetScale(scale.x, scale.y, scale.z);
}

void ModelClass::SetRotation(float x, float y, float z)
{
	m_rotation[0] = x;
	m_rotation[1] = y;
	m_rotation[2] = z;
}

XMFLOAT4 ModelClass::GetPosition() const
{
	return{ m_position[0], m_position[1], m_position[2], m_position[3] };
}

XMFLOAT3 ModelClass::GetPositionXYZ() const
{
	return{ m_position[0], m_position[1], m_position[2] };
}

XMFLOAT3 ModelClass::GetScale() const
{
	return{ m_scale[0], m_scale[1], m_scale[2] };
}

XMFLOAT3 ModelClass::GetRotation() const
{
	return{ m_rotation[0], m_rotation[1], m_rotation[2] };
}

float* ModelClass::GetPositionRef()
{
	return &m_position[0];
}

float* ModelClass::GetScaleRef()
{
	return &m_scale[0];
}

float* ModelClass::GetRotationRef()
{
	return &m_rotation[0];
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
		{"position", json11::Json::array{m_position[0], m_position[1], m_position[2]}},
		{"scale", json11::Json::array{m_scale[0], m_scale[1], m_scale[2]}},
		{"rotation", json11::Json::array{m_rotation[0], m_rotation[1], m_rotation[2]}}
	};
	return obj.dump();
}

void ModelClass::LoadData()
{

}

ModelClass* ModelClass::LoadModel(ID3D11Device * d3d)
{
	ModelClass* model = new ModelClass();
	model->Initialize(d3d, model->LoadModelCalculatePath().c_str());
	model->m_name = "Test_Name";
	return model;
}

void ModelClass::LoadModel()
{
	if (m_device)
	{
		Initialize(m_device, LoadModelCalculatePath().c_str());
	}
}

bool ModelClass::InitializeBuffers(ID3D11Device* device, const char* modelFilename)
{
	std::vector<VertexType> verticesVector(0);
	VertexType* vertices{ nullptr };
	unsigned long* indices;
	HRESULT result;

	/////////// GET VERTEX COUNT ////////
	int m_vertexCount = 0;

	char curChar;

	///////// CORRECT VERTICES LOADING ////////
	std::vector<DirectX::XMFLOAT3> vertexPosition;
	std::vector<DirectX::XMFLOAT2> texPosition;
	std::vector<DirectX::XMFLOAT3> normalPosition;
	std::vector<int> texIndices;
	std::vector<int> normalIndices;
	std::vector<unsigned long> indicesVec;

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
	for (int i = 0; i < m_indexCount; i++)
	{
		indices[i] = indicesVec.at(i);
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
	unsigned int stride = sizeof(VertexType);
	unsigned int offset = 0;

	deviceContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);
	deviceContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	return;
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

void ModelClass::CalculateDataForNormalMapping(VertexType* &vertices)
{
	int faceCount = m_vertexCount / 3;
	int index = 0;

	VertexType vertex1, vertex2, vertex3;
	XMFLOAT3 tangent, binormal, normal;

	for (int i = 0; i < faceCount; i++)
	{
		vertex1 = vertices[index++];
		vertex2 = vertices[index++];
		vertex3 = vertices[index++];

		CalculateTangentBinormal(vertex1, vertex2, vertex3, tangent, binormal);
		CalculateNormal(tangent, binormal, normal);

		vertices[index - 1].normal = normal;
		vertices[index - 1].tangent = tangent;
		vertices[index - 1].binormal = binormal;

		vertices[index - 2].normal = normal;
		vertices[index - 2].tangent = tangent;
		vertices[index - 2].binormal = binormal;

		vertices[index - 3].normal = normal;
		vertices[index - 3].tangent = tangent;
		vertices[index - 3].binormal = binormal;
	}
}

void ModelClass::CalculateTangentBinormal(VertexType vertex1, VertexType vertex2, VertexType vertex3, XMFLOAT3 & tangent, XMFLOAT3 & binormal)
{
	float vector1[3], vector2[3];
	float tuVector[2], tvVector[2];
	float den;
	float length;

	// Calculate the two vectors for this face.
	vector1[0] = vertex2.position.x - vertex1.position.x;
	vector1[1] = vertex2.position.y - vertex1.position.y;
	vector1[2] = vertex2.position.z - vertex1.position.z;

	vector2[0] = vertex3.position.x - vertex1.position.x;
	vector2[1] = vertex3.position.y - vertex1.position.y;
	vector2[2] = vertex3.position.z - vertex1.position.z;

	// Calculate the tu and tv texture space vectors.
	tuVector[0] = vertex2.tex.x - vertex1.tex.x;
	tvVector[0] = vertex2.tex.y - vertex1.tex.y;

	tuVector[1] = vertex3.tex.x - vertex1.tex.x;
	tvVector[1] = vertex3.tex.y - vertex1.tex.y;

	// Calculate the denominator of the tangent/binormal equation.
	den = 1.0f / (tuVector[0] * tvVector[1] - tuVector[1] * tvVector[0]);

	// Calculate the cross products and multiply by the coefficient to get the tangent and binormal.
	tangent.x = (tvVector[1] * vector1[0] - tvVector[0] * vector2[0]) * den;
	tangent.y = (tvVector[1] * vector1[1] - tvVector[0] * vector2[1]) * den;
	tangent.z = (tvVector[1] * vector1[2] - tvVector[0] * vector2[2]) * den;

	binormal.x = (tuVector[0] * vector2[0] - tuVector[1] * vector1[0]) * den;
	binormal.y = (tuVector[0] * vector2[1] - tuVector[1] * vector1[1]) * den;
	binormal.z = (tuVector[0] * vector2[2] - tuVector[1] * vector1[2]) * den;

	// Calculate the length of this normal.
	length = sqrt((tangent.x * tangent.x) + (tangent.y * tangent.y) + (tangent.z * tangent.z));

	// Normalize the normal and then store it
	tangent.x = tangent.x / length;
	tangent.y = tangent.y / length;
	tangent.z = tangent.z / length;

	// Calculate the length of this normal.
	length = sqrt((binormal.x * binormal.x) + (binormal.y * binormal.y) + (binormal.z * binormal.z));

	// Normalize the normal and then store it
	binormal.x = binormal.x / length;
	binormal.y = binormal.y / length;
	binormal.z = binormal.z / length;
}

void ModelClass::CalculateNormal(XMFLOAT3 & tangent, XMFLOAT3 & binormal, XMFLOAT3 & normal)
{
	float length;

	// Calculate the cross product of the tangent and binormal which will give the normal vector.
	normal.x = (tangent.y * binormal.z) - (tangent.z * binormal.y);
	normal.y = (tangent.z * binormal.x) - (tangent.x * binormal.z);
	normal.z = (tangent.x * binormal.y) - (tangent.y * binormal.x);

	// Calculate the length of the normal.
	length = sqrt((normal.x * normal.x) + (normal.y * normal.y) + (normal.z * normal.z));

	// Normalize the normal.
	normal.x = normal.x / length;
	normal.y = normal.y / length;
	normal.z = normal.z / length;
}

bool ModelClass::CreateBuffers(ID3D11Device* device, VertexType * &vertices, unsigned long * &indices, int vertexCount, int indexCount)
{
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData, indexData;
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
	HRESULT result = device->CreateBuffer(&vertexBufferDesc, &vertexData, &m_vertexBuffer);
	if (FAILED(result))
	{
		return false;
	}

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
	result = device->CreateBuffer(&indexBufferDesc, &indexData, &m_indexBuffer);
	if (FAILED(result))
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

	std::ofstream output("Models/" + newFilename, std::ios::binary);

	int size = vertexType.size();
	output.write(reinterpret_cast<const char*>(&size), sizeof(size));
	size = vertexIndices.size();
	output.write(reinterpret_cast<const char*>(&size), sizeof(size));

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

	output.clear();
	output.close();
}

bool ModelClass::ReadBinary(const char* modelFilename, std::vector<VertexType> &vertexType, std::vector<unsigned long> &vertexIndices)
{
	std::string newFilename = modelFilename;
	newFilename = newFilename.substr(0, newFilename.find("."));
	newFilename += ".modelclass";

	std::ifstream input("Models/" + newFilename, std::ios::binary);

	if (input.fail())
		return false;

	VertexType valVertex;
	long valIndices = 0;
	int sizeVertices = 0;
	int sizeIndices = 0;
	input.read(reinterpret_cast<char*>(&sizeVertices), sizeof(sizeVertices));
	input.read(reinterpret_cast<char*>(&sizeIndices), sizeof(sizeIndices));

	for (int i = 0; i < sizeVertices; i++)
	{
		input.read(reinterpret_cast<char*>(&valVertex), sizeof(valVertex));
		vertexType.push_back(valVertex);
	}
	for (int i = 0; i < sizeIndices; i++)
	{
		input.read(reinterpret_cast<char*>(&valIndices), sizeof(valIndices));
		vertexIndices.push_back(valIndices);
	}

	//Load bounds AABB
	input.read(reinterpret_cast<char*>(&bounds.maxX), sizeof(bounds.maxX));
	input.read(reinterpret_cast<char*>(&bounds.maxY), sizeof(bounds.maxY));
	input.read(reinterpret_cast<char*>(&bounds.maxZ), sizeof(bounds.maxZ));
	input.read(reinterpret_cast<char*>(&bounds.minX), sizeof(bounds.minX));
	input.read(reinterpret_cast<char*>(&bounds.minZ), sizeof(bounds.minY));
	input.read(reinterpret_cast<char*>(&bounds.minY), sizeof(bounds.minZ));

	input.clear();
	input.close();
	return true;
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
	VertexType* vertices;
	unsigned long* indices;

	constexpr int verticesCount = 3;

	vertices = new VertexType[verticesCount];
	//Only triangle
	vertices[0].position = XMFLOAT3((left + right) / 2, top, 0.0f);  // Top	
	vertices[1].position = XMFLOAT3(right, bottom, 0.0f);  // Bottom right.
	vertices[2].position = XMFLOAT3(left, bottom, 0.0f);  // Bottom left.

	indices = new unsigned long[verticesCount];
	for (int i = 0; i < verticesCount; i++)
		indices[i] = i;

	m_indexCount = verticesCount;

	if (CreateBuffers(device, vertices, indices, verticesCount, verticesCount) == false)
		return false;

	return true;
}

bool ModelClass::CreateSquare(ID3D11Device * device, float centerX, float centerY, float size, bool isEmpty, bool withTex)
{
	VertexType* vertices;
	unsigned long* indices;

	int verticesCount = isEmpty ? 24 : 6;
	vertices = new VertexType[verticesCount];

	float left = centerX - size * 0.5f;
	float right = centerX + size * 0.5f;
	float top = centerY / 2 + size * 0.5f;
	float bottom = centerY / 2 - size * 0.5f;

	top *= 16.0 / 9.0;
	bottom *= 16.0 / 9.0;

	float widthLeft = 0.003f;
	float widthTop = widthLeft * 16.0f / 9.0f;

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

	if (CreateBuffers(device, vertices, indices, verticesCount, verticesCount) == false)
		return false;

	return true;
}

bool ModelClass::is_number(const std::string & s)
{
	return !s.empty() && std::find_if(s.begin(),
		s.end(), [](char c) { return !std::isdigit(c); }) == s.end();
}

void ModelClass::LoadNewIndex(std::string line, int & vIndex, int & vtIndex, int & vnIndex)
{

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

std::string ModelClass::LoadModelCalculatePath()
{
	PWSTR pszFilePath;
	wchar_t* wFilePath = 0;
	std::wstringstream ss;
	IFileOpenDialog *pFileOpen;
	const COMDLG_FILTERSPEC objSpec = { L"OBJ (Wavefront .obj)", L"*.obj" };
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

					//LoadNewTextureFromFile(wFilePath); -- INLINED
					//if (onlyPreview == false)
					//	BaseShaderClass::LoadTexture(d3d->GetDevice(), wFilePath, *m_externalTexture, *m_externalTextureView);

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
