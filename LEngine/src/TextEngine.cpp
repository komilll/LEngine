#include "TextEngine.h"

void TextEngine::Initialize(ID3D11Device* device, wchar_t const* fontPath)
{
	//TODO Do in constructor
	//TODO Should it be unique pointer?
	m_font = std::make_unique<DirectX::SpriteFont>(device, fontPath);
}

TextEngine::FontData* TextEngine::WriteText(ID3D11DeviceContext* deviceContext, float screenWidth, float screenHeight, float posX, float posY, std::string text_, float scale_, Align align_, XMVECTOR color_)
{
	FontData data;

	//TODO Why unique ptr?
	m_spriteBatch = std::make_unique<SpriteBatch>(deviceContext);
	const std::wstring wstr = std::wstring(text_.begin(), text_.end());

	//Measure new text length and change position based on aligments
	data.posX = screenWidth * (0.5f + posX * 0.5f);
	data.posY = screenHeight * (0.5f - posY * 0.5f);
	data.text = text_;
	data.scale = scale_;
	data.color = color_;
	//If left - left default contructor { 0, 0, 0, 0 }
	if (align_ == Align::CENTER)
		data.origin = m_font->MeasureString(wstr.c_str()) * 0.5f;
	else if (align_ == Align::RIGHT)
		data.origin = m_font->MeasureString(wstr.c_str());

	//Add to list from which text will be rendered; Store and use further only via index
	data.SetIndex(m_data.size());
	data.textEngineRef = this;
	m_data.push_back(data);
	return &data; //TODO Return address of local variable of temporary
}

void TextEngine::RenderText(ID3D11DeviceContext * deviceContext, float screenWidth, float screenHeight)
{
	DirectX::XMVECTOR fontPos{ 0.0f, 0.0f };

	if (m_spriteBatch)
	{
		m_spriteBatch->Begin();
		//Fetch data from list and render each text
		for (const auto& data : m_data)
		{
			fontPos.m128_f32[0] = data.posX;
			fontPos.m128_f32[1] = data.posY;

			std::wstring wstr = std::wstring(data.text.begin(), data.text.end());

			m_font->DrawString(m_spriteBatch.get(), wstr.c_str(), fontPos, data.color, 0.0f, data.origin, data.scale);
		}
		m_spriteBatch->End();
	}
}

TextEngine::FontData * TextEngine::GetData(int index)
{
	return &m_data.at(index);
}
