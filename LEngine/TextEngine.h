#pragma once
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

private:
	struct FontData
	{
		DirectX::XMVECTOR origin{ 0.0, 0.0, 0.0, 0.0 };
		float posX = 0;
		float posY = 0;
		float scale = 0;
		const wchar_t* text = nullptr;
		XMVECTOR color = Colors::White;
	};

public:
	TextEngine();

	void Initialize(ID3D11Device* d3d, wchar_t const* fontPath);
	void WriteText(ID3D11DeviceContext* deviceContext, float screenWidth, float screenHeight, float posX, float posY, const wchar_t* text, float scale = 1.0f, 
		Align align = Align::LEFT, XMVECTOR color = DirectX::Colors::White);
	void RenderText(ID3D11DeviceContext* deviceContext, float screenWidth, float screenHeight);

private:
	std::unique_ptr<DirectX::SpriteFont> m_font;
	std::vector<FontData> m_data;
};
#endif // !_TEXT_ENGINE_H