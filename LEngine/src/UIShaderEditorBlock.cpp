#include "UIShaderEditorBlock.h"

UIShaderEditorBlock::UIShaderEditorBlock()
{
	UIBase::UIBase();
	ChangeColor(blockColor);
}

UIShaderEditorBlock::UIShaderEditorBlock(XMFLOAT2 startPosition)
{
	UIBase::UIBase();
	m_moveAfterInitializing = true;
	m_movemementAfterInitialization = startPosition;
	ChangeColor(blockColor);
}

UIShaderEditorBlock::UIShaderEditorBlock(XMFLOAT2 startPosition, std::string functionName, std::string returnType, vector<std::string> argumentTypes, int id)
{
	UIBase::UIBase();
	m_moveAfterInitializing = true;
	m_movemementAfterInitialization = startPosition;
	ChangeColor(blockColor);

	m_functionName = functionName;
	m_returnType = returnType;
	m_argumentTypes = argumentTypes;
	m_blockID = id;
}

bool UIShaderEditorBlock::MouseOnArea(MouseClass * mouse)
{
	const POINT p = mouse->CurrentMouseLocation();
	
	//Calculate mouse X
	float mouseX = static_cast<float>(p.x) / static_cast<float>(m_D3D->GetWindowSize().x);
	mouseX = mouseX * 2.0f - 1.0f;
	if (mouseX > 1.0f)
		mouseX = 1.0f;
	else if (mouseX < -1.0f)
		mouseX = -1.0f;

	//Calculate mouse Y
	float mouseY = static_cast<float>(p.y) / static_cast<float>(m_D3D->GetWindowSize().y);
	mouseY = mouseY * 2.0f - 1.0f;
	if (mouseY > 1.0f)
		mouseY = 1.0f;
	else if (mouseY < -1.0f)
		mouseY = -1.0f;

	mouseY *= -1.0f;

	if (mouseX > (m_blockVertices.minX + m_translationX) * m_scale && mouseX < (m_blockVertices.maxX + m_translationX) * m_scale &&
		mouseY > (m_blockVertices.minY + m_translationY) * m_scale && mouseY < (m_blockVertices.maxY + m_translationY) * m_scale)
	{
		return true;
	}

	return false;
}

bool UIShaderEditorBlock::Initialize(D3DClass * d3d, int inCount, int outCount)
{
	//TODO Why late initialization? Cannot be done in constructor?
	m_D3D = d3d;
	if (!BaseShaderClass::Initialize(d3d->GetDevice(), *d3d->GetHWND(), L"uiline.vs", L"uiline.ps", BaseShaderClass::vertexInputType(GetInputNames(), GetInputFormats())))
		return false;

	CalculateBlockSize(inCount, outCount);
	InitializeInputNodes(inCount);
	InitializeOutputNodes(outCount);
	if (m_moveAfterInitializing)
	{
		Move(m_movemementAfterInitialization.x, m_movemementAfterInitialization.y);
	}
	//Create text on block
	m_textEngine = new TextEngine;
	m_textEngine->Initialize(d3d->GetDevice(), L"Fonts/Calibri_12.spritefont");
	m_blockName = m_functionName;
	for (char& c : m_blockName)
		c = toupper(c);
	m_textEngine->WriteText(d3d->GetDeviceContext(), d3d->GetWindowSize().x, d3d->GetWindowSize().y, m_translationX, m_translationY, m_blockName, 1.0f, TextEngine::Align::LEFT);
	m_textEngine->WriteText(d3d->GetDeviceContext(), d3d->GetWindowSize().x, d3d->GetWindowSize().y, m_translationX, m_translationY, m_blockName, 1.0f, TextEngine::Align::LEFT);

	//Create outline
	m_outlineObject = new UIBase;
	if (!m_outlineObject->Initialize(d3d->GetDevice(), *d3d->GetHWND(), L"uiline.vs", L"uiline.ps", BaseShaderClass::vertexInputType(GetInputNames(), GetInputFormats())))
		return false;
	if (!m_outlineObject->InitializeModelGeneric(d3d->GetDevice(), CalculateOutlineSize(m_blockVertices), false, true))
		return false;
	m_outlineObject->ChangeColor(outlineColor);

	//Create color preview if float3/float4
	if (m_functionName == "float3" || m_functionName == "float4")
	{
		m_colorPreview = new UIBase();
		if (!m_colorPreview->Initialize(m_D3D->GetDevice(), *m_D3D->GetHWND(), L"uiline.vs", L"uiline.ps", BaseShaderClass::vertexInputType(GetInputNames(), GetInputFormats())))
			return false;
		constexpr float sizeX = 0.08f;
		if (!m_colorPreview->InitializeModelGeneric(m_D3D->GetDevice(), UIBase::RectangleVertices::RectangleVertices(-sizeX, sizeX, -sizeX * 16.0f / 9.0f, sizeX * 16.0f / 9.0f)))
			return false;
		m_colorPreview->ChangeColor(1.0f, 0.0f, 0.0f, 0.5f);
	}

	//Create texture preview
	else if (m_functionName == "texture")
	{
		constexpr float sizeX = 0.08f;
		m_texturePreview = new UITextureMoveable();
		if (!m_texturePreview->Initialize(m_D3D->GetDevice(), *m_D3D->GetHWND(), L"uitexture.vs", L"uitexture.ps", BaseShaderClass::vertexInputType(GetInputNames(), GetInputFormats())))
			return false;
		if (!m_texturePreview->InitializeModelGeneric(m_D3D->GetDevice(), UIBase::RectangleVertices::RectangleVertices(-sizeX, sizeX, -sizeX * 16.0f / 9.0f, sizeX * 16.0f / 9.0f)))
			return false;		
	}

	return (m_blockInitialized = InitializeModelGeneric(d3d->GetDevice(), m_blockVertices));
}

void UIShaderEditorBlock::Move(float x, float y)
{
	for (const auto& node : m_inputNodes)
		node->Move(x, y);

	for (const auto& node : m_outputNodes)
		node->Move(x, y);

	m_translationX += x;
	m_translationY += y;
}

void UIShaderEditorBlock::StartDragging()
{
	m_dragged = true;
	m_focused = true;
}

void UIShaderEditorBlock::StopDragging()
{
	m_dragged = false;
}

bool UIShaderEditorBlock::IsDragging() const
{
	return m_dragged;
}

bool UIShaderEditorBlock::IsPinDragging() const
{
	return m_pinDragged;
}

UIShaderEditorBlock::EDragPinBehaviour UIShaderEditorBlock::DragPins(MouseClass * mouse, UIShaderEditorOutput*& out)
{
	for (const auto& pin : m_outputNodes)
	{
		if (pin->IsDragging())
		{
			if (mouse->GetLMBPressed() == false)
			{
				pin->StopDragging();
				m_pinDragged = false;
				out = pin;
				//return pin;
				return EDragPinBehaviour::Stop;
			}
			out = pin;
			//return pin;
			return EDragPinBehaviour::Dragging;
		}
	}

	for (const auto& pin : m_outputNodes)
	{
		if (pin->MouseOnArea(mouse))
		{
			if (mouse->GetMMBPressed())
			{
				pin->m_toDeleteLine = true;
				m_pinDragged = false;
				out = nullptr;
				//return nullptr;
				return EDragPinBehaviour::Break;
			}
			if (mouse->GetLMBPressed())
			{
				pin->StartDragging();
				m_pinDragged = true;
				out = pin;
				//return pin;
				return EDragPinBehaviour::Start;
			}
		}
	}

	for (const auto& pin : m_inputNodes)
	{
		if (pin->MouseOnArea(mouse))
		{
			if (mouse->GetMMBPressed())
			{
				pin->m_connectedOutputNode = nullptr;
				m_pinDragged = false;
				out = nullptr;
				//return nullptr;
				return EDragPinBehaviour::Break;
			}
		}
	}

	out = nullptr;
	//return nullptr;
	return EDragPinBehaviour::Nothing;
}

bool UIShaderEditorBlock::Render(ID3D11DeviceContext * deviceContext)
{
	if (!m_blockInitialized)
		return true;

	XMMATRIX worldMatrix = XMMatrixIdentity();
	worldMatrix = XMMatrixMultiply(worldMatrix, XMMatrixTranslation(m_translationX, m_translationY, 0.0f));
	worldMatrix = XMMatrixMultiply(worldMatrix, XMMatrixScaling(m_scale, m_scale, m_scale));

	if (m_focused && m_outlineObject)
	{
		//Render outline
		if (m_outlineObject && !m_outlineObject->Render(deviceContext, 0, worldMatrix, worldMatrix * 0, worldMatrix * 0))
			return false;
	}
	worldMatrix = XMMatrixIdentity();
	worldMatrix = XMMatrixMultiply(worldMatrix, XMMatrixTranslation(m_translationX, m_translationY, 0.0f));
	worldMatrix = XMMatrixMultiply(worldMatrix, XMMatrixScaling(m_scale, m_scale, m_scale));

	if (UIBase::Render(deviceContext, 0, worldMatrix, worldMatrix * 0, worldMatrix * 0))
	{
		for (const auto& node : m_inputNodes)
		{
			if (node)
				node->Render(deviceContext);
		}

		for (const auto& node : m_outputNodes)
		{
			if (node)
				node->Render(deviceContext);
		}

		if (m_textEngine)
		{
			//TODO Too many code repetition - create function
			if (m_functionName == "float")
			{
				m_textEngine->GetData(0)->SetPosition((m_translationX - (m_blockVertices.maxX - m_blockVertices.minX) * 0.5f) * m_scale, 
					(m_translationY + 0.035f) * m_scale, m_D3D->GetWindowSize().x, m_D3D->GetWindowSize().y);
				ostringstream oss;
				oss << fixed << setprecision(2) << m_outputNodes[0]->m_value;
				m_textEngine->GetData(0)->text = "(" + oss.str() + ")";
				m_textEngine->GetData(0)->scale = m_scale;

				if (GetFirstOutputNode()->m_isVariable)
				{
					m_textEngine->GetData(1)->SetPosition((m_translationX - (m_blockVertices.maxX - m_blockVertices.minX) * 0.5f) * m_scale,
						(m_translationY + (0.175f + 0.025f) * (m_blockVertices.maxY / 0.2f)) * m_scale, m_D3D->GetWindowSize().x, m_D3D->GetWindowSize().y);
					m_textEngine->GetData(1)->text = GetFirstOutputNode()->GetVisibleName();
					m_textEngine->GetData(1)->scale = m_scale;
				}
				else
				{
					m_textEngine->GetData(1)->text = "";
					m_textEngine->GetData(1)->scale = 0.0f;
				}
			}
			else if (m_functionName == "float2")
			{
				m_textEngine->GetData(0)->SetPosition((m_translationX - (m_blockVertices.maxX - m_blockVertices.minX) * 0.5f) * m_scale,
					(m_translationY + 0.035f) * m_scale, m_D3D->GetWindowSize().x, m_D3D->GetWindowSize().y);
				m_textEngine->GetData(0)->text = "(";
				for (int i = 0; i < 2; ++i)
				{
					ostringstream oss;
					oss << fixed << setprecision(2) << m_outputNodes[0]->m_valueTwo[i];
					m_textEngine->GetData(0)->text += oss.str();
					if (i < 1)
						m_textEngine->GetData(0)->text += ", ";
				}
				m_textEngine->GetData(0)->text += ")";
				m_textEngine->GetData(0)->scale = m_scale;
				
				if (GetFirstOutputNode()->m_isVariable)
				{
					m_textEngine->GetData(1)->SetPosition((m_translationX - (m_blockVertices.maxX - m_blockVertices.minX) * 0.5f) * m_scale,
						(m_translationY + (0.175f + 0.025f) * (m_blockVertices.maxY / 0.2f)) * m_scale, m_D3D->GetWindowSize().x, m_D3D->GetWindowSize().y);
					m_textEngine->GetData(1)->text = GetFirstOutputNode()->GetVisibleName();
					m_textEngine->GetData(1)->scale = m_scale;
				}
				else
				{
					m_textEngine->GetData(1)->text = "";
					m_textEngine->GetData(1)->scale = 0.0f;
				}
			}
			else if (m_functionName == "float3")
			{
				m_textEngine->GetData(0)->SetPosition((m_translationX - (m_blockVertices.maxX - m_blockVertices.minX) * 0.5f) * m_scale,
					(m_translationY + 0.175f) * m_scale, m_D3D->GetWindowSize().x, m_D3D->GetWindowSize().y);
				m_textEngine->GetData(0)->text = "(";
				for (int i = 0; i < 3; ++i)
				{
					ostringstream oss;
					oss << fixed << setprecision(2) << m_outputNodes[0]->m_valueThree[i];
					m_textEngine->GetData(0)->text += oss.str();
					if (i < 2)
						m_textEngine->GetData(0)->text += ", ";
				}
				m_textEngine->GetData(0)->text += ")";
				m_textEngine->GetData(0)->scale = m_scale;

				if (GetFirstOutputNode()->m_isVariable)
				{
					m_textEngine->GetData(1)->SetPosition((m_translationX - (m_blockVertices.maxX - m_blockVertices.minX) * 0.5f) * m_scale,
						(m_translationY + (0.175f + 0.025f) * (m_blockVertices.maxY / 0.2f)) * m_scale, m_D3D->GetWindowSize().x, m_D3D->GetWindowSize().y);
					m_textEngine->GetData(1)->text = GetFirstOutputNode()->GetVisibleName();
					m_textEngine->GetData(1)->scale = m_scale;
				}
				else
				{
					m_textEngine->GetData(1)->text = "";
					m_textEngine->GetData(1)->scale = 0.0f;
				}
			}
			else if (m_functionName == "float4")
			{
				m_textEngine->GetData(0)->SetPosition((m_translationX - (m_blockVertices.maxX - m_blockVertices.minX) * 0.5f) * m_scale,
					(m_translationY + 0.175f) * m_scale, m_D3D->GetWindowSize().x, m_D3D->GetWindowSize().y);
				m_textEngine->GetData(0)->text = "(";
				for (int i = 0; i < 4; ++i)
				{
					ostringstream oss;
					oss << fixed << setprecision(2) << m_outputNodes[0]->m_valueFour[i];
					m_textEngine->GetData(0)->text += oss.str();
					if (i < 3)
						m_textEngine->GetData(0)->text += ", ";
				}
				m_textEngine->GetData(0)->text += ")";
				m_textEngine->GetData(0)->scale = m_scale;

				if (GetFirstOutputNode()->m_isVariable)
				{
					m_textEngine->GetData(1)->SetPosition((m_translationX - (m_blockVertices.maxX - m_blockVertices.minX) * 0.5f) * m_scale,
						(m_translationY + (0.175f + 0.025f) * (m_blockVertices.maxY / 0.2f)) * m_scale, m_D3D->GetWindowSize().x, m_D3D->GetWindowSize().y);
					m_textEngine->GetData(1)->text = GetFirstOutputNode()->GetVisibleName();
					m_textEngine->GetData(1)->scale = m_scale;
				}
				else
				{
					m_textEngine->GetData(1)->text = "";
					m_textEngine->GetData(1)->scale = 0.0f;
				}
			}
			else
			{
				m_textEngine->GetData(0)->scale = m_scale;

				if (GetFirstOutputNode()->m_isVariable)
				{
					m_textEngine->GetData(0)->SetPosition((m_translationX - (m_blockVertices.maxX - m_blockVertices.minX) * 0.5f) * m_scale,
						(m_translationY + (0.175f - 0.015f) * (m_blockVertices.maxY / 0.2f)) * m_scale, m_D3D->GetWindowSize().x, m_D3D->GetWindowSize().y);

					m_textEngine->GetData(1)->SetPosition((m_translationX - (m_blockVertices.maxX - m_blockVertices.minX) * 0.5f) * m_scale,
						(m_translationY + (0.175f + 0.035f) * (m_blockVertices.maxY / 0.2f)) * m_scale, m_D3D->GetWindowSize().x, m_D3D->GetWindowSize().y);
					m_textEngine->GetData(1)->text = GetFirstOutputNode()->GetVisibleName();
					m_textEngine->GetData(1)->scale = m_scale;
				}
				else
				{
					m_textEngine->GetData(0)->SetPosition((m_translationX - (m_blockVertices.maxX - m_blockVertices.minX) * 0.5f) * m_scale,
						(m_translationY + 0.175f * (m_blockVertices.maxY / 0.2f)) * m_scale, m_D3D->GetWindowSize().x, m_D3D->GetWindowSize().y);

					m_textEngine->GetData(1)->text = "";
					m_textEngine->GetData(1)->scale = 0.0f;
				}
			}
			m_textEngine->RenderText(deviceContext, m_D3D->GetWindowSize().x, m_D3D->GetWindowSize().y);
		}

		if (m_functionName == "float3" || m_functionName == "float4")
		{
			if (m_colorPreview)
			{
				if (m_functionName == "float3")
					m_colorPreview->ChangeColor(GetFirstOutputNode()->m_valueThree[0], GetFirstOutputNode()->m_valueThree[1], GetFirstOutputNode()->m_valueThree[2], 1.0f);
				else if (m_functionName == "float4")
					m_colorPreview->ChangeColor(GetFirstOutputNode()->m_valueFour[0], GetFirstOutputNode()->m_valueFour[1], GetFirstOutputNode()->m_valueFour[2], 1.0f);

				constexpr float fixPosition = 0.04f;
				worldMatrix = XMMatrixIdentity();
				worldMatrix = XMMatrixMultiply(worldMatrix, XMMatrixTranslation(m_translationX - fixPosition, m_translationY - fixPosition, 0.0f));
				worldMatrix = XMMatrixMultiply(worldMatrix, XMMatrixScaling(m_scale, m_scale, m_scale));
				if (m_colorPreview && !m_colorPreview->Render(deviceContext, 0, worldMatrix, worldMatrix * 0, worldMatrix * 0))
					return false;
			}
		}
		else if (m_functionName == "texture")
		{
			if (m_texturePreview)
			{
				constexpr float fixPosition = 0.04f;
				worldMatrix = XMMatrixIdentity();
				worldMatrix = XMMatrixMultiply(worldMatrix, XMMatrixTranslation(m_translationX - fixPosition, m_translationY - fixPosition, 0.0f));
				worldMatrix = XMMatrixMultiply(worldMatrix, XMMatrixScaling(m_scale, m_scale, m_scale));
				m_texturePreview->LoadTexture(m_D3D->GetDevice(), GetFirstOutputNode()->m_connectedTextureView);
				if (m_texturePreview && !m_texturePreview->Render(deviceContext, 0, worldMatrix, worldMatrix * 0, worldMatrix * 0))
					return false;
			}
		}

		return true;
	}
	else
		return false;
}

std::string UIShaderEditorBlock::GenerateShaderCode(bool skipTabulator) const
{	
	std::string func;
	if (GetInputCount() == 0) //Variable
	{
		func = { (skipTabulator ? "" : "\t") };
		if (m_functionName != "texture") //Texture is already defined - skip
		{
			func += m_returnType + " " + m_variableName + " = ";
		}
		if (m_outputNodes.size() > 0)
		{
			std::ostringstream ss;
			if (m_functionName == "texture")
			{
				//Leave empty - already defined
			}
			else if (m_returnType == "float")
			{
				float val = m_outputNodes.at(0)->m_value;
				if ((int)val == val)
					ss << val << ".0f";
				else
					ss << val << "f";
				func += ss.str();
			}
			else if (m_returnType == "float2")
			{
				XMFLOAT2 val{ m_outputNodes.at(0)->m_valueTwo };
				func += "float2(";
				if ((int)val.x == val.x)
					ss << val.x << ".0f, ";
				else
					ss << val.x << "f, ";
				if ((int)val.y == val.y)
					ss << val.y << ".0f";
				else
					ss << val.y << "f";
				func += ss.str();
				func += ")";
			}
			else if (m_returnType == "float3")
			{
				XMFLOAT3 val{ m_outputNodes.at(0)->m_valueThree };
				func += "float3(";
				if ((int)val.x == val.x)
					ss << val.x << ".0f, ";
				else
					ss << val.x << "f, ";
				if ((int)val.y == val.y)
					ss << val.y << ".0f, ";
				else
					ss << val.y << "f, ";
				if ((int)val.z == val.z)
					ss << val.z << ".0f";
				else
					ss << val.z << "f";
				func += ss.str();
				func += ")";
			}
			else if (m_returnType == "float4")
			{
				XMFLOAT4 val{ m_outputNodes.at(0)->m_valueFour };
				func += "float4(";
				if ((int)val.x == val.x)
					ss << val.x << ".0f, ";
				else
					ss << val.x << "f, ";
				if ((int)val.y == val.y)
					ss << val.y << ".0f, ";
				else
					ss << val.y << "f, ";
				if ((int)val.z == val.z)
					ss << val.z << ".0f, ";
				else
					ss << val.z << "f, ";
				if ((int)val.w == val.w)
					ss << val.w << ".0f";
				else
					ss << val.w << "f";
				func += ss.str();
				func += ")";
			}
			func += ";\n";
		}
		else
		{
			func = "";
		}
	}
	else if (m_functionName != "sampletexture") //Function call
												//Skip for texture sample
	{
		func = { (skipTabulator ? "" : "\t") + m_returnType + " " + m_variableName + " = " + m_functionName + "(" };
		std::vector<std::string> args = {};

		for (unsigned int i = 0; i < m_inputNodes.size(); ++i)
		{
			const auto& node = m_inputNodes.at(i);
			if (node->m_connectedOutputNode != nullptr)
			{
				args.push_back(ConvertType(node->m_connectedOutputNode->m_variableName, node->m_connectedOutputNode->m_returnType, m_argumentTypes.at(i)));
			}
			else
			{
				args.push_back(ReturnEmptyForGivenType(m_argumentTypes.at(i)));
			}
		}
		for (unsigned int i = 0; i < args.size(); ++i)
		{
			func += args.at(i);
			if (i < args.size() - 1)
			{
				func += ", ";
			}
		}
		func += ");\n";
	}

	return func;
}

std::string UIShaderEditorBlock::ConvertType(std::string outName, std::string typeIn, std::string typeOut) const
{
	if (typeIn == typeOut)
	{
		return outName;
	}

	if (typeIn == "float")
	{
		if (typeOut == "float2")
			return "float2(" + outName + ", " + outName + ")";
		if (typeOut == "float3")
			return "float3(" + outName + ", " + outName + ", " + outName + ")";
		if (typeOut == "float4")
			return "float4(" + outName + ", " + outName + ", " + outName + ", " + outName + ")";
	}
	else if (typeIn == "float3")
	{
		if (typeOut == "float4")
			return "float4(" + outName + ".x, " + outName + ".y, " + outName + ".z, 1.0f)";
	}
	else if (typeIn == "float4")
	{
		if (typeOut == "float3")
			return "float3(" + outName + ".x, " + outName + ".y, " + outName + ".z)";
	}

	return "ERROR";
}

int UIShaderEditorBlock::GetInputCount() const
{
	return m_inputNodes.size();
}

void UIShaderEditorBlock::SetOutputPinName(std::string name)
{
	if (m_outputNodes.size() == 1 && m_outputNodes.at(0))
	{
		m_outputNodes.at(0)->m_variableName = name;
	}
}

UIShaderEditorOutput * UIShaderEditorBlock::GetFirstOutputNode()
{
	if (m_outputNodes.size() == 1)
		return m_outputNodes.at(0);
	else
		return nullptr;
}

std::string UIShaderEditorBlock::GetFunctionName() const
{
	return m_functionName;
}

std::string UIShaderEditorBlock::GetReturnType() const
{
	return m_returnType;
}

void UIShaderEditorBlock::SetScale(float scale)
{
	m_scale = scale;
}

bool UIShaderEditorBlock::TryToMarkBlock(RectangleVertices markingBounds)
{
	int correctPos = 0; //Need to get 2 points - correct X axis and correct Y axis
	const float minX = m_blockVertices.minX + m_translationX;
	const float maxX = m_blockVertices.maxX + m_translationX;
	const float minY = m_blockVertices.minY + m_translationY;
	const float maxY = m_blockVertices.maxY + m_translationY;

//TEST X POSITION
	//Block in middle X
	if (markingBounds.minX < minX && markingBounds.maxX > maxX)
	{
		correctPos++;
	}
	//Block on left crossed X
	else if (markingBounds.minX > minX && markingBounds.maxX > maxX && markingBounds.minX < maxX)
	{
		correctPos++;
	}
	//Block on right crossed X
	else if (markingBounds.maxX < maxX && markingBounds.minX < minX && markingBounds.maxX > minX)
	{
		correctPos++;
	}
	//Crossed through middle X
	else if (minX < markingBounds.minX && maxX > markingBounds.maxX)
	{
		correctPos++;
	}
//TEXT Y POSITION
	//Block in middle Y
	if (markingBounds.minY < minY && markingBounds.maxY > maxY)
	{
		correctPos++;
	}
	//Block on top crossed Y
	else if (markingBounds.maxY < maxY && markingBounds.minY < minY && markingBounds.maxY > minY)
	{
		correctPos++;
	}
	//Block on bottom crossed Y
	else if (markingBounds.maxY > maxY && markingBounds.minY > minY && markingBounds.minY < maxY)
	{
		correctPos++;
	}
	//Crossed through middle Y
	else if (markingBounds.maxY < maxY && markingBounds.minY > minY && markingBounds.maxY > minY)
	{
		correctPos++;
	}

	return correctPos >= 2;
}

UIShaderEditorBlock::Size UIShaderEditorBlock::GetPosition() const
{
	return Size(m_translationX, m_translationY);
}

void UIShaderEditorBlock::ChangeBlockName()
{
	//TODO ChangeBlockName - unused?
	//m_textEngine->GetData(0)->text = GetFirstOutputNode()->GetVisibleName();
}

int UIShaderEditorBlock::GetBlockID() const
{
	return m_blockID;
}

void UIShaderEditorBlock::UpdateVariable()
{
	CalculateBlockSize(1, 1);
	InitializeModelGeneric(m_D3D->GetDevice(), m_blockVertices);
	m_outlineObject->InitializeModelGeneric(m_D3D->GetDevice(), CalculateOutlineSize(m_blockVertices), false, true);
}

void UIShaderEditorBlock::CalculateBlockSize(int inCount, int outCount)
{
	//TODO Create function
	int inOutCount = inCount > outCount ? inCount : outCount;
	Size blockSize = blockSizeVector[inOutCount - 1];
	blockSize.x = 0.1f;

	if (m_functionName == "float")
	{
		inOutCount = 1;
		blockSize.x *= 1;
		blockSize.y = 0.11f;
		if (GetFirstOutputNode() && GetFirstOutputNode()->m_isVariable)
			blockSize.y += 0.05f;
	}
	else if (m_functionName == "float2")
	{
		inOutCount = 3;
		blockSize.x *= 2;
		blockSize.y = 0.11f;
		if (GetFirstOutputNode() && GetFirstOutputNode()->m_isVariable)
			blockSize.y += 0.05f;
	}
	else if (m_functionName == "float3")
	{
		inOutCount = 4;
		blockSize.x *= 3;
		blockSize.y = 0.4f;
		if (GetFirstOutputNode() && GetFirstOutputNode()->m_isVariable)
			blockSize.y += 0.05f;
	}
	else if (m_functionName == "float4")
	{
		inOutCount = 5;
		blockSize.x *= 3;
		blockSize.y = 0.4f;
		if (GetFirstOutputNode() && GetFirstOutputNode()->m_isVariable)
			blockSize.y += 0.05f;
	}
	else if (m_functionName == "texture")
	{
		inOutCount = 5;
		blockSize.x *= 3;
		blockSize.y = 0.4f;
		if (GetFirstOutputNode() && GetFirstOutputNode()->m_isVariable)
			blockSize.y += 0.05f;
	}
	else
	{
		blockSize.x = 0.02f;
		if (m_functionName.size() > 12)
			blockSize.x += m_functionName.size() * 0.017f;
		else
			blockSize.x += m_functionName.size() * 0.015f;
	}
	blockSize.x = max(blockSize.x, 0.125f);

	m_blockVertices.minX = -blockSize.x * 0.5f;
	m_blockVertices.maxX = blockSize.x * 0.5f;
	m_blockVertices.minY = -blockSize.y * 0.5f;
	m_blockVertices.maxY = blockSize.y * 0.5f;
}

UIBase::RectangleVertices UIShaderEditorBlock::CalculateOutlineSize(UIBase::RectangleVertices blockSize) const
{
	return RectangleVertices{ blockSize.minX - outlineMargin, blockSize.maxX + outlineMargin, blockSize.minY - outlineMargin, blockSize.maxY + outlineMargin };
}

bool UIShaderEditorBlock::InitializeInputNodes(int count)
{
	for (int i = 0; i < count; ++i)
	{
		UIShaderEditorInput* inputNode = new UIShaderEditorInput;
		inputNode->m_returnType = m_argumentTypes.at(i);
		if (!inputNode->Initialize(m_D3D, ModelClass::ShapeSize::RECTANGLE,
			m_blockVertices.minX + inOutMargin.x, m_blockVertices.minX + inOutMargin.x + inOutSize.x,
			m_blockVertices.maxY - inOutMargin.y, m_blockVertices.maxY - inOutMargin.y - inOutSize.y))
		{
			return false;
		}
		inputNode->Move(0.0f, -static_cast<float>(i) * paddingBetweenBlocks);		

		m_inputNodes.push_back(std::move(inputNode));
	}

	return true;
}

bool UIShaderEditorBlock::InitializeOutputNodes(int count)
{
	for (int i = 0; i < count; ++i)
	{
		UIShaderEditorOutput* outputNode = new UIShaderEditorOutput;
		outputNode->m_returnType = m_returnType;
		if (!outputNode->Initialize(m_D3D, ModelClass::ShapeSize::RECTANGLE,
			m_blockVertices.maxX - inOutMargin.x - inOutSize.x, m_blockVertices.maxX - inOutMargin.x,
			m_blockVertices.maxY - inOutMargin.y, m_blockVertices.maxY - inOutMargin.y - inOutSize.y))
		{
			return false;
		}
		outputNode->Move(0.0f, -(float)i * paddingBetweenBlocks);				

		m_outputNodes.push_back(std::move(outputNode));
	}
	if (m_outputNodes.size() == 1 && m_inputNodes.size() == 0)
	{
		GetFirstOutputNode()->Move(0.0f, 0.055f);
	}

	return true;
}

std::string UIShaderEditorBlock::ReturnEmptyForGivenType(std::string type) const
{
	if (type == "float")
		return "0.0f";
	else if (type == "float2")
		return "float2(0.0f, 0.0f)";
	else if (type == "float3")
		return "float3(0.0f, 0.0f, 0.0f)";
	else if (type == "float4")
		return "float4(0.0f, 0.0f, 0.0f, 0.0f)";

	return "";
}

UIShaderEditorInput* UIShaderEditorBlock::CheckIfMouseOnInputPin(MouseClass* mouse) const
{
	for (const auto& pin : m_inputNodes)
	{
		if (pin->MouseOnArea(mouse))
		{
			return pin;
		}
	}
	return nullptr;
}
