#include "UIBase.h"

UIBase::UIBase()
{
	BaseShaderClass::BaseShaderClass();
}

bool UIBase::InitializeModelGeneric(ID3D11Device * device, ModelClass::ShapeSize shape, float left, float right, float top, float bottom, bool withTex)
{
	m_model = new ModelClass;
	if (!m_model->Initialize(device, shape, left, right, top, bottom, withTex))
		return false;

	return true;
}

bool UIBase::InitializeSquare(ID3D11Device * device, float centerX, float centerY, float size, bool isEmpty, bool withTex)
{
	m_model = new ModelClass;
	if (!m_model->InitializeSquare(device, centerX, centerY, size, isEmpty, withTex))
		return false;

	return true;
}

//Use standarized input data; Need to be changed and upgraded
std::vector<LPCSTR> UIBase::GetInputNames()
{
	std::vector <LPCSTR> names;
	names.push_back("position");
	names.push_back("texcoord");
	names.push_back("normal");
	names.push_back("tangent");
	names.push_back("binormal");
	return names;
}

//Use standarized input formats; Need to be changed and upgraded
std::vector<DXGI_FORMAT> UIBase::GetInputFormats()
{
	std::vector <DXGI_FORMAT> formats;
	formats.push_back(DXGI_FORMAT_R32G32B32_FLOAT);
	formats.push_back(DXGI_FORMAT_R32G32_FLOAT);
	formats.push_back(DXGI_FORMAT_R32G32B32_FLOAT);
	formats.push_back(DXGI_FORMAT_R32G32B32_FLOAT);
	formats.push_back(DXGI_FORMAT_R32G32B32_FLOAT);
	return formats;
}

void UIBase::GetColor(XMFLOAT4 & color)
{
	color = m_uiColor;
}

bool UIBase::Render(ID3D11DeviceContext * deviceContext, int indexCount, XMMATRIX & worldMatrix, XMMATRIX & viewMatrix, XMMATRIX & projectionMatrix)
{
	if (m_model == nullptr)
		return false;
	m_model->Render(deviceContext);

	if (!SetShaderParameters(deviceContext, worldMatrix, viewMatrix, projectionMatrix))
		return false;

	indexCount = m_model->GetIndexCount();
	RenderShader(deviceContext, indexCount);

	return true;
}

void UIBase::ChangeColor(XMFLOAT4 color)
{
	m_uiColor = color;
}

void UIBase::ChangeColor(float r, float g, float b, float a)
{
	m_uiColor = XMFLOAT4(r, g, b, a);
}

void UIBase::ChangeAlpha(float alpha)
{
	m_uiColor.w = alpha;
}

bool UIBase::MouseOnArea(MouseClass* mouse)
{
	return false;
}

ModelClass * UIBase::GetModel()
{
	return m_model;
}

bool UIBase::CreateBufferAdditionals(ID3D11Device *& device)
{
	BaseShaderClass::CreateBufferAdditionals(device);
	D3D11_BUFFER_DESC tempBufferDesc;

	tempBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	tempBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	tempBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	tempBufferDesc.MiscFlags = 0;
	tempBufferDesc.StructureByteStride = 0;

	tempBufferDesc.ByteWidth = sizeof(AppearanceBuffer);
	if (FAILED(device->CreateBuffer(&tempBufferDesc, NULL, &m_appearanceBuffer)))
		return false;

	m_buffers = { m_appearanceBuffer };
}

bool UIBase::CreateSamplerState(ID3D11Device * device)
{
	return BaseShaderClass::CreateSamplerState(device);
}

bool UIBase::SetShaderParameters(ID3D11DeviceContext * deviceContext, XMMATRIX & worldMatrix, XMMATRIX & viewMatrix, XMMATRIX & projectionMatrix)
{
	if (BaseShaderClass::SetShaderParameters(deviceContext, worldMatrix, viewMatrix, projectionMatrix) == false)
		return false;

	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	AppearanceBuffer* dataPtr2;
	unsigned int bufferNumber;

	/////// PIXEL BUFFERS ///////
	//Appearance buffer
	result = deviceContext->Map(m_appearanceBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
		return false;

	dataPtr2 = (AppearanceBuffer*)mappedResource.pData;
	dataPtr2->color = m_uiColor;

	deviceContext->Unmap(m_appearanceBuffer, 0);
	bufferNumber = 0;
	deviceContext->PSSetConstantBuffers(bufferNumber, 1, &m_appearanceBuffer);

	return true;
}