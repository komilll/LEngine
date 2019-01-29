#pragma once
#ifndef _UI_TICK_H_
#define _UI_TICK_H_

#include "UIBase.h"

class UITick : public UIBase
{
public:
	UITick();

	///<summary> Create tick field (Square) with size and position </summary>
	bool Initialize(D3DClass *d3d, float positionX, float positionY, float size);
	bool ChangeTick();

	virtual bool MouseOnArea(MouseClass* mouse) override;

private:
	D3DClass* m_D3D;
	float m_positionX, m_positionY, m_size;
	float m_leftMost, m_rightMost, m_topMost, m_bottomMost;
	bool m_tickState;
};

#endif // !_UI_TICK_H_