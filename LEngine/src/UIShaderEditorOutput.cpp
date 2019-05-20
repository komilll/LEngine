#include "UIShaderEditorOutput.h"

UIShaderEditorOutput::UIShaderEditorOutput()
{
	UIBase::UIBase();
}

bool UIShaderEditorOutput::MouseOnArea(MouseClass * mouse)
{
	bool result = false;
	float mouseX{ 0 };
	float mouseY{ 0 };
	POINT p = mouse->CurrentMouseLocation();

	//Calculate mouse X
	mouseX = (float)p.x / (float)m_D3D->GetWindowSize().x;
	mouseX = mouseX * 2.0f - 1.0f;
	if (mouseX > 1.0f)
		mouseX = 1.0f;
	else if (mouseX < -1.0f)
		mouseX = -1.0f;

	//Calculate mouse Y
	mouseY = (float)p.y / (float)m_D3D->GetWindowSize().y;
	mouseY = mouseY * 2.0f - 1.0f;
	if (mouseY > 1.0f)
		mouseY = 1.0f;
	else if (mouseY < -1.0f)
		mouseY = -1.0f;

	mouseY *= -1.0f;

	if (mouseX >(min_X + m_translationX) && mouseX < (max_X + m_translationX) &&
		mouseY >(min_Y + m_translationY) && mouseY < (max_Y + m_translationY))
	{
		result = true;
	}

	return result;
}

bool UIShaderEditorOutput::Initialize(D3DClass * d3d, ModelClass::ShapeSize shape, float left, float right, float top, float bottom)
{
	if (m_returnType == "float" || m_returnType == "")
	{
		if (!BaseShaderClass::Initialize(d3d->GetDevice(), *d3d->GetHWND(), UI_SHADER_VS, UI_SHADER_PS, BaseShaderClass::vertexInputType(GetInputNames(), GetInputFormats())))
			return false;
	}
	else if (!BaseShaderClass::Initialize(d3d->GetDevice(), *d3d->GetHWND(), PIN_SHADER_VS, PIN_SHADER_PS, BaseShaderClass::vertexInputType(GetInputNames(), GetInputFormats())))
	{
		return false;
	}

	m_D3D = d3d;
	min_X = left;
	max_X = right;
	min_Y = bottom;
	max_Y = top;

	if (m_returnType == "float2")
		LoadTexture(d3d->GetDevice(), L"float2.png", m_pinTexture, m_pinTextureView, false);
	else if (m_returnType == "float3")
		LoadTexture(d3d->GetDevice(), L"float3.png", m_pinTexture, m_pinTextureView, false);
	else if (m_returnType == "float4")
		LoadTexture(d3d->GetDevice(), L"float4.png", m_pinTexture, m_pinTextureView, false);
	StopDragging();

	return InitializeModelGeneric(d3d->GetDevice(), shape, left, right, top, bottom);
}

bool UIShaderEditorOutput::Initialize(D3DClass * d3d, float centerX, float centerY, float size)
{
	if (!BaseShaderClass::Initialize(d3d->GetDevice(), *d3d->GetHWND(), UI_SHADER_VS, UI_SHADER_PS, BaseShaderClass::vertexInputType(GetInputNames(), GetInputFormats())))
		return false;

	if (m_returnType == "float2")
		LoadTexture(d3d->GetDevice(), L"float2.png", m_pinTexture, m_pinTextureView, false);
	else if (m_returnType == "float3")
		LoadTexture(d3d->GetDevice(), L"float3.png", m_pinTexture, m_pinTextureView, false);
	else if (m_returnType == "float4")
		LoadTexture(d3d->GetDevice(), L"float4.png", m_pinTexture, m_pinTextureView, false);

	return InitializeSquare(d3d->GetDevice(), centerX, centerY, size);
}

void UIShaderEditorOutput::Move(float x, float y)
{
	m_translationX += x;
	m_translationY += y;
}

void UIShaderEditorOutput::GetTranslation(float & x, float & y)
{
	x = m_translationX;
	y = m_translationY;
}

void UIShaderEditorOutput::GetPosition(float & x, float & y)
{
	x = (max_X - min_X) * 0.5f + min_X + m_translationX;
	y = (max_Y - min_Y) * 0.5f + min_Y + m_translationY;
}

bool UIShaderEditorOutput::Render(ID3D11DeviceContext * deviceContext)
{
	XMMATRIX tmpMatrix;
	tmpMatrix *= 0;
	tmpMatrix.r[0] = XMVECTOR{ m_translationX, m_translationY, 0, 0 };

	return UIBase::Render(deviceContext, 0, tmpMatrix, tmpMatrix * 0, tmpMatrix * 0);
}

bool UIShaderEditorOutput::SetShaderParameters(ID3D11DeviceContext * deviceContext, XMMATRIX & worldMatrix, XMMATRIX & viewMatrix, XMMATRIX & projectionMatrix)
{
	if (UIBase::SetShaderParameters(deviceContext, worldMatrix, viewMatrix, projectionMatrix) == false)
		return false;

	if (m_returnType != "float" && m_returnType != "")
		deviceContext->PSSetShaderResources(0, 1, &m_pinTextureView);

	return true;
}

void UIShaderEditorOutput::StartDragging()
{
	m_dragged = true;
	if (m_returnType == "float")
		ChangeColor(1.0, 1.0f, 1.0f, 1.0f);
	else
		ChangeColor(2.5f, 2.5f, 2.5f, 1.0f);
}

void UIShaderEditorOutput::StopDragging()
{
	m_dragged = false;
	if (m_returnType == "float")
		ChangeColor(0.7f, 0.7f, 0.7f, 1.0f);
	else
		ChangeColor(1.0f, 1.0f, 1.0f, 1.0f);
}

bool UIShaderEditorOutput::IsDragging()
{
	return m_dragged;
}
