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

UIShaderEditorBlock::UIShaderEditorBlock(XMFLOAT2 startPosition, std::string functionName, std::string returnType, vector<std::string> argumentTypes)
{
	UIBase::UIBase();
	m_moveAfterInitializing = true;
	m_movemementAfterInitialization = startPosition;
	ChangeColor(blockColor);

	m_functionName = functionName;
	m_returnType = returnType;
	m_argumentTypes = argumentTypes;
}

bool UIShaderEditorBlock::MouseOnArea(MouseClass * mouse)
{
	bool result = false;
	float mouseX{ 0 };
	float mouseY{ 0 };
	POINT p = mouse->CurrentMouseLocation();

	//Calculate mouse X
	mouseX = (float)p.x / (float)m_D3D->GetWindowSize().x;
	mouseX = mouseX * 2.0f - 1.0f;
	if (mouseX > 1.0f)
		mouseX = 1.0f;
	else if (mouseX < -1.0f)
		mouseX = -1.0f;

	//Calculate mouse Y
	mouseY = (float)p.y / (float)m_D3D->GetWindowSize().y;
	mouseY = mouseY * 2.0f - 1.0f;
	if (mouseY > 1.0f)
		mouseY = 1.0f;
	else if (mouseY < -1.0f)
		mouseY = -1.0f;

	mouseY *= -1.0f;

	if (mouseX > (m_blockVertices.minX + m_translationX) && mouseX < (m_blockVertices.maxX + m_translationX) &&
		mouseY > (m_blockVertices.minY + m_translationY) && mouseY < (m_blockVertices.maxY + m_translationY) )
	{
		result = true;
	}

	return result;
}

bool UIShaderEditorBlock::Initialize(D3DClass * d3d, int inCount, int outCount)
{
	m_D3D = d3d;
	if (!BaseShaderClass::Initialize(d3d->GetDevice(), *d3d->GetHWND(), UI_SHADER_VS, UI_SHADER_PS, BaseShaderClass::vertexInputType(GetInputNames(), GetInputFormats())))
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
	m_textEngine->Initialize(d3d->GetDevice(), L"Fonts/font.spritefont");
	m_blockName = m_functionName;
	for (char& c : m_blockName)
		c = toupper(c);
	m_textEngine->WriteText(d3d->GetDeviceContext(), d3d->GetWindowSize().x, d3d->GetWindowSize().y, m_translationX, m_translationY, m_blockName, 1.0f, TextEngine::Align::CENTER);

	//Create outline
	m_outlineObject = new UIBase;
	if (!m_outlineObject->Initialize(d3d->GetDevice(), *d3d->GetHWND(), UI_SHADER_VS, UI_SHADER_PS, BaseShaderClass::vertexInputType(GetInputNames(), GetInputFormats())))
		return false;
	if (!m_outlineObject->InitializeModelGeneric(d3d->GetDevice(), CalculateOutlineSize(m_blockVertices), false, true))
		return false;
	m_outlineObject->ChangeColor(outlineColor);

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

bool UIShaderEditorBlock::IsDragging()
{
	return m_dragged;
}

bool UIShaderEditorBlock::IsPinDragging()
{
	return m_pinDragged;
}

UIShaderEditorOutput* UIShaderEditorBlock::DragPins(MouseClass * mouse)
{
	for (const auto& pin : m_outputNodes)
	{
		if (pin->IsDragging())
		{
			if (mouse->GetLMBPressed() == false)
			{
				pin->StopDragging();
				m_pinDragged = false;
			}
			return pin;
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
				return nullptr;
			}
			if (mouse->GetLMBPressed())
			{
				pin->StartDragging();
				m_pinDragged = true;
				return pin;
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
				return nullptr;
			}
		}
	}

	return nullptr;
}

bool UIShaderEditorBlock::Render(ID3D11DeviceContext * deviceContext)
{
	if (!m_blockInitialized)
		return true;

	XMMATRIX tmpMatrix;
	tmpMatrix *= 0;
	tmpMatrix.r[0] = XMVECTOR{ m_translationX, m_translationY, 0, 0 };

	if (m_focused && m_outlineObject)
	{
		//Render outline
		if (m_outlineObject && !m_outlineObject->Render(deviceContext, 0, tmpMatrix, tmpMatrix * 0, tmpMatrix * 0))
			return false;
	}
	tmpMatrix.r[0] = XMVECTOR{ m_translationX, m_translationY, 0, 0 };

	if (UIBase::Render(deviceContext, 0, tmpMatrix, tmpMatrix * 0, tmpMatrix * 0))
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
			if (m_functionName == "float")
			{
				m_textEngine->GetData(0)->SetPosition(m_translationX - 0.1f, m_translationY, m_D3D->GetWindowSize().x, m_D3D->GetWindowSize().y);
				ostringstream oss;
				oss << m_outputNodes[0]->m_value;
				m_textEngine->GetData(0)->text = "FLOAT:" + oss.str();
				m_textEngine->GetData(0)->scale = 0.75f;
			}
			else
			{
				m_textEngine->GetData(0)->SetPosition(m_translationX, m_translationY, m_D3D->GetWindowSize().x, m_D3D->GetWindowSize().y);
			}
			m_textEngine->RenderText(deviceContext, m_D3D->GetWindowSize().x, m_D3D->GetWindowSize().y);
		}

		return true;
	}
	else
		return false;
}

std::string UIShaderEditorBlock::GenerateShaderCode(bool skipTabulator)
{	
	std::string func{};
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
	else //Function call
	{
		func = { (skipTabulator ? "" : "\t") + m_returnType + " " + m_variableName + " = " + m_functionName + "(" };
		std::vector<std::string> args = {};

		for (int i = 0; i < m_inputNodes.size(); ++i)
		{
			const auto& node = m_inputNodes.at(i);
			if (node->m_connectedOutputNode != nullptr)
			{
				args.push_back(node->m_connectedOutputNode->m_variableName);
			}
			else
			{
				args.push_back(ReturnEmptyForGivenType(m_argumentTypes.at(i)));
			}
		}
		for (int i = 0; i < args.size(); ++i)
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

int UIShaderEditorBlock::GetInputCount()
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

std::string UIShaderEditorBlock::GetFunctionName()
{
	return m_functionName;
}

std::string UIShaderEditorBlock::GetReturnType()
{
	return m_returnType;
}

void UIShaderEditorBlock::CalculateBlockSize(int inCount, int outCount)
{
	int inOutCount = inCount > outCount ? inCount : outCount;
	Size blockSize = blockSizeVector[inOutCount - 1];
	m_blockVertices.minX = -blockSize.x * 0.5f;
	m_blockVertices.maxX = blockSize.x * 0.5f;
	m_blockVertices.minY = -blockSize.y * 0.5f;
	m_blockVertices.maxY = blockSize.y * 0.5f;
}

UIBase::RectangleVertices UIShaderEditorBlock::CalculateOutlineSize(UIBase::RectangleVertices blockSize)
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
		inputNode->Move(0.0f, -(float)i * paddingBetweenBlocks);

		m_inputNodes.push_back(inputNode);
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

		m_outputNodes.push_back(outputNode);
	}

	return true;
}

std::string UIShaderEditorBlock::ReturnEmptyForGivenType(std::string type)
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

UIShaderEditorInput* UIShaderEditorBlock::CheckIfMouseOnInputPin(MouseClass* mouse)
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
