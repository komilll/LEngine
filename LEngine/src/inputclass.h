#ifndef _INPUTCLASS_H_
#define _INPUTCLASS_H_

#include <array>

class InputClass
{
public:
	void KeyDown(unsigned int keyIndex);
	void KeyUp(unsigned int keyIndex);

	bool IsKeyDown(unsigned int keyIndex) const;
	bool IsLetterKeyDown() const;
	bool IsNumberKeyDown() const;
	bool IsAlphanumericKeyDown() const;

private:
	std::array<bool, 256> m_keys{ false };
};

#endif