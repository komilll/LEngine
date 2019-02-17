#include "UIBackground.h"


bool UIBackground::Initialize(D3DClass * d3d, ModelClass::ShapeSize shape, float left, float right, float top, float bottom)
{
	if (!BaseShaderClass::Initialize(d3d->GetDevice(), *d3d->GetHWND(), UI_SHADER_VS, UI_SHADER_PS, BaseShaderClass::vertexInputType(GetInputNames(), GetInputFormats())))
		return false;

	return InitializeModelGeneric(d3d->GetDevice(), shape, left, right, top, bottom);
}

bool UIBackground::Initialize(D3DClass * d3d, float centerX, float centerY, float size)
{
	if (!BaseShaderClass::Initialize(d3d->GetDevice(), *d3d->GetHWND(), UI_SHADER_VS, UI_SHADER_PS, BaseShaderClass::vertexInputType(GetInputNames(), GetInputFormats())))
		return false;

	return InitializeSquare(d3d->GetDevice(), centerX, centerY, size);
}
