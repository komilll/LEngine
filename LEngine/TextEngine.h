#pragma once
#pragma warning(disable:4996)
#ifndef _TEXT_ENGINE_H
#define _TEXT_ENGINE_H

#include <d3d11.h>
#include <SpriteFont.h>
#include <DirectXMath.h>
#include <vector>
#include "d3dclass.h"

class TextEngine
{
public:
	enum Align{ LEFT, RIGHT, CENTER };

public:
	struct FontData
	{
		DirectX::XMVECTOR origin{ 0.0, 0.0, 0.0, 0.0 };
		float posX = 0;
		float posY = 0;
		float scale = 0;
		std::string text = "";
		XMVECTOR color = Colors::White;

		void SetText(std::string newString)
		{
			text = newString;
			for (int i = 0; i < text.length(); i++)
			{
				if (text[i] == '.')
					text = text.substr(0, i + 1 + 2);
			}
		}
	};

public:
	TextEngine();

	void Initialize(ID3D11Device* d3d, wchar_t const* fontPath);
	FontData* WriteText(ID3D11DeviceContext* deviceContext, float screenWidth, float screenHeight, float posX, float posY, std::string text, float scale = 1.0f, 
		Align align = Align::LEFT, XMVECTOR color = DirectX::Colors::White);
	void RenderText(ID3D11DeviceContext* deviceContext, float screenWidth, float screenHeight);

private:
	std::unique_ptr<DirectX::SpriteFont> m_font;
	std::vector<FontData> m_data;
};
#endif // !_TEXT_ENGINE_H