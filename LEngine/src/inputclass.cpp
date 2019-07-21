#include "inputclass.h"

void InputClass::KeyDown(unsigned int keyIndex)
{
	m_keys[keyIndex] = true;
}


void InputClass::KeyUp(unsigned int keyIndex)
{
	m_keys[keyIndex] = false;
}


bool InputClass::IsKeyDown(unsigned int keyIndex) const
{
	return m_keys[keyIndex];
}

bool InputClass::IsLetterKeyDown() const
{
	for (unsigned int i = 65; i <= 90; ++i)
	{
		if (IsKeyDown(i))
			return true;
	}
	return false;
}

bool InputClass::IsNumberKeyDown() const
{
	for (unsigned int i = 48; i <= 57; ++i)
	{
		if (IsKeyDown(i))
			return true;
	}
	return false;
}

bool InputClass::IsAlphanumericKeyDown() const
{
	return IsLetterKeyDown() || IsNumberKeyDown();
}
