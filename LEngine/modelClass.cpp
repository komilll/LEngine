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
	if(!InitializeBuffers(device, modelFilename))
		return false;

	return true;
}

bool ModelClass::Initialize(ID3D11Device * device, ShapeSize shape, float left, float right, float top, float bottom)
{
	if (shape == ModelClass::ShapeSize::RECTANGLE)
		return CreateRectangle(device, left, right, top, bottom);

	return false;
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


bool ModelClass::InitializeBuffers(ID3D11Device* device, const char* modelFilename)
{
	std::vector<VertexType> verticesVector(0);
	VertexType* vertices;
	unsigned long* indices;
	HRESULT result;

	/////////// GET VERTEX COUNT ////////
	int m_vertexCount = 0;

	char curChar;
	std::string curLine;
	std::ifstream input;

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
		input.open(modelFilename);

		if (input.fail())
			return false;

		std::getline(input, curLine);

		std::string x, y, z;
		std::string type = "";
		int curIndex = 0;
		while (!input.eof())
		{
			if (curLine == "")
			{
				std::getline(input, curLine);
				continue;
			}

			std::istringstream iss(curLine);
			iss >> type;
			if (type == "v")
			{
				iss >> x >> y >> z;
				vertexPosition.push_back(DirectX::XMFLOAT3(std::stof(x), std::stof(y), std::stof(z)));
			}
			else if (type == "vt")
			{
				iss >> x >> y;
				texPosition.push_back(DirectX::XMFLOAT2(std::stof(x), std::stof(y)));
			}
			else if (type == "vn")
			{
				iss >> x >> y >> z;
				normalPosition.push_back(DirectX::XMFLOAT3(std::stof(x), std::stof(y), std::stof(z)));
			}

			std::getline(input, curLine);

		}

		vertices = new VertexType[m_vertexCount = vertexPosition.size()];

		for (int i = 0; i < m_vertexCount; i++)
		{
			vertices[i].position = vertexPosition.at(i);
		}
		////////// FEEDING INDICES WITH DATA /////////
		input.close();
		input.open(modelFilename);
		std::getline(input, curLine);
		type = "";

		int vIndex = 0;
		int vtIndex = 0;
		int vnIndex = 0;
		while (!input.eof())
		{
			std::istringstream iss(curLine);
			iss >> type;

			if (type == "f")
			{
				if (texPosition.size() == 0)
					vtIndex = -1;
				while (iss >> x)
				{
					SetIndices(x, vIndex, vtIndex, vnIndex);
					indicesVec.push_back(vIndex);
					//indices[curIndex] = vIndex;
					if (texPosition.size() > 0)
					{
						vertices[vIndex].tex = texPosition.at(vtIndex);
						texIndices.push_back(vtIndex);
					}
					if (normalPosition.size() > 0)
					{
						vertices[vIndex].normal = normalPosition.at(vnIndex);
						normalIndices.push_back(vnIndex);
					}
					//curIndex++;
				}
			}
			std::getline(input, curLine);
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

	input.close();
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
		if (curOut != "")
			index += stoi(curOut);
		
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

	input.clear();
	input.close();
	return true;
}

/////// SHAPES DRAWING ///////
bool ModelClass::CreateRectangle(ID3D11Device* device, float left, float right, float top, float bottom)
{
	VertexType* vertices;
	unsigned long* indices;

	vertices = new VertexType[6];
	//First triangle
	vertices[0].position = XMFLOAT3(left, top, 0.0f);  // Top left.	
	vertices[1].position = XMFLOAT3(right, bottom, 0.0f);  // Bottom right.
	vertices[2].position = XMFLOAT3(left, bottom, 0.0f);  // Bottom left.
	//Second triangle
	vertices[3].position = XMFLOAT3(left, top, 0.0f);  // Top left.
	vertices[4].position = XMFLOAT3(right, top, 0.0f);  // Top right.
	vertices[5].position = XMFLOAT3(right, bottom, 0.0f);  // Bottom right.

	indices = new unsigned long[6];
	for (int i = 0; i < 6; i++)
		indices[i] = i;

	if (CreateBuffers(device, vertices, indices, 6, 6) == false)
		return false;
}
