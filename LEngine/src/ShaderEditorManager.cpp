#include "ShaderEditorManager.h"

ShaderEditorManager::ShaderEditorManager(D3DClass * d3d, MouseClass * mouse)
{
	m_D3D = d3d;
	m_mouse = mouse;
	m_pbrBlock = new UIShaderPBRBlock();
	m_pbrBlock->Initialize(d3d);
	LoadFunctionsFromDirectory();
}

void ShaderEditorManager::UpdateBlocks(bool mouseOnly)
{
	if (!mouseOnly)
	{
		if (!RenderBlocks(m_D3D->GetDeviceContext()))
			return;
	}

	//Check pins first - before moving block, interact with pins
	if (!UpdatePinsOfAllBlocks())
		return;

	if (!UpdatePBRBlock(mouseOnly))
		return;

	//If user is dragging one block - he won't interact with any other block until freed previous one
	for (const auto& block : m_blocks)
	{
		if (block->IsDragging())
		{
			std::pair<float, float> mouseMov = m_mouse->MouseFrameMovement();
			block->Move(mouseMov.first, mouseMov.second);
			if (m_mouse->GetLMBPressed() == false)
			{
				block->StopDragging();
			}
			return;
		}
	}

	//User can start dragging only one block at the time - dragging multiple blocks is not available
	for (const auto& block : m_blocks)
	{
		if (block->MouseOnArea(m_mouse) && m_mouse->GetLMBPressed())
		{
			ResetFocusOnAllBlocks();
			block->StartDragging();
			return;
		}
	}

	//No interaction with blocks or pins - check if pressed to create new block
	if (m_mouse->GetRMBPressed() && !WillRenderChoosingWindow())
	{
		m_mouse->SetRMBPressed(false);
		ShowFunctionChoosingWindow();
	}
	else if (m_mouse->GetLMBPressed() && !WillRenderChoosingWindow())
	{
		ResetFocusOnAllBlocks();
	}
}

bool ShaderEditorManager::UpdatePBRBlock(bool mouseOnly)
{
	for (const auto& block : m_blocks)
	{
		if (block->IsDragging())
			return true;
	}

	if (m_pbrBlock)
	{
		//if (!mouseOnly)
		//	m_pbrBlock->Render(m_D3D->GetDeviceContext());

		if (m_pbrBlock->IsDragging())
		{
			std::pair<float, float> mouseMov = m_mouse->MouseFrameMovement();
			m_pbrBlock->Move(mouseMov.first, mouseMov.second);
			if (m_mouse->GetLMBPressed() == false)
			{
				m_pbrBlock->StopDragging();
			}
			return false;
		}

		if (m_pbrBlock->MouseOnArea(m_mouse) && m_mouse->GetLMBPressed())
		{
			m_pbrBlock->StartDragging();
			return false;
		}
	}

	return true;
}

bool ShaderEditorManager::UpdatePinsOfAllBlocks()
{
	for (const auto& block : m_blocks)
	{
		if (block->IsDragging())
			return true;
	}

	if (m_pbrBlock && m_pbrBlock->IsDragging())
		return true;

	UIShaderEditorOutput* out = nullptr;
	UIShaderEditorBlock* currentBlock = nullptr;

	//If pin is being dragged - consume input
	for (const auto& block : m_blocks)
	{
		if (block->IsPinDragging())
		{
			out = block->DragPins(m_mouse);
			currentBlock = block;
		}
	}

	//If dropped on input pin - connect them
	bool canTryPBR = true;
	for (const auto& block : m_blocks)
	{
		if (UIShaderEditorInput* in = block->CheckIfMouseOnInputPin(m_mouse))
		{
			if (out && currentBlock && currentBlock->IsPinDragging() == false)
			{
				in->m_connectedOutputNode = out;
				in->ChangeColor(0.0f, 0.0f, 1.0f, 1.0f);
				DrawLine(in, out);
				canTryPBR = false;
				break;
			}
		}
	}
	//If dropped on PBR input pin - connect them
	if (canTryPBR && m_pbrBlock)
	{
		if (UIShaderEditorInput* in = m_pbrBlock->CheckIfMouseOnInputPin(m_mouse))
		{
			if (out && currentBlock && currentBlock->IsPinDragging() == false)
			{
				in->m_connectedOutputNode = out;
				in->ChangeColor(0.0f, 0.0f, 1.0f, 1.0f);
				DrawLine(in, out);
			}
			else if (m_mouse->GetRMBPressed())
			{
				in->m_connectedOutputNode = nullptr;
				return false;
			}
		}
	}

	if (out)
	{
		return false;
	}
	//If no pin is being dragged - check to start dragging
	for (const auto& block : m_blocks)
	{
		if (block->DragPins(m_mouse))
			return false;
	}
	return true;
}

void ShaderEditorManager::DrawLine(UIShaderEditorInput * in, UIShaderEditorOutput * out)
{
	for (int i = m_lines.size() - 1; i >= 0; --i)
	{
		if (m_lines.at(i)->GetInput() == in)
		{
			delete m_lines.at(i);
			m_lines.at(i) = nullptr;
			m_lines.erase(m_lines.begin()+i);
		}
	}

	UILine* line = new UILine;
	line->Initialize(m_D3D, out, in);
	out->m_toDeleteLine = false;
	m_lines.push_back(line);
}

void ShaderEditorManager::ResetFocusOnAllBlocks()
{
	for (const auto& block : m_blocks)
	{
		block->m_focused = false;
	}
}

void ShaderEditorManager::CreateChoosingWindowItemsArray()
{
	//ChoosingWindowItems = new char[GetFilenamesInDirectory("ShaderFunctions", false).size()];
	std::string toReturn{};

	char* test[] = {"TEST"};

	for (const auto& file : GetFilenamesInDirectory("ShaderFunctions", false))
	{
		
	}
}

std::string ShaderEditorManager::GenerateBlockCode(UIShaderEditorBlock * block)
{
	if (block->m_inputNodes.size() == 0)
	{
		if (block->GetFirstOutputNode())
		{
			return block->GenerateShaderCode();
		}
		else
			return "";
	}

	std::string toReturn{};
	for (const auto& in : block->m_inputNodes)
	{
		UIShaderEditorOutput* out = in->m_connectedOutputNode;
		for (const auto& connectedBlock : m_blocks)
		{
			for (const auto& otherOut : connectedBlock->m_outputNodes)
			{
				if (out == otherOut)
				{
					toReturn += { GenerateBlockCode(connectedBlock) };
					goto skipHere;
				}
			}
		}
	skipHere:
		{}
	}

	if (m_originalGeneratorBlock == block)
	{
		toReturn += block->GenerateShaderCode();
		toReturn += "\n\treturn " + block->m_variableName + ";";
	}
	else
		toReturn += block->GenerateShaderCode();

	return toReturn;
}

std::string ShaderEditorManager::GetFunctionDeclarations()
{
	std::string toReturn{};

	for (const auto& file : GetFilenamesInDirectory("ShaderFunctions"))
	{
		if (std::ifstream stream{ file, std::ios::binary })
		{
			std::string line{};
			getline(stream, line);
			std::string finLine{};
			for (const char& c : line)
			{
				if (c != '\n' && c != '\r')
					finLine += c;
			}
			finLine += ";\n";
			toReturn += finLine;
		}
	}
	return toReturn;
}

std::string ShaderEditorManager::GetFunctionDefinitions()
{
	std::string toReturn{};

	for (const auto& file : GetFilenamesInDirectory("ShaderFunctions"))
	{
		if (std::ifstream stream{ file, std::ios::binary })
		{
			while (!stream.eof())
			{
				std::string line{};
				getline(stream, line);
				line += "\n";
				toReturn += line;
			}
		}
	}

	return toReturn;
}

vector<std::string> ShaderEditorManager::GetFilenamesInDirectory(std::string dir, bool withDir)
{
	vector<string> names;
	string search_path{ dir + "/*.*" };
	WIN32_FIND_DATA fd;
	HANDLE hFind = ::FindFirstFile(search_path.c_str(), &fd);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			// read all (real) files in current folder
			// , delete '!' read other 2 default folder . and ..
			if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
				if (withDir)
					names.push_back(dir + "/" + fd.cFileName);
				else
					names.push_back(fd.cFileName);
			}
		} while (::FindNextFile(hFind, &fd));
		::FindClose(hFind);
	}

	return names;
}

void ShaderEditorManager::ShowFunctionChoosingWindow()
{
	m_choosingWindowPosX = m_mouse->CurrentMouseLocation().x;
	m_choosingWindowPosY = m_mouse->CurrentMouseLocation().y;
	m_mouse->GetMouseLocationScreenSpace(m_choosingWindowPosXScreenspace, m_choosingWindowPosYScreenspace);
	
	m_choosingWindow = true;
}

void ShaderEditorManager::LoadFunctionsFromDirectory()
{
	//std::vector<std::string> names = GetFilenamesInDirectory("ShaderFunctions", false);
	//
	//for (int i = 0; i < names.size(); ++i)
	//{
	//	 strcpy(m_choosingWindowItems[i], names.at(i).c_str());
	//}
}

void ShaderEditorManager::GenerateCodeToFile()
{
	std::string func{};
	for (int i = 0; i < m_blocks.size(); ++i)
	{
		const auto& block = m_blocks.at(i);
		if (block->GetInputCount() == 0) //Variable
		{
			char variableName = i + 65;
			block->m_variableName = variableName;
			block->SetOutputPinName(block->m_variableName);
		}
	}
	for (int i = 0; i < m_blocks.size(); ++i)
	{
		const auto& block = m_blocks.at(i);
		if (block->GetInputCount() > 0) //Function
		{
			std::string variableName{ "out_" };
			variableName += (char)(i + '0');
			block->m_variableName = variableName;
		}
	}
	for (int i = 0; i < m_pbrBlock->m_inputNodes.size(); ++i)
	{
		if (UIShaderEditorOutput* out = m_pbrBlock->m_inputNodes.at(i)->m_connectedOutputNode)
		{
			std::string funcName{ m_pbrBlock->m_inputTypes.at(i) + " Get" + m_pbrBlock->m_inputNames.at(i) + "()" };
			std::string funcBody{ "\n{\n" };

			for (const auto& block : m_blocks)
			{
				if (out == block->GetFirstOutputNode())
				{
					m_originalGeneratorBlock = block;
					funcBody += GenerateBlockCode(block);
					break;
				}
			}

			func += (funcName + funcBody);
			func += "\n}";
			func += "\n"; //New line and empty line
		}
		else
		{
			std::string funcName{ m_pbrBlock->m_inputTypes.at(i) + " Get" + m_pbrBlock->m_inputNames.at(i) + "()" };
			func += funcName;
			func += "\n{\n";

			std::string type = m_pbrBlock->m_inputTypes.at(i);
			if (type == "float")
				func += "\treturn 0.0f;";
			else if (type == "float2")
				func += "\treturn float2(0.0f, 0.0f);";
			else if (type == "float3")
				func += "\treturn float3(0.0f, 0.0f, 0.0f);";
			else if (type == "float4")
				func += "\treturn float4(0.0f, 0.0f, 0.0f, 0.0f);";

			func += "\n}";
			func += "\n"; //New line and empty line
		}
	}
	
	//std::ofstream dst("pbr_used.ps", std::ios::binary);
	std::ofstream dst("function.txt", std::ios::binary);

	if (std::ifstream src{ "ShaderEditor/base_declarations.txt", std::ios::binary })
	{
		dst << src.rdbuf(); //base_declarations
	}
	dst << GetFunctionDeclarations();
	if (std::ifstream src{ "ShaderEditor/base_body.txt", std::ios::binary })
	{
		dst << src.rdbuf(); //base_body
	}
	dst << GetFunctionDefinitions();
	if (std::ifstream src{ "ShaderEditor/generated_header.txt", std::ios::binary })
	{
		dst << src.rdbuf(); //generated_header
	}
	dst << func; //Get generated functions
}

bool ShaderEditorManager::WillRenderChoosingWindow()
{
	return m_choosingWindow;
}

int * ShaderEditorManager::GetChoosingWindowHandler()
{
	return &m_choosingWindowHandler;
}

void ShaderEditorManager::CreateBlock(std::string name)
{
	m_choosingWindow = false;
	std::string line{};
	std::string returnType{};
	vector<std::string> argumentTypes{""};
	std::string functionName{};
	bool filledReturnType = false;
	bool filledFunctionName = false;
	bool argumentWaitForComma = false;
	int argumentIndex = 0;

	for (char& c : name)
		c = tolower(c);

	if (ifstream in{ "ShaderFunctions/" + name + ".txt"})
	{
		getline(in, line);
		for (const auto& c : line)
		{
			if (!filledReturnType)
			{
				if (c != ' ')
					returnType += c;
				else
					filledReturnType = true;
			}
			else if (!filledFunctionName)
			{
				if (c != '(')
					functionName += c;
				else
					filledFunctionName = true;
			}
			else
			{
				if (c == ' ')
				{
					if (argumentTypes.at(argumentIndex) != "")
					{
						argumentWaitForComma = true;
					}
					continue;
				}
				else if (argumentWaitForComma)
				{
					if (c == ',')
					{
						argumentWaitForComma = false;
						argumentTypes.push_back({});
						argumentIndex++;
					}
					continue;
				}
				argumentTypes.at(argumentIndex) += c;
			}
		}
	}

	AddShaderBlock(new UIShaderEditorBlock({ {m_choosingWindowPosXScreenspace, m_choosingWindowPosYScreenspace }, functionName, returnType, argumentTypes}), argumentTypes.size(), 1);
}

float ShaderEditorManager::GetWindowPositionX()
{
	return m_choosingWindowPosX;
}

float ShaderEditorManager::GetWindowPositionY()
{
	return m_choosingWindowPosY;
}

void ShaderEditorManager::PressedOutsideOfChoosingWindow()
{
	m_choosingWindow = false;
}

void ShaderEditorManager::DeleteCurrentShaderBlock()
{
	for (int i = m_blocks.size() - 1; i > -1; i--)
	{
		auto& block = m_blocks.at(i);
		if (block->m_focused)
		{
			//TODO Potentially memory leak
			delete block;
			block = nullptr;
			m_blocks.erase(m_blocks.begin() + i);
		}
	}
}

void ShaderEditorManager::AddShaderBlock(UIShaderEditorBlock* block, int inCount, int outCount)
{
	block->Initialize(m_D3D, inCount, outCount);
	m_blocks.push_back(block);
}

void ShaderEditorManager::AddShaderBlock(UIShaderEditorBlock * block)
{
	m_blocks.push_back(block);
}

bool ShaderEditorManager::RenderBlocks(ID3D11DeviceContext* deviceContext)
{
	for (const auto& block : m_blocks)
	{
		if (!block->Render(deviceContext))
			return false;
	}

	if (m_pbrBlock)
	{
		if (!m_pbrBlock->Render(deviceContext))
			return false;
	}

	for (int i = 0; i < m_lines.size(); ++i)
	{		
		//Click on output pin - break connection
		if (m_lines.at(i)->GetOutput()->m_toDeleteLine)
		{
			m_lines.at(i)->GetOutput()->m_toDeleteLine = false;
			m_lines.at(i)->GetInput()->m_connectedOutputNode = nullptr;
			delete m_lines.at(i);
			m_lines.at(i) = nullptr;
			m_lines.erase(m_lines.begin() + i);
			continue;
		}
		//Clicked on input pin - break connection
		else if (!m_lines.at(i)->GetInput() || !m_lines.at(i)->GetInput()->m_connectedOutputNode)
		{
			delete m_lines.at(i);
			m_lines.at(i) = nullptr;
			m_lines.erase(m_lines.begin() + i);
			continue;
		}
		//Line is empty for some reason - remove from collection
		else if (m_lines.at(i) == nullptr)
		{
			m_lines.erase(m_lines.begin() + i);
			continue;
		}

		if (!m_lines.at(i)->Render(deviceContext))
			return false;
	}

	return true;
}