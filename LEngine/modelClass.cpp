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


bool ModelClass::Initialize(ID3D11Device* device)
{
	bool result;


	// Initialize the vertex and index buffers.
	result = InitializeBuffers(device);
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


bool ModelClass::InitializeBuffers(ID3D11Device* device)
{
	VertexType* vertices;
	unsigned long* indices;
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData, indexData;
	HRESULT result;

	/////////// GET VERTEX COUNT ////////
	//int m_vertexCount = 0;

	//char curChar;
	//std::string curLine;
	//std::ifstream input;
	//input.open("model.obj");

	//if (input.fail())
	//	return false;

	//std::getline(input, curLine);
	//while (curLine[0] != 'v')
	//	std::getline(input, curLine);


	//while (curLine[0] == 'v' && curLine[1] == ' ')
	//{
	//	std::getline(input, curLine);
	//	m_vertexCount++;
	//}
	//input.close();
	//int m_indicesCount = 36;
	//m_indexCount = m_indicesCount;

	//vertices = new VertexType[m_vertexCount];
	//indices = new unsigned long[m_indicesCount];
	/////////// CORRECT VERTICES LOADING ////////
	//input.open("model.obj");

	//if (input.fail())
	//	return false;

	//std::getline(input, curLine);
	//while (curLine[0] != 'v')
	//	std::getline(input, curLine);

	//float x = INFINITY;
	//float y = INFINITY;
	//float z = INFINITY;
	//std::string curNumber = "";
	//int curIndex = 0;
	//while (curLine[0] == 'v' && curLine[1] == ' ')
	//{
	//	for (int i = 2; i < curLine.length(); i++)
	//	{
	//		if (curLine[i] == ' ')
	//		{
	//			if (x == INFINITY)
	//				x = atof(curNumber.c_str());
	//			else if (y == INFINITY)
	//				y = atof(curNumber.c_str());

	//			curNumber = "";
	//		}
	//		else
	//			curNumber += curLine[i];
	//	}
	//	z = atof(curNumber.c_str());;
	//	curNumber = "";

	//	std::getline(input, curLine);
	//	//Feeding struct with vertex data
	//	vertices[curIndex].position = DirectX::XMFLOAT3(x, y, z);
	//	curIndex++;
	//}
	//////////// FEEDING INDICES WITH DATA /////////
	//std::string lookUp = "";
	//curNumber = "";
	//curIndex = 0;
	//x = INFINITY;
	//y = INFINITY;
	//z = INFINITY;
	//while (curLine[0] != 'f')
	//	std::getline(input, curLine);
	//while (curLine[0] == 'f' && curLine[1] == ' ')
	//{
	//	for (int i = 2; i < curLine.length(); i++)
	//	{
	//		if (curLine[i] == ' ')
	//		{
	//			z = atoi(curNumber.c_str());;
	//			if (x != INFINITY)
	//			{
	//				indices[curIndex] = x - 1;
	//				curIndex++;
	//				lookUp += ((int)x);
	//		/*		lookUp += ", ";*/
	//			}
	//			x = INFINITY;
	//			y = INFINITY;
	//			z = INFINITY;
	//			curNumber = "";
	//		}
	//		else if (curLine[i] == '/')
	//		{
	//			if (x == INFINITY)
	//				x = atoi(curNumber.c_str());
	//			else if (y == INFINITY)
	//			{
	//				y = 0;
	//				y = atoi(curNumber.c_str());
	//			}

	//			curNumber = "";
	//		}
	//		else
	//			curNumber += curLine[i];
	//	}
	//	z = atoi(curNumber.c_str());;
	//	if (x != INFINITY)
	//	{
	//		indices[curIndex] = x - 1;
	//		curIndex++;
	//		int toSave = x;
	//		const char* c = (char*)toSave;
	//		lookUp += ((int)x);
	//		//lookUp += "\n";
	//	}
	//	x = INFINITY;
	//	y = INFINITY;
	//	z = INFINITY;
	//	curNumber = "";

	//	if (input.eof())
	//		break;
	//	std::getline(input, curLine);
	//}
	////for (int i = 0; i < m_indexCount; i++)
	////{
	////	indices[i] = i;
	////}
	//input.close();

	int m_vertexCount = 4;
	int m_indicesCount = 6;
	m_indexCount = m_indicesCount;

	vertices = new VertexType[m_vertexCount];
	indices = new unsigned long[m_indicesCount];

	vertices[0].position = DirectX::XMFLOAT3(-1.0f, 1.0f, 5.0f);  // Top left.
	vertices[1].position = DirectX::XMFLOAT3(1.0f, 1.0f, 0.0f);  // Top right.
	vertices[2].position = DirectX::XMFLOAT3(1.0f, -1.0f, 0.0f);  // Bottom right.
	vertices[3].position = DirectX::XMFLOAT3(-1.0f, -1.0f, 0.0f);  // Bottom left.

																   // Load the index array with data.
	indices[0] = 0;  // Bottom left.
	indices[1] = 1;  // Top middle.
	indices[2] = 2;  // Bottom right.
	indices[3] = 0;  // Bottom left.
	indices[4] = 2;  // Top middle.
	indices[5] = 3;  // Bottom right.

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