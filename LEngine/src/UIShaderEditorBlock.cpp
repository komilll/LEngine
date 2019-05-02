#include "UIShaderEditorBlock.h"

UIShaderEditorBlock::UIShaderEditorBlock()
{
	UIBase::UIBase();
}

bool UIShaderEditorBlock::MouseOnArea(MouseClass * mouse)
{
	POINT p;
	if (!GetCursorPos(&p))
	{
		return false;
	}

	bool result = false;
	float mouseX{ 0 };
	float mouseY{ 0 };
	RECT desktop;
	HWND hwnd = ::GetDesktopWindow();

	::GetWindowRect(hwnd, &desktop);
	//Calculate mouse X
	p.x -= (desktop.right - m_D3D->GetWindowSize().x) * 0.5f;
	mouseX = (float)p.x / (float)m_D3D->GetWindowSize().x;
	mouseX = mouseX * 2.0f - 1.0f;
	if (mouseX > 1.0f)
		mouseX = 1.0f;
	else if (mouseX < -1.0f)
		mouseX = -1.0f;

	//Calculate mouse Y
	p.y -= (desktop.bottom - m_D3D->GetWindowSize().y) * 0.5f;
	mouseY = (float)p.y / (float)m_D3D->GetWindowSize().y;
	mouseY = mouseY * 2.0f - 1.0f;
	if (mouseY > 1.0f)
		mouseY = 1.0f;
	else if (mouseY < -1.0f)
		mouseY = -1.0f;

	mouseY *= -1.0f;
	
	if (mouseX > (min_X + m_translationX) && mouseX < (max_X + m_translationX) &&
		mouseY > (min_Y + m_translationY) && mouseY < (max_Y + m_translationY) )
	{
		result = true;
	}

	return result;
}

bool UIShaderEditorBlock::Initialize(D3DClass * d3d, ModelClass::ShapeSize shape, float left, float right, float top, float bottom)
{
	if (!BaseShaderClass::Initialize(d3d->GetDevice(), *d3d->GetHWND(), UI_SHADER_VS, UI_SHADER_PS, BaseShaderClass::vertexInputType(GetInputNames(), GetInputFormats())))
		return false;

	m_D3D = d3d;
	min_X = left;
	max_X = right;
	min_Y = bottom;
	max_Y = top;

	return InitializeModelGeneric(d3d->GetDevice(), shape, left, right, top, bottom);
}

bool UIShaderEditorBlock::Initialize(D3DClass * d3d, float centerX, float centerY, float size)
{
	if (!BaseShaderClass::Initialize(d3d->GetDevice(), *d3d->GetHWND(), UI_SHADER_VS, UI_SHADER_PS, BaseShaderClass::vertexInputType(GetInputNames(), GetInputFormats())))
		return false;

	return InitializeSquare(d3d->GetDevice(), centerX, centerY, size);
}

void UIShaderEditorBlock::Move(float x, float y)
{
	m_translationX = x;
	m_translationY = y;
}

bool UIShaderEditorBlock::Render(ID3D11DeviceContext * deviceContext)
{
	XMMATRIX tmpMatrix;
	tmpMatrix *= 0;
	tmpMatrix.r[0] = XMVECTOR{ m_translationX, m_translationY, 0, 0 };

	return UIBase::Render(deviceContext, 0, tmpMatrix, tmpMatrix * 0, tmpMatrix * 0);
}
