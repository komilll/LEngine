#include "skybox.h"

Skybox::Skybox()
{
	m_vertexShader = 0;
	m_pixelShader = 0;
	m_layout = 0;
	m_matrixBuffer = 0;
	m_sampleState = 0;
	m_texture = 0;
}

Skybox::Skybox(const Skybox &)
{
}

Skybox::~Skybox()
{
}

bool Skybox::Initialize(ID3D11Device *device, HWND hwnd)
{
	if (!InitializeShader(device, hwnd, L"skybox.vs", L"skybox.ps"))
	{
		MessageBox(hwnd, "Failed to initialize skybox", "Graphics error", MB_OK);
		return false;
	}
	
	return true;
}

void Skybox::Shutdown()
{
	ShutdownShader();
}

bool Skybox::Render(ID3D11DeviceContext * deviceContext, int indexCount, XMMATRIX &worldMatrix_, XMMATRIX &viewMatrix_, XMMATRIX& projectionMatrix_)
{
	if (!SetShaderParameters(deviceContext, worldMatrix_, viewMatrix_, projectionMatrix_))
	{
		return false;
	}

	RenderShader(deviceContext, indexCount);

	return true;
}

bool Skybox::InitializeShader(ID3D11Device * device, HWND hwnd, LPCWSTR vsFilename, LPCWSTR psFilename)
{
	HRESULT result;

	ID3D10Blob* errorMsg;
	ID3D10Blob* vertexBuffer;
	ID3D10Blob* pixelBuffer;
	unsigned const int numElements = 1;
	D3D11_INPUT_ELEMENT_DESC polygonLayout[numElements];
	D3D11_BUFFER_DESC matrixBufferDesc;
	D3D11_SAMPLER_DESC samplerDesc;


	result = D3DCompileFromFile(vsFilename, NULL, NULL, "ColorVertexShader", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &vertexBuffer, &errorMsg);
	if (FAILED(result))
	{

		return false;
	}

	result = D3DCompileFromFile(psFilename, NULL, NULL, "ColorPixelShader", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &pixelBuffer, &errorMsg);
	if (FAILED(result))
	{

		return false;
	}

	result = device->CreateVertexShader(vertexBuffer->GetBufferPointer(), vertexBuffer->GetBufferSize(), NULL, &m_vertexShader);
	if (FAILED(result))
	{
		return false;
	}

	result = device->CreatePixelShader(pixelBuffer->GetBufferPointer(), pixelBuffer->GetBufferSize(), NULL, &m_pixelShader);
	if (FAILED(result))
	{
		return false;
	}

	polygonLayout[0].SemanticName = "POSITION";
	polygonLayout[0].SemanticIndex = 0;
	polygonLayout[0].AlignedByteOffset = 0;
	polygonLayout[0].InputSlot = 0;
	polygonLayout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	polygonLayout[0].InstanceDataStepRate = 0;

	result = device->CreateInputLayout(polygonLayout, numElements, vertexBuffer->GetBufferPointer(), vertexBuffer->GetBufferSize(), &m_layout);
	if (FAILED(result))
	{
		return false;
	}

	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;

	result = device->CreateBuffer(&matrixBufferDesc, NULL, &m_matrixBuffer);
	if (FAILED(result))
	{
		return false;
	}

	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.BorderColor[0] = 0;
	samplerDesc.BorderColor[1] = 0;
	samplerDesc.BorderColor[2] = 0;
	samplerDesc.BorderColor[3] = 0;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	result = device->CreateSamplerState(&samplerDesc, &m_sampleState);
	if (FAILED(result))
	{
		return false;
	}

	result = CreateDDSTextureFromFile(device, L"skybox.dds", &m_textureTmp, &m_texture);
	if (FAILED(result))
	{
		return false;
	}

	return true;
}

void Skybox::ShutdownShader()
{
	if (m_vertexShader)
	{
		m_vertexShader->Release();
		m_vertexShader = 0;
	}

	if (m_pixelShader)
	{
		m_pixelShader->Release();
		m_pixelShader = 0;
	}

	if (m_layout)
	{
		m_layout->Release();
		m_layout = 0;
	}

	if (m_matrixBuffer)
	{
		m_matrixBuffer->Release();
		m_matrixBuffer = 0;
	}

	if (m_sampleState)
	{
		m_sampleState->Release();
		m_sampleState = 0;
	}

	if (m_texture)
	{
		m_texture->Release();
		m_texture = 0;
	}
}

void Skybox::OutputShaderErrorMessage(ID3D10Blob * message, HWND hwnd, CHAR * shaderFilename)
{
	char* compileErrors;
	unsigned long bufferSize, i;
	ofstream fout;

	// Get a pointer to the error message text buffer.
	compileErrors = (char*)(message->GetBufferPointer());

	// Get the length of the message.
	bufferSize = message->GetBufferSize();

	// Open a file to write the error message to.
	fout.open("shader-error.txt");

	// Write out the error message.
	for (i = 0; i<bufferSize; i++)
	{
		fout << compileErrors[i];
	}

	// Close the file.
	fout.close();

	// Release the error message.
	message->Release();
	message = 0;

	// Pop a message up on the screen to notify the user to check the text file for compile errors.
	MessageBox(hwnd, "Error compiling shader.  Check shader-error.txt for message.", shaderFilename, MB_OK);

	return;
}

bool Skybox::SetShaderParameters(ID3D11DeviceContext * deviceContext, XMMATRIX& worldMatrix, XMMATRIX& viewMatrix, XMMATRIX &projectionMatrix)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBufferType* dataPtr;
	unsigned int bufferNumber;

	// Transpose the matrices to prepare them for the shader.
	worldMatrix = XMMatrixTranspose(worldMatrix);
	viewMatrix = XMMatrixTranspose(viewMatrix);
	projectionMatrix = XMMatrixTranspose(projectionMatrix);

	// Lock the constant buffer so it can be written to.
	result = deviceContext->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
	{
		return false;
	}

	// Get a pointer to the data in the constant buffer.
	dataPtr = (MatrixBufferType*)mappedResource.pData;

	// Copy the matrices into the constant buffer.
	dataPtr->world = worldMatrix;
	dataPtr->view = viewMatrix;
	dataPtr->projection = projectionMatrix;

	// Unlock the constant buffer.
	deviceContext->Unmap(m_matrixBuffer, 0);

	// Set the position of the constant buffer in the vertex shader.
	bufferNumber = 0;

	// Finanly set the constant buffer in the vertex shader with the updated values.
	deviceContext->VSSetConstantBuffers(bufferNumber, 1, &m_matrixBuffer);

	deviceContext->PSSetShaderResources(0, 1, &m_texture);
	return true;
}

void Skybox::RenderShader(ID3D11DeviceContext * deviceContext, int indexCount)
{
	deviceContext->IASetInputLayout(m_layout);

	deviceContext->VSSetShader(m_vertexShader, NULL, 0);
	deviceContext->PSSetShader(m_pixelShader, NULL, 0);

	deviceContext->PSSetSamplers(0, 1, &m_sampleState);

	deviceContext->DrawIndexed(indexCount, 0, 0);
}