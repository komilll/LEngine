#pragma once
#ifndef _UIBACKGROUND_H_
#define _UIBACKGROUND_H_

#include "UIBase.h"
#include "d3dclass.h"

//TODO Replaced by ImGUI - Delete or add constructor otherwise
////////////////////////////////////////////
// PROPABLY WILL BE REPLACE BY dxguid.lib //
////////////////////////////////////////////
class UIBackground : public UIBase
{
public:
	///<summary> Initialize shape: Rectangle/Triangle </summary>
	bool Initialize(D3DClass* d3d, ModelClass::ShapeSize shape, float left, float right, float top, float bottom);
	///<summary> Initialize shape: Square </summary>
	bool Initialize(D3DClass* d3d, float centerX, float centerY, float size);
};

#endif // !_UIBACKGROUND_H_