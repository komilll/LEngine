#include "BaseShaderClass.h"

BaseShaderClass::BaseShaderClass()
{
	m_vertexShader = 0;
	m_pixelShader = 0;
	m_layout = 0;
}


BaseShaderClass::BaseShaderClass(const BaseShaderClass& other)
{
}


BaseShaderClass::~BaseShaderClass()
{
}


bool BaseShaderClass::Initialize(ID3D11Device* device, HWND hwnd, WCHAR* vsFilename, WCHAR* psFilename, vertexInputType vertexInput)
{
	bool result;


	// Initialize the vertex and pixel shaders.
	result = InitializeShader(device, hwnd, vsFilename, psFilename, vertexInput);
	if (!result)
	{
		return false;
	}

	return true;
}


void BaseShaderClass::Shutdown()
{
	// Shutdown the vertex and pixel shaders as well as the related objects.
	ShutdownShader();

	return;
}


bool BaseShaderClass::Render(ID3D11DeviceContext* deviceContext, int indexCount, XMMATRIX& worldMatrix, XMMATRIX& viewMatrix,
	XMMATRIX& projectionMatrix)
{
	bool result;


	// Set the shader parameters that it will use for rendering.
	result = SetShaderParameters(deviceContext, worldMatrix, viewMatrix, projectionMatrix);
	if (!result)
	{
		return false;
	}

	// Now render the prepared buffers with the shader.
	RenderShader(deviceContext, indexCount);

	return true;
}


bool BaseShaderClass::InitializeShader(ID3D11Device* device, HWND hwnd, WCHAR* vsFilename, WCHAR* psFilename, vertexInputType vertexInput)
{
	HRESULT result;
	ID3D10Blob* errorMessage;
	ID3D10Blob* vertexShaderBuffer;
	ID3D10Blob* pixelShaderBuffer;
	D3D11_SAMPLER_DESC samplerDesc;

	// Initialize the pointers this function will use to null.
	errorMessage = 0;
	vertexShaderBuffer = 0;
	pixelShaderBuffer = 0;

	result = D3DCompileFromFile(vsFilename, NULL, NULL, "ColorVertexShader", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &vertexShaderBuffer, &errorMessage);

	if (FAILED(result))
		return false;

	// Compile the pixel shader code.
	result = D3DCompileFromFile(psFilename, NULL, NULL, "ColorPixelShader", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &pixelShaderBuffer, &errorMessage);
	if (FAILED(result))
		return false;

	result = device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), NULL, &m_vertexShader);
	if (FAILED(result))
		return false;

	result = device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, &m_pixelShader);
	if (FAILED(result))
		return false;

	if (!CreateInputLayout(device, vertexShaderBuffer, vertexInput))
		return false;

	// Release the vertex shader buffer and pixel shader buffer since they are no longer needed.
	vertexShaderBuffer->Release();
	vertexShaderBuffer = 0;

	pixelShaderBuffer->Release();
	pixelShaderBuffer = 0;

	if (!CreateBuffers(device))
		return false;

	if (!CreateSamplerState(device))
		return false;



	return true;
}

bool BaseShaderClass::CreateInputLayout(ID3D11Device* device, ID3D10Blob* vertexShaderBuffer, vertexInputType vertexInput)
{
	if (vertexInput.name.size() != vertexInput.format.size())
		return false;

	int size = vertexInput.name.size();
	D3D11_INPUT_ELEMENT_DESC* polygonLayout = new D3D11_INPUT_ELEMENT_DESC[size];
	auto map = new vertexInputMap();
	std::locale loc;

	for (int i = 0; i < size; i++)
	{
		auto name = vertexInput.name.at(i);
		auto format = vertexInput.format.at(i);
		auto val = map->find(name);

		polygonLayout[0].SemanticName = std::toupper(name, loc);
		polygonLayout[0].SemanticIndex = (val != map->end()) ? val->second : 0;
		polygonLayout[0].Format = format;
		polygonLayout[0].InputSlot = 0;
		polygonLayout[0].AlignedByteOffset = i == 0 ? 0 : D3D11_APPEND_ALIGNED_ELEMENT;
		polygonLayout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		polygonLayout[0].InstanceDataStepRate = 0;

		if (val != map->end())
			val->second += 1;
		else
			map->insert(std::pair<vertexInputNameType, int>(name, 1));
	}
	unsigned int numElements;

	// Create the vertex input layout description.
	//polygonLayout[0].SemanticName = "POSITION";
	//polygonLayout[0].SemanticIndex = 0;
	//polygonLayout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	//polygonLayout[0].InputSlot = 0;
	//polygonLayout[0].AlignedByteOffset = 0;
	//polygonLayout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	//polygonLayout[0].InstanceDataStepRate = 0;

	//polygonLayout[1].SemanticName = "TEXCOORD";
	//polygonLayout[1].SemanticIndex = 0;
	//polygonLayout[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	//polygonLayout[1].InputSlot = 0;
	//polygonLayout[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	//polygonLayout[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	//polygonLayout[1].InstanceDataStepRate = 0;

	//polygonLayout[2].SemanticName = "NORMAL";
	//polygonLayout[2].SemanticIndex = 0;
	//polygonLayout[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	//polygonLayout[2].InputSlot = 0;
	//polygonLayout[2].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	//polygonLayout[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	//polygonLayout[2].InstanceDataStepRate = 0;

	// Get a count of the elements in the layout.
	numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

	// Create the vertex input layout.
	HRESULT result = device->CreateInputLayout(polygonLayout, numElements, vertexShaderBuffer->GetBufferPointer(),
		vertexShaderBuffer->GetBufferSize(), &m_layout);
	if (FAILED(result))
		return false;

	return true;
}

bool BaseShaderClass::CreateBuffers(ID3D11Device * device)
{
	//// Setup the description of the dynamic matrix constant buffer that is in the vertex shader.
	//matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	//matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
	//matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	//matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	//matrixBufferDesc.MiscFlags = 0;
	//matrixBufferDesc.StructureByteStride = 0;

	//// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	//result = device->CreateBuffer(&matrixBufferDesc, NULL, &m_matrixBuffer);
	//if (FAILED(result))
	//{
	//	return false;
	//}

	//lightingBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	//lightingBufferDesc.ByteWidth = sizeof(LightingBufferType);
	//lightingBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	//lightingBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	//lightingBufferDesc.MiscFlags = 0;
	//lightingBufferDesc.StructureByteStride = 0;

	//result = device->CreateBuffer(&lightingBufferDesc, NULL, &m_lightingBuffer);
	//if (FAILED(result))
	//{
	//	return false;
	//}

	//cameraBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	//cameraBufferDesc.ByteWidth = sizeof(CameraBufferType);
	//cameraBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	//cameraBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	//cameraBufferDesc.MiscFlags = 0;
	//cameraBufferDesc.StructureByteStride = 0;

	//result = device->CreateBuffer(&cameraBufferDesc, NULL, &m_cameraBuffer);
	//if (FAILED(result))
	//{
	//	return false;
	//}

	return true;
}

bool BaseShaderClass::CreateSamplerState(ID3D11Device * device)
{
	D3D11_SAMPLER_DESC samplerDesc;

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

	// Create the texture sampler state.
	HRESULT result = device->CreateSamplerState(&samplerDesc, &m_samplerState);
	if (FAILED(result))
		return false;

	return true;
}


void BaseShaderClass::ShutdownShader()
{
	if (m_pixelShader)
	{
		m_pixelShader->Release();
		m_pixelShader = 0;
	}
	if (m_vertexShader)
	{
		m_vertexShader->Release();
		m_vertexShader = 0;
	}
	if (m_layout)
	{
		m_layout->Release();
		m_layout = 0;
	}
	return;
}


void BaseShaderClass::OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, CHAR* shaderFilename)
{
	char* compileErrors;
	unsigned long bufferSize, i;
	ofstream fout;


	// Get a pointer to the error message text buffer.
	compileErrors = (char*)(errorMessage->GetBufferPointer());

	// Get the length of the message.
	bufferSize = errorMessage->GetBufferSize();

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
	errorMessage->Release();
	errorMessage = 0;

	// Pop a message up on the screen to notify the user to check the text file for compile errors.
	//MessageBox(hwnd, L"Error compiling shader.  Check shader-error.txt for message.", shaderFilename, MB_OK);

	return;
}

bool BaseShaderClass::LoadTexture(ID3D11Device * device, const wchar_t* filename, ID3D11Resource *&texture, ID3D11ShaderResourceView *&textureView)
{
	HRESULT result = CreateDDSTextureFromFile(device, filename, &texture, &textureView);
	if (FAILED(result))
		return false;

	return true;
}

bool BaseShaderClass::SetShaderParameters(ID3D11DeviceContext* deviceContext, XMMATRIX& worldMatrix, XMMATRIX& viewMatrix,
	XMMATRIX& projectionMatrix)
{
	return true;
}


void BaseShaderClass::RenderShader(ID3D11DeviceContext* deviceContext, int indexCount)
{
	// Set the vertex input layout.
	deviceContext->IASetInputLayout(m_layout);

	// Set the vertex and pixel shaders that will be used to render this triangle.
	deviceContext->VSSetShader(m_vertexShader, NULL, 0);
	deviceContext->PSSetShader(m_pixelShader, NULL, 0);

	deviceContext->PSSetSamplers(0, 1, &m_samplerState);

	// Render the triangle.
	deviceContext->DrawIndexed(indexCount, 0, 0);

	return;
}