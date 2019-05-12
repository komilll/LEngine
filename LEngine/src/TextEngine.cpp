#include "TextEngine.h"

TextEngine::TextEngine()
{
}

void TextEngine::Initialize(ID3D11Device* device, wchar_t const* fontPath)
{
	m_font = std::make_unique<DirectX::SpriteFont>(device, fontPath);
}

TextEngine::FontData* TextEngine::WriteText(ID3D11DeviceContext* deviceContext, float screenWidth, float screenHeight, float posX, float posY, std::string text_, float scale_, Align align_, XMVECTOR color_)
{
	FontData data;

	m_spriteBatch = std::make_unique<SpriteBatch>(deviceContext);
	std::wstring wstr = std::wstring(text_.begin(), text_.end());

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
	return &data;
}

void TextEngine::RenderText(ID3D11DeviceContext * deviceContext, float screenWidth, float screenHeight)
{
	DirectX::XMVECTOR fontPos;

	m_spriteBatch->Begin();
	//Fetch data from list and render each text
	for (int i = 0; i < m_data.size(); i++)
	{
		fontPos.m128_f32[0] = m_data.at(i).posX;
		fontPos.m128_f32[1] = m_data.at(i).posY;

		std::wstring wstr = std::wstring(m_data.at(i).text.begin(), m_data.at(i).text.end());

		m_font->DrawString(m_spriteBatch.get(), wstr.c_str(), fontPos, m_data.at(i).color, 0.0f, m_data.at(i).origin, m_data.at(i).scale);
	}
	m_spriteBatch->End();
}

TextEngine::FontData * TextEngine::GetData(int index)
{
	return &m_data.at(index);
}
