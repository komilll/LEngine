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
	bool result;


	// Initialize the vertex and index buffers.
	result = InitializeBuffers(device, modelFilename);
	if(!result)
	{
		return false;
	}

	return true;
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
	VertexType* vertices;
	unsigned long* indices;
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData, indexData;
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
	std::vector<int> vertexIndices;
	std::vector<int> texIndices;
	std::vector<int> normalIndices;
	input.open(modelFilename);

	if (input.fail())
		return false;

	std::getline(input, curLine);

	std::string x, y, z;
	std::string type = "";
	int curIndex = 0;
	while (!input.eof())
	{
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

	std::vector<long> indicesVec;
	vertices = new VertexType[m_vertexCount = vertexPosition.size()];

	for (int i = 0; i < m_vertexCount; i++)
	{
		vertices[i].position = vertexPosition.at(i);
	}
	////////// FEEDING INDICES WITH DATA /////////
	input.close();
	input.open(modelFilename);
	std::getline(input, curLine);
	while (curLine[0] != 'f')
		std::getline(input, curLine);

	int vIndex, vtIndex, vnIndex;
	while (curLine[0] == 'f' && curLine[1] == ' ')
	{
		curLine = curLine.substr(2);
		std::istringstream iss(curLine);
		
		while (iss >> x)
		{
			SetIndices(x, vIndex, vtIndex, vnIndex);
			indicesVec.push_back(vIndex);
			//indices[curIndex] = vIndex;
			vertices[vIndex].tex = texPosition.at(vtIndex);
			vertices[vIndex].normal = normalPosition.at(vnIndex);
			//curIndex++;
		}
		std::getline(input, curLine);
	}

	auto m_indicesCount = indicesVec.size();
	m_indexCount = m_indicesCount;
	indices = new unsigned long[m_indicesCount];
	for (int i = 0; i < m_indexCount; i++)
	{
		indices[i] = indicesVec.at(i);
	}

	input.close();

	//Create vertex buffer description
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(VertexType) * m_vertexCount;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;
	//Fill subresource data with vertices
	vertexData.pSysMem = vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;
	//Try to create vertex buffer and store it in varaible
	result = device->CreateBuffer(&vertexBufferDesc, &vertexData, &m_vertexBuffer);
	if (FAILED(result))
	{
		return false;
	}

	//Create index buffer description
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(unsigned long) * m_indexCount;
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
		else if (curIndex == 1)
			textureIndex = index;
		else
			normalIndex = index;

		curIndex++;
	}
}