#include "BaseShaderClass.h"

bool BaseShaderClass::Initialize(ID3D11Device* device, HWND hwnd, WCHAR* vsFilename, WCHAR* psFilename, vertexInputType vertexInput)
{
	//Initialize through this method because constructor may throw
	return InitializeShader(device, hwnd, vsFilename, psFilename, vertexInput);
}


void BaseShaderClass::Shutdown()
{
	// Shutdown the vertex and pixel shaders as well as the related objects.
	ShutdownShader();
}


bool BaseShaderClass::Render(ID3D11DeviceContext* deviceContext, int indexCount, XMMATRIX& worldMatrix, XMMATRIX& viewMatrix, XMMATRIX& projectionMatrix)
{
	// Set the shader parameters that it will use for rendering.
	if (!SetShaderParameters(deviceContext, worldMatrix, viewMatrix, projectionMatrix))
		return false;

	// Now render the prepared buffers with the shader.
	RenderShader(deviceContext, indexCount);

	return true;
}

bool BaseShaderClass::InitializeShader(ID3D11Device* device, HWND hwnd, WCHAR* vsFilename, WCHAR* psFilename, vertexInputType vertexInput)
{
	HRESULT result;
	ID3D10Blob* errorMessage{ nullptr };
	ID3D10Blob* vertexShaderBuffer{ nullptr };
	ID3D10Blob* pixelShaderBuffer{ nullptr };

	//Compile vertex shader
	result = D3DCompileFromFile(vsFilename, NULL, NULL, "ColorVertexShader", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &vertexShaderBuffer, &errorMessage);
	if (FAILED(result))
	{
		MessageBox(hwnd, (char*)(errorMessage->GetBufferPointer()), "VS Error", MB_OK);
		return false;
	}

	//Compile pixel shader
	result = D3DCompileFromFile(psFilename, NULL, NULL, "ColorPixelShader", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &pixelShaderBuffer, &errorMessage);
	if (FAILED(result))
	{
		MessageBox(hwnd, (char*)(errorMessage->GetBufferPointer()), "PS Error", MB_OK);
		return false;
	}

	//Fill VS buffer
	result = device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), NULL, &m_vertexShader);
	if (FAILED(result))
		return false;

	//Fill PS buffer
	result = device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, &m_pixelShader);
	if (FAILED(result))
		return false;

	if (!CreateInputLayout(device, vertexShaderBuffer, vertexInput))
		return false;

	// Release the vertex shader buffer and pixel shader buffer since they are no longer needed.
	vertexShaderBuffer->Release();
	pixelShaderBuffer->Release();

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

	const int size = vertexInput.name.size();
	D3D11_INPUT_ELEMENT_DESC *polygonLayout = new D3D11_INPUT_ELEMENT_DESC[size];
	auto map = new vertexInputMap();

	//Use struct to fill polygon layout
	for (int i = 0; i < size; i++)
	{
		auto name = vertexInput.name.at(i);
		auto format = vertexInput.format.at(i);
		auto val = map->find(name); //Using map might be slow; Used sparringly - should be enough

		polygonLayout[i].SemanticName = name;
		polygonLayout[i].SemanticIndex = (val != map->end()) ? val->second : 0;
		polygonLayout[i].Format = format;
		polygonLayout[i].InputSlot = 0;
		polygonLayout[i].AlignedByteOffset = i == 0 ? 0 : D3D11_APPEND_ALIGNED_ELEMENT;
		polygonLayout[i].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		polygonLayout[i].InstanceDataStepRate = 0;

		if (val != map->end())
			val->second += 1;
		else
			map->insert(std::pair<LPCSTR, int>(name, 1));
	}

	// Create the vertex input layout.
	const HRESULT result = device->CreateInputLayout(polygonLayout, static_cast<UINT>(size), vertexShaderBuffer->GetBufferPointer(),
		vertexShaderBuffer->GetBufferSize(), &m_layout);

	return !FAILED(result);
}

bool BaseShaderClass::CreateBuffers(ID3D11Device * device)
{
	D3D11_BUFFER_DESC matrixBufferDesc;

	// Setup the description of the dynamic matrix constant buffer that is in the vertex shader.
	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;

	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	const HRESULT result = device->CreateBuffer(&matrixBufferDesc, NULL, &m_matrixBuffer);
	if (FAILED(result))
		return false;

	return CreateBufferAdditionals(device);
}

bool BaseShaderClass::CreateBufferAdditionals(ID3D11Device *& device)
{
	return true; //Overridden by child classes
}

bool BaseShaderClass::CreateSamplerState(ID3D11Device * device)
{
	//Base sampler state that is used as general-purpose is creating here
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
	const HRESULT result = device->CreateSamplerState(&samplerDesc, &m_samplerState);
	if (FAILED(result))
		return false;

	//This sampler holds first slot (usually) in samplers list
	AddSampler(false, 0, 1, m_samplerState);

	return true;
}


void BaseShaderClass::ShutdownShader()
{
	//TODO Old - provide better cleaning or remove completely
	for (int i = 0; i < m_buffers.size(); i++)
	{
		m_buffers.at(i)->Release();
		m_buffers.at(i) = 0;
	}
	m_buffers.clear();

	if (m_matrixBuffer)
	{
		m_matrixBuffer->Release();
		m_matrixBuffer = 0;
	}
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
}


void BaseShaderClass::OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, CHAR* shaderFilename) const
{
	if (std::ofstream fout{ "shader-error.txt" })
	{
		// Get the length of the message.
		unsigned long bufferSize = errorMessage->GetBufferSize();

		// Get a pointer to the error message text buffer.
		const char* const compileErrors = (char*)(errorMessage->GetBufferPointer());

		for (auto i = 0; i < bufferSize; ++i)
		{
			fout << compileErrors[i];
		}
	}
	errorMessage->Release();
	errorMessage = 0;
}

bool BaseShaderClass::LoadTexture(ID3D11Device * device, const wchar_t* filename, ID3D11Resource *&texture, ID3D11ShaderResourceView *&textureView, bool isDDS)
{
	//Make sure to avoid data leaking by releasing resource before creating new textures
	if (texture != nullptr)
		texture->Release();

	if (textureView != nullptr)
		textureView->Release();

	HRESULT result;
	if (isDDS)
		result = CreateDDSTextureFromFile(device, filename, &texture, &textureView);
	else
		result = CreateWICTextureFromFile(device, filename, &texture, &textureView);

	if (FAILED(result))
	{
		if (texture != nullptr)
			texture->Release();
		if (textureView != nullptr)
			textureView->Release();

		texture = nullptr;
		textureView = nullptr;

		return false;
	}

	return true;
}

bool BaseShaderClass::LoadTexture(ID3D11Device* device, ID3D11Resource *& inTexture, ID3D11Resource *& outTexture, ID3D11ShaderResourceView *& outTextureView)
{
	//Make sure to avoid data leaking by releasing resource before creating new textures
	if (outTexture != nullptr)
		outTexture->Release();

	if (outTextureView != nullptr)
		outTextureView->Release();

	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
	ZeroMemory(&shaderResourceViewDesc, sizeof(shaderResourceViewDesc));
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.TextureCube.MostDetailedMip = 0;
	shaderResourceViewDesc.TextureCube.MipLevels = 1;

	return device->CreateShaderResourceView(inTexture, &shaderResourceViewDesc, &outTextureView);
}

void BaseShaderClass::AddSampler(bool isVectorResource, UINT startSlot, UINT numSapmlers, ID3D11SamplerState *& samplerStates)
{
	m_samplers.push_back(new BaseShaderClass::SamplerType{ isVectorResource, startSlot, numSapmlers, samplerStates });
}

bool BaseShaderClass::SetShaderParameters(ID3D11DeviceContext* deviceContext, XMMATRIX& worldMatrix, XMMATRIX& viewMatrix, XMMATRIX& projectionMatrix)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;

	worldMatrix = XMMatrixTranspose(worldMatrix);
	viewMatrix = XMMatrixTranspose(viewMatrix);
	projectionMatrix = XMMatrixTranspose(projectionMatrix);

	// Lock the constant buffer so it can be written to.
	const HRESULT result = deviceContext->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
		return false;

	// Get a pointer to the data in the constant buffer.
	MatrixBufferType* dataPtr = static_cast<MatrixBufferType*>(mappedResource.pData);

	dataPtr->world = worldMatrix;
	dataPtr->view = viewMatrix;
	dataPtr->projection = projectionMatrix;

	deviceContext->Unmap(m_matrixBuffer, 0);
	deviceContext->VSSetConstantBuffers(0, 1, &m_matrixBuffer); //Base class - binds only 0 buffer for matrices MVP

	return true;
}


void BaseShaderClass::RenderShader(ID3D11DeviceContext* deviceContext, int indexCount)
{
	// Set the vertex input layout.
	deviceContext->IASetInputLayout(m_layout);

	// Set the vertex and pixel shaders that will be used to render this triangle.
	deviceContext->VSSetShader(m_vertexShader, NULL, 0);
	deviceContext->PSSetShader(m_pixelShader, NULL, 0);

	for (SamplerType* const& sampler : m_samplers)
	{
		if (sampler->isVectorSampler)
			deviceContext->VSSetSamplers(sampler->startSlot, sampler->numSamplers, &sampler->samplerStates);
		else
			deviceContext->PSSetSamplers(sampler->startSlot, sampler->numSamplers, &sampler->samplerStates);
	}
	deviceContext->DrawIndexed(indexCount, 0, 0);
}