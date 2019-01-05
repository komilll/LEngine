#include "shaderController.h"

ShaderController::ShaderController()
{
}

bool ShaderController::Initialize(HWND hwnd, ID3D11Device* device)
{
	HRESULT result;
	ID3D10Blob* errorMessage;
	ID3D10Blob* vertexShaderBuffer;
	ID3D10Blob* pixelShaderBuffer;
	D3D11_INPUT_ELEMENT_DESC polygonLayout[1];
	unsigned int numElements;
	D3D11_BUFFER_DESC matrixBufferDesc, colorBufferDesc;


	// Initialize the pointers this function will use to null.
	errorMessage = 0;
	vertexShaderBuffer = 0;
	pixelShaderBuffer = 0;
	LPCWSTR vsFilename = L"basicColor.vs";
	static LPCSTR VSEntryPoint = "ColorVertexShader";
	static LPCSTR VSCompilerTarget = "vs_5_0";
	ID3DBlob* errorMessages = nullptr;

	result = D3DCompileFromFile(vsFilename, NULL, NULL, VSEntryPoint, VSCompilerTarget, D3D10_SHADER_ENABLE_STRICTNESS, 0, &vertexShaderBuffer, &errorMessages);
	// Compile the vertex shader code.
	//result = D3DX11CompileFromFile(vsFilename, NULL, NULL, "ColorVertexShader", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, NULL,
	//	&vertexShaderBuffer, &errorMessage, NULL);
	//if (FAILED(result))
	//{
	//	// If the shader failed to compile it should have writen something to the error message.
	//	if (errorMessage)
	//	{
	//		OutputShaderErrorMessage(errorMessage, hwnd, vsFilename);
	//	}
	//	// If there was  nothing in the error message then it simply could not find the shader file itself.
	//	else
	//	{
	//		MessageBox(hwnd, vsFilename, "Missing Shader File", MB_OK);
	//	}

	//	return false;
	//}

	// Compile the pixel shader code.
	LPCWSTR psFilename = L"basicColor.ps";
	static LPCSTR PSEntryPoint = "ColorPixelShader";
	static LPCSTR PSCompilerTarget = "ps_5_0";

	result = D3DCompileFromFile(psFilename, NULL, NULL, PSEntryPoint, PSCompilerTarget, D3D10_SHADER_ENABLE_STRICTNESS, 0, &pixelShaderBuffer, &errorMessages);
	//result = D3DX11CompileFromFile(psFilename, NULL, NULL, "ColorPixelShader", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, NULL,
	//	&pixelShaderBuffer, &errorMessage, NULL);
	//if (FAILED(result))
	//{
	//	// If the shader failed to compile it should have writen something to the error message.
	//	if (errorMessage)
	//	{
	//		OutputShaderErrorMessage(errorMessage, hwnd, psFilename);
	//	}
	//	// If there was nothing in the error message then it simply could not find the file itself.
	//	else
	//	{
	//		MessageBox(hwnd, psFilename, "Missing Shader File", MB_OK);
	//	}

	//	return false;
	//}

	// Create the vertex shader from the buffer.
	result = device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), NULL, &m_vertexShader);
	if (FAILED(result))
	{
		return false;
	}

	// Create the pixel shader from the buffer.
	result = device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, &m_pixelShader);
	if (FAILED(result))
	{
		return false;
	}

	// Create the vertex input layout description.
	// This setup needs to match the VertexType stucture in the ModelClass and in the shader.
	polygonLayout[0].SemanticName = "POSITION";
	polygonLayout[0].SemanticIndex = 0;
	polygonLayout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	polygonLayout[0].InputSlot = 0;
	polygonLayout[0].AlignedByteOffset = 0;
	polygonLayout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[0].InstanceDataStepRate = 0;

	//polygonLayout[1].SemanticName = "COLOR";
	//polygonLayout[1].SemanticIndex = 0;
	//polygonLayout[1].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	//polygonLayout[1].InputSlot = 0;
	//polygonLayout[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	//polygonLayout[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	//polygonLayout[1].InstanceDataStepRate = 0;

	// Get a count of the elements in the layout.
	numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

	// Create the vertex input layout.
	result = device->CreateInputLayout(polygonLayout, numElements, vertexShaderBuffer->GetBufferPointer(),
		vertexShaderBuffer->GetBufferSize(), &m_layout);
	if (FAILED(result))
	{
		return false;
	}

	// Release the vertex shader buffer and pixel shader buffer since they are no longer needed.
	vertexShaderBuffer->Release();
	vertexShaderBuffer = 0;

	pixelShaderBuffer->Release();
	pixelShaderBuffer = 0;

	// Setup the description of the dynamic matrix constant buffer that is in the vertex shader.
	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;

	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	result = device->CreateBuffer(&matrixBufferDesc, NULL, &m_matrixBuffer);
	if (FAILED(result))
	{
		return false;
	}

	//colorBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	//colorBufferDesc.ByteWidth = sizeof(MatrixBufferType);
	//colorBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	//colorBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	//colorBufferDesc.MiscFlags = 0;
	//colorBufferDesc.StructureByteStride = 0;

	//result = device->CreateBuffer(&colorBufferDesc, NULL, &m_colorBuffer);
	//if (FAILED(result))
	//{
	//	return false;
	//}

	return true;
}

void ShaderController::Render(ID3D11DeviceContext* deviceContext, int indexCount,
	DirectX::XMMATRIX &worldMatrix, DirectX::XMMATRIX &viewMatrix, DirectX::XMMATRIX &projectionMatrix)
{
	if (SetBuffersOnRender(deviceContext, worldMatrix, viewMatrix, projectionMatrix))
	{
		RenderShader(deviceContext, indexCount);
	}
}

bool ShaderController::SetBuffersOnRender(ID3D11DeviceContext* deviceContext, DirectX::XMMATRIX &worldMatrix, DirectX::XMMATRIX &viewMatrix,
	DirectX::XMMATRIX &projectionMatrix)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBufferType* dataPtr;
	unsigned int bufferNumber;

	// Transpose the matrices to prepare them for the shader.
	//DirectX::XMMatrixTranspose(worldMatrix);
	//DirectX::XMMatrixTranspose(viewMatrix);
	//DirectX::XMMatrixTranspose(projectionMatrix);

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
	return true;
}


void ShaderController::RenderShader(ID3D11DeviceContext* deviceContext, int indexCount)
{
	deviceContext->IASetInputLayout(m_layout);

	deviceContext->VSSetShader(m_vertexShader, NULL, 0);
	deviceContext->PSSetShader(m_pixelShader, NULL, 0);

	deviceContext->DrawIndexed(indexCount, 0, 0);
}