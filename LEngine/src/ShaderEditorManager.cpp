#include "ShaderEditorManager.h"

ShaderEditorManager::ShaderEditorManager(D3DClass * d3d, MouseClass * mouse)
{
	m_D3D = d3d;
	m_mouse = mouse;
	m_pbrBlock = new UIShaderPBRBlock();
	m_pbrBlock->Initialize(d3d);
	LoadFunctionsFromDirectory();
	CreateChoosingWindowItemsArray();

	m_blocks.reserve(64);
	m_markingArea = new UIBase;
	m_markingArea->Initialize(d3d->GetDevice(), *d3d->GetHWND(), L"uiline.vs", L"uiline.ps", BaseShaderClass::vertexInputType(m_markingArea->GetInputNames(), m_markingArea->GetInputFormats()));
	//m_markingArea->InitializeModelGeneric(d3d->GetDevice(), {0.2f, 0.5f, -0.5f, 0.5f}, false, true);
	m_markingArea->InitializeModelGeneric(m_D3D->GetDevice(), { -10000.0f, -10000.0f, -10000.0f, -10000.0f }, false, true);
	m_markingArea->ChangeColor(0.85f, 0.85f, 0.85f, 1.0f);

	LoadAllMaterialsToArray();
}

void ShaderEditorManager::UpdateBlocks(bool mouseOnly)
{
	if (!mouseOnly)
	{
		if (!RenderBlocks(m_D3D->GetDeviceContext()))
			return;
		
		if (m_mouse->GetMouseScroll() != 0.0f && !WillRenderChoosingWindow())
		{
			m_scale += (float)m_mouse->GetMouseScroll() * 0.1f;
			if (m_scale > 1.0f)
				m_scale = 1.0f;
			else if (m_scale < 0.1f)
				m_scale = 0.1f;

			for (const auto& line : m_lines)
				line->SetScale(m_scale);

			for (const auto& block : m_blocks)
			{
				block->SetScale(m_scale);
				for (const auto& in : block->m_inputNodes)
					in->SetScale(m_scale);
				for (const auto& out : block->m_outputNodes)
					out->SetScale(m_scale);
			}

			m_pbrBlock->SetScale(m_scale);
			for (const auto& in : m_pbrBlock->m_inputNodes)
				in->SetScale(m_scale);
		}
	}

	if (!m_alreadyMarkingArea && !m_pickingColorObject)
	{
		m_focusedPBR = m_pbrBlock->m_focused;
		//Check pins first - before moving block, interact with pins
		if (!UpdatePinsOfAllBlocks())
			return;

		m_draggingScreen = false;

		if (!UpdatePBRBlock(mouseOnly))
			return;

		//If user is dragging one block - he won't interact with any other block until freed previous one
		for (const auto& block : m_blocks)
		{
			if (block->IsDragging())
			{
				std::pair<float, float> mouseMov = m_mouse->MouseFrameMovement();
				block->Move(mouseMov.first / m_scale, mouseMov.second / m_scale);
				if (m_mouse->GetLMBPressed() == false)
				{
					block->StopDragging();
				}
				MoveMultipleBlocks(block, mouseMov);
				return;
			}
		}

		//User can start dragging only one block at the time - dragging multiple blocks is not available
		for (const auto& block : m_blocks)
		{
			if (block->MouseOnArea(m_mouse) && m_mouse->GetLMBPressed())
			{
				if (!m_mouseHoveredImGui)
				{
					ShaderEditorManager::ETryDragResult tmp = TryToResetFocusOnAllBlocks(block);
					if (tmp == ShaderEditorManager::ETryDragResult::DoNotCreate)
					{
						return;
					}
					block->StartDragging();
					m_focusedBlock = block;
				}
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
			TryToResetFocusOnAllBlocks();
		}
		//No pin is being dragged, nor any block - try to move screen
		if (m_mouse->GetMMBPressed())
		{
			m_draggingScreen = true;
			float moveX = -m_mouse->GetMouseMovementFrame().first / m_D3D->GetWindowSize().x / m_scale;
			float moveY = m_mouse->GetMouseMovementFrame().second / m_D3D->GetWindowSize().y / m_scale;
			for (const auto& block : m_blocks)
			{
				block->Move(moveX, moveY);
			}
			m_pbrBlock->Move(moveX, moveY);
			return;
		}
	}
	//No blocks interaction/screen movement - try to mark many elements (to further movement/deleting/copying)
	if (m_mouse->GetLMBPressed() && !m_mouseHoveredImGui) //Start marking area
	{
		if (!m_alreadyMarkingArea)
		{
			m_alreadyMarkingArea = true;
			std::pair<float, float> tmpMousePos = GetCurrentMousePosition();
			m_mouseDragStartX = tmpMousePos.first;
			m_mouseDragStartY = tmpMousePos.second;
		}
		else
		{
			float newWidth = 0.007f / m_scale;
			m_markingArea->InitializeModelGeneric(m_D3D->GetDevice(), { GetMarkingBounds() }, false, true, newWidth);
		}
	}
	else if (m_alreadyMarkingArea) //Stop marking area
	{
		TryToMarkManyBlocks(GetMarkingBounds());
		m_alreadyMarkingArea = false;
		m_markingArea->InitializeModelGeneric(m_D3D->GetDevice(), { -10000.0f, -10000.0f, -10000.0f, -10000.0f }, false, true);
		return;
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
			//m_pbrBlock->Move(mouseMov.first / m_scale, mouseMov.second / m_scale);
			MoveMultipleBlocks(nullptr, mouseMov);
			if (m_mouse->GetLMBPressed() == false)
			{
				m_pbrBlock->StopDragging();
			}
			return false;
		}

		if (m_pbrBlock->MouseOnArea(m_mouse) && m_mouse->GetLMBPressed())
		{
			if (!m_mouseHoveredImGui)
			{
				m_pbrBlock->m_focused = true;
				m_focusedBlock = nullptr;
			}
			m_pbrBlock->StartDragging();
			return false;
		}
	}

	return true;
}

bool ShaderEditorManager::UpdatePinsOfAllBlocks()
{
	if (m_draggingScreen)
		return true;

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
			//if (block->DragPins(m_mouse) == UIShaderEditorBlock::EDragPinBehaviour::Break)
			//{
			//	m_refreshModel = true;
			//}
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
				if (CheckConnectionRules(in, out))
				{
					in->m_connectedOutputNode = out;
					in->ChangeColor(1.0f, 1.0f, 1.0f, 1.0f);
					out->ChangeColor(1.0f, 1.0f, 1.0f, 1.0f);
					DrawLine(in, out);
					m_refreshModel = true;
				}
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
				if (CheckConnectionRules(in, out))
				{
					in->m_connectedOutputNode = out;
					in->ChangeColor(1.0f, 1.0f, 1.0f, 1.0f);
					out->ChangeColor(1.0f, 1.0f, 1.0f, 1.0f);
					DrawLine(in, out);
					m_refreshModel = true;
				}
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
	line->Initialize(m_D3D, out, in, m_scale);
	out->m_toDeleteLine = false;
	m_lines.push_back(line);
}

void ShaderEditorManager::ResetFocusOnAllBlocks()
{
	for (const auto& block : m_blocks)
	{
		block->m_focused = false;
	}
	m_pbrBlock->m_focused = false;
	m_focusedBlock = nullptr;
}

ShaderEditorManager::ETryDragResult ShaderEditorManager::TryToResetFocusOnAllBlocks()
{
	return TryToResetFocusOnAllBlocks(nullptr);
}

ShaderEditorManager::ETryDragResult ShaderEditorManager::TryToResetFocusOnAllBlocks(UIShaderEditorBlock * const other)
{
	if (!m_mouseHoveredImGui)
	{
		bool canReset = true;
		bool containsOther = false;
		int count = 0;
		for (const auto& block : m_blocks)
		{
			if (block->m_focused)
			{
				count++;
				if (block->MouseOnArea(m_mouse))
					canReset = false;

				if (block == other)
					containsOther = true;
			}
		}
		if (!containsOther && other && count > 1)
		{
			//if (!canReset)
				ResetFocusOnAllBlocks();
			return ShaderEditorManager::ETryDragResult::DoNotCreate;
		}
		if (canReset || count <= 1)
		{
			ResetFocusOnAllBlocks();
			return ShaderEditorManager::ETryDragResult::Succeed;
		}
	}

	return ShaderEditorManager::ETryDragResult::Failed;
}

void ShaderEditorManager::CreateChoosingWindowItemsArray()
{
	for (const auto& file : GetFilenamesInDirectory("ShaderFunctions", false))
	{
		std::string tmp{};
		for (const auto& c : file)
		{
			if (c == '.')
				break;
			tmp += ::toupper(c);
		}
		ChoosingWindowItems.push_back(new char());
		strcpy(const_cast<char*>(ChoosingWindowItems.at(ChoosingWindowItems.size() - 1)), tmp.c_str());
	}
	ChoosingWindowItemsOriginal._Construct(ChoosingWindowItems.begin(), ChoosingWindowItems.end());

	std::sort(ChoosingWindowItems.begin(), ChoosingWindowItems.end());
	std::sort(ChoosingWindowItemsOriginal.begin(), ChoosingWindowItemsOriginal.end());
}

bool ShaderEditorManager::CheckConnectionRules(UIShaderEditorInput * in, UIShaderEditorOutput * out)
{
	if (in->m_returnType == out->m_returnType)
		return true;
	if (out->m_returnType == "float")
		return true;
	if (out->m_returnType == "float3" && in->m_returnType == "float4")
		return true;
	if (out->m_returnType == "float4" && in->m_returnType == "float3")
		return true;

	return false;
}

bool ShaderEditorManager::TryCreateScalarBlocks(std::string name)
{
	m_blockIDCounter++;
	if (name == "float1")
	{
		AddShaderBlock(new UIShaderEditorBlock({ { m_choosingWindowPosXScreenspace, m_choosingWindowPosYScreenspace }, "float", "float",{ "" }, m_blockIDCounter }), 0, 1);
		m_blocks.at(m_blocks.size() - 1)->m_fileName = name;
		return true;
	}
	else if (name == "float2")
	{
		AddShaderBlock(new UIShaderEditorBlock({ { m_choosingWindowPosXScreenspace, m_choosingWindowPosYScreenspace }, "float2", "float2",{ "" }, m_blockIDCounter }), 0, 1);
		m_blocks.at(m_blocks.size() - 1)->m_fileName = name;
		return true;
	}
	else if (name == "float3")
	{
		AddShaderBlock(new UIShaderEditorBlock({ { m_choosingWindowPosXScreenspace, m_choosingWindowPosYScreenspace }, "float3", "float3",{ "" }, m_blockIDCounter }), 0, 1);
		m_blocks.at(m_blocks.size() - 1)->m_fileName = name;
		return true;
	}
	else if (name == "float4")
	{
		AddShaderBlock(new UIShaderEditorBlock({ { m_choosingWindowPosXScreenspace, m_choosingWindowPosYScreenspace }, "float4", "float4",{ "" }, m_blockIDCounter }), 0, 1);
		m_blocks.at(m_blocks.size() - 1)->m_fileName = name;
		return true;
	}
	else if (name == "texture")
	{
		AddShaderBlock(new UIShaderEditorBlock({ { m_choosingWindowPosXScreenspace, m_choosingWindowPosYScreenspace }, "texture", "float4",{ "" }, m_blockIDCounter }), 0, 1);
		m_blocks.at(m_blocks.size() - 1)->m_fileName = name;
		return true;
	}

	return false;
}

std::string ShaderEditorManager::GenerateBlockCode(UIShaderEditorBlock * block)
{
	if (block->m_inputNodes.size() == 0)
	{
		if (block->GetFirstOutputNode())
		{
			std::string tmp = block->GenerateShaderCode();			
			for (const auto& elem : m_usedVariableNamesInGenerator)
			{
				if (elem == tmp)
				{
					return "";
				}
			}
			m_usedVariableNamesInGenerator.push_back(tmp);
			if (tmp == "\t;\n")
				tmp = "";
			return tmp;
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
		//toReturn += "\n\treturn " + block->m_variableName + ";";
		toReturn += ConvertReturnType(block->m_variableName, block->GetReturnType(), m_originalRequiredType);
	}
	else
		toReturn += block->GenerateShaderCode();

	return toReturn;
}

std::string ShaderEditorManager::GetTextureDeclarations()
{
	int count = 0;
	m_generatedTextureAdresses.clear();
	for (const auto& block : m_blocks) 
	{
		if (block->GetFunctionName() == "texture")
		{
			count++;
			m_generatedTextureAdresses.push_back(block->m_outputNodes[0]->m_texturePath);
		}
	}

	if (count > 0)
	{
		int index = 0;
		std::string toReturn{};
		for (const auto& block : m_blocks)
		{
			if (block->GetFunctionName() == "texture")
			{
				ostringstream ssBegin;
				ostringstream ssEnd;
				ssBegin << index;
				ssEnd << (index + 7);
				toReturn += "Texture2D additionalTexture_" + ssBegin.str() + " : register(t" + ssEnd.str() + ");\n";
				toReturn += "static float4 " + block->m_outputNodes[0]->m_variableName + ";\n";
				index++;
			}
		}
		return toReturn;
	}

	return "";
}

std::string ShaderEditorManager::GetTextureDefinitions()
{
	std::string toReturn{};

	int index = 0;
	for (const auto& block : m_blocks)
	{
		if (block->GetFunctionName() == "texture")
		{
			ostringstream ss;
			ss << index;
			index++;
			toReturn += "\t" + block->m_outputNodes[0]->m_variableName + " = additionalTexture_" + ss.str() + ".Sample(SampleType, input.tex);\n";
		}
	}

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
			if (line == "")
			{
				continue;
			}
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

std::string ShaderEditorManager::ConvertReturnType(std::string outName, std::string typeIn, std::string typeOut)
{
	if (typeIn == typeOut)
	{
		return "\n\treturn " + outName + ";";
	}

	if (typeIn == "float")
	{
		if (typeOut == "float2")
			return "\n\treturn float2(" + outName + ", " + outName + ");";
		if (typeOut == "float3")
			return "\n\treturn float3(" + outName + ", " + outName + ", " + outName + ");";
		if (typeOut == "float4")
			return "\n\treturn float4(" + outName + ", " + outName + ", " + outName + ", " + outName + ");";
	}
	else if (typeIn == "float3")
	{
		if (typeOut == "float4")
			return "\n\treturn float3(" + outName + ".x, " + outName + ".y, " + outName + ".z, 1.0f);";
	}
	else if (typeIn == "float4")
	{
		if (typeOut == "float3")
			return "\n\treturn float4(" + outName + ".x, " + outName + ".y, " + outName + ".z);";
	}

	return "ERROR";
}

std::vector<std::string> ShaderEditorManager::GetUsedTextures()
{
	return m_usedTextures;
}

void ShaderEditorManager::ShowFunctionChoosingWindow()
{
	m_choosingWindowPosX = m_mouse->CurrentMouseLocation().x;
	m_choosingWindowPosY = m_mouse->CurrentMouseLocation().y;
	m_mouse->GetMouseLocationScreenSpace(m_choosingWindowPosXScreenspace, m_choosingWindowPosYScreenspace);
	
	m_choosingWindow = true;
	*m_focusOnChoosingWindowsShader = false;
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

void ShaderEditorManager::GeneratePBRClassCode(std::string filename)
{
	GetTextureDeclarations();
	m_usedTextures.clear();
	for (const auto& name : m_generatedTextureAdresses)
	{
		m_usedTextures.push_back(name);
	}
	return;

	if (filename == "")
	{
		filename = "src/ShaderPBRGenerated.cpp";
	}
	else
	{
		filename = "GeneratedCpp/" + filename + ".cpp";
	}
	std::ofstream dst(filename, std::ios::binary);
	if (std::ifstream src{ "ShaderEditor/shader_pbr_generated_base.txt", std::ios::binary })
	{
		dst << src.rdbuf(); //base_textures_header
	}
	dst << "void ShaderPBRGenerated::LoadGeneratedTextures(ID3D11Device *device)\n";
	dst << "{\n";
	dst << "\tID3D11Resource* resource{nullptr};\n\n";

	for (int i = 0; i < m_generatedTextureAdresses.size(); ++i)
	{
		dst << "\tm_additionalMapViews.push_back(nullptr);\n";
	}
	for (int i = 0; i < m_generatedTextureAdresses.size(); ++i)
	{
		ostringstream ss;
		ss << i;
		dst << "\tLoadTexture(device, L\"" + m_generatedTextureAdresses.at(i) + "\", resource, m_additionalMapViews.at(" + ss.str() + "), true);\n";
	}
	dst << "}";
	dst.close();
}

void ShaderEditorManager::GenerateVariableNames()
{
	for (int i = 0; i < m_blocks.size(); ++i)
	{
		const auto& block = m_blocks.at(i);
		if (block->GetInputCount() == 0) //Variable
		{
			if (block->GetFunctionName() == "texture") //Texture
			{
				ostringstream ss;
				ss << i;
				block->m_variableName = "tex_" + ss.str();
				block->SetOutputPinName(block->m_variableName);
			}
			else
			{
				char variableName = i + 65;
				block->m_variableName = variableName;
				block->SetOutputPinName(block->m_variableName);
			}
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
			block->m_outputNodes[0]->m_variableName = variableName;
		}
	}
}

bool ShaderEditorManager::SaveMaterial(std::string filename)
{
	if (filename == "")
	{
		filename = GenerateMaterialName();
	}
	GenerateVariableNames();
	{
		std::ofstream output("Materials/" + filename + ".material", std::ios::binary);

		int count = m_blocks.size();
		std::vector<std::string> functions = GetFilenamesInDirectory("ShaderFunctions", false);
		output << count << "\n";

		for (const auto& block : m_blocks)
		{
			output << "BLOCK" << "\n";
			float x = block->GetPosition().x;
			float y = block->GetPosition().y;
			std::string name = block->m_fileName;
			
			output << x << "\n";
			output << y << "\n";
			output << name << "\n";
			output << SaveBlockValueMaterial(block);
			if (block->m_inputNodes.size() == 0 && block->GetFirstOutputNode())
			{
				output << block->GetFirstOutputNode()->GetVisibleName() << "\n";
			}
		}

		output << m_pbrBlock->GetPosition().x << "\n";
		output << m_pbrBlock->GetPosition().y << "\n";

		output.clear();
		output.close();
	}
	//
	{
		std::ofstream output("Materials/" + filename + ".materialpins", std::ios::binary);

		for (const auto& block : m_blocks)
		{
			for (const auto& out : block->m_outputNodes)
				output << out->m_variableName << "\n";
		}

		for (const auto& block : m_blocks)
		{
			for (const auto& in : block->m_inputNodes)
			{
				if (in->m_connectedOutputNode)
					output << in->m_connectedOutputNode->m_variableName << "\n";
				else
					output << "test" << "\n";
			}
		}

		for (const auto& in : m_pbrBlock->m_inputNodes)
		{
			if (in->m_connectedOutputNode)
				output << in->m_connectedOutputNode->m_variableName << "\n";
			else
				output << "test" << "\n";
		}

		output.clear();
		output.close();
	}

	GenerateCodeToFile(filename);
	GeneratePBRClassCode(filename);
	LoadAllMaterialsToArray(); //Generate array of materials again to show all materials on GUI
	return true;
}

bool ShaderEditorManager::IsWorkingOnSavedMaterial()
{
	return m_currentMaterialName.size() > 0;
}

bool ShaderEditorManager::LoadMaterial(std::string filename)
{
	if (filename == "")
	{
		filename = GenerateMaterialName();
	}
	m_currentMaterialName = filename;
	m_materialToSaveName = m_currentMaterialName;
	DestroyEditor();
	{
		std::ifstream input("Materials/" + filename + ".material", std::ios::binary);
		if (input.fail())
			return false;

		std::string line;
		int numOfElements;

		getline(input, line);
		numOfElements = ::atof(line.c_str());

		float x;
		float y;
		std::string name;

		for (int i = 0; i < numOfElements; ++i)
		{
			while (line != "BLOCK")
			{
				getline(input, line);
			}
			getline(input, line);
			x = ::atof(line.c_str());

			getline(input, line);
			y = ::atof(line.c_str());

			getline(input, line);
			name = line;

			CreateBlock(name);
			auto block = m_blocks.at(m_blocks.size() - 1);
			block->Move(x, y);

#pragma region Load Value
			//Load value of block (if scalar/texture)
			if (block->m_fileName == "float1" || block->m_fileName == "float2" || block->m_fileName == "float3" ||
				block->m_fileName == "float4" | block->m_fileName == "texture")
			{
				if (block->m_fileName == "float1")
				{
					getline(input, line);
					block->GetFirstOutputNode()->m_value = ::atof(line.c_str());
				}
				else if (block->m_fileName == "float2")
				{
					for (int i = 0; i < 2; ++i)
					{
						getline(input, line);
						block->GetFirstOutputNode()->m_valueTwo[i] = ::atof(line.c_str());
					}
				}
				else if (block->m_fileName == "float3")
				{
					for (int i = 0; i < 3; ++i)
					{
						getline(input, line);
						block->GetFirstOutputNode()->m_valueThree[i] = ::atof(line.c_str());
					}
				}
				else if (block->m_fileName == "float4")
				{
					for (int i = 0; i < 4; ++i)
					{
						getline(input, line);
						block->GetFirstOutputNode()->m_valueFour[i] = ::atof(line.c_str());
					}
				}
				else if (block->m_fileName == "texture")
				{
					getline(input, line);
					std::wstring wLine = std::wstring(line.begin(), line.end());
					const wchar_t* path = wLine.c_str();
					BaseShaderClass::LoadTexture(m_D3D->GetDevice(), path, block->GetFirstOutputNode()->m_connectedTexture, block->GetFirstOutputNode()->m_connectedTextureView);
					block->GetFirstOutputNode()->m_texturePath = line;
				}

				getline(input, line);
				block->GetFirstOutputNode()->m_visibleName = line;
				block->GetFirstOutputNode()->SaveVisibleName();
				block->ChangeBlockName();
			}
#pragma endregion
		}

		getline(input, line);
		x = ::atof(line.c_str());

		getline(input, line);
		y = ::atof(line.c_str());

		m_pbrBlock->ResetPosition();
		m_pbrBlock->Move(x, y);

		input.clear();
		input.close();
	}
	//
	{
		std::ifstream input("Materials/" + filename + ".materialpins", std::ios::binary);
		if (input.fail())
			return false;

		std::string line;

		for (const auto& block : m_blocks)
		{
			for (const auto& out : block->m_outputNodes)
			{
				getline(input, line);
				out->m_variableName = line;
			}
		}
		for (const auto& block : m_blocks)
		{
			for (const auto& in : block->m_inputNodes)
			{
				getline(input, line);
				in->m_connectedOutputNode = FindOutputNode(line);
				if (in->m_connectedOutputNode)
				{
					DrawLine(in, in->m_connectedOutputNode);
				}
			}
		}

		for (const auto& in : m_pbrBlock->m_inputNodes)
		{
			getline(input, line);
			in->m_connectedOutputNode = FindOutputNode(line);
			if (in->m_connectedOutputNode)
			{
				DrawLine(in, in->m_connectedOutputNode);
			}
		}

		input.clear();
		input.close();
	}

	LoadMaterialInputs();
	return true;
}

bool ShaderEditorManager::LoadMaterial(int index)
{
	return LoadMaterial(m_materialNames.at(index));
}

std::vector<std::string>& ShaderEditorManager::GetAllMaterialNames()
{
	return m_materialNames;
}

std::vector<MaterialPrefab>& ShaderEditorManager::GetAllMaterials()
{
	return m_materials;
}

std::vector<UIShaderEditorBlock*>& ShaderEditorManager::GetMaterialInputs()
{
	return m_materialInputs;
}

std::pair<float, float> ShaderEditorManager::GetCurrentMousePosition()
{
	float mouseX{ 0 };
	float mouseY{ 0 };
	POINT p = m_mouse->CurrentMouseLocation();

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

	return{ mouseX, mouseY };
}

UIBase::RectangleVertices ShaderEditorManager::GetMarkingBounds()
{
	std::pair<float, float> tmpMousePos = GetCurrentMousePosition();
	float currentMouseX = tmpMousePos.first;
	float currentMouseY = tmpMousePos.second;

	float minX = (m_mouseDragStartX < currentMouseX) ? m_mouseDragStartX : currentMouseX;
	float maxX = (m_mouseDragStartX > currentMouseX) ? m_mouseDragStartX : currentMouseX;
	float minY = (m_mouseDragStartY < currentMouseY) ? m_mouseDragStartY : currentMouseY;
	float maxY = (m_mouseDragStartY > currentMouseY) ? m_mouseDragStartY : currentMouseY;

	return{ minX / m_scale, maxX / m_scale, minY / m_scale, maxY / m_scale };
}

void ShaderEditorManager::TryToMarkManyBlocks(float minX, float maxX, float minY, float maxY)
{
	TryToMarkManyBlocks({ minX, maxX, minY, maxY });
}

void ShaderEditorManager::TryToMarkManyBlocks(UIBase::RectangleVertices bounds)
{
	for (const auto& block : m_blocks)
	{
		if (block->TryToMarkBlock(bounds))
		{
			block->m_focused = true;
		}
	}
	if (m_pbrBlock->TryToMarkBlock(bounds))
		m_pbrBlock->m_focused = true;
}

void ShaderEditorManager::MoveMultipleBlocks(UIShaderEditorBlock * currentBlock, std::pair<float, float> mouseMov)
{
	//Move all focused blocks (not only the dragged one)
	for (const auto& otherBlock : m_blocks)
	{
		if (otherBlock->m_focused && currentBlock != otherBlock)
		{
			otherBlock->Move(mouseMov.first / m_scale, mouseMov.second / m_scale);
		}
	}
	if (m_pbrBlock->m_focused || m_pbrBlock->IsDragging())
		m_pbrBlock->Move(mouseMov.first / m_scale, mouseMov.second / m_scale);
}

UIShaderEditorOutput * ShaderEditorManager::FindOutputNode(std::string name)
{
	for (const auto& block : m_blocks)
	{
		for (const auto& out : block->m_outputNodes)
		{
			if (out->m_variableName == name)
			{
				return out;
			}
		}
	}
	return nullptr;
}

std::string ShaderEditorManager::SaveBlockValueMaterial(UIShaderEditorBlock* block)
{
	if (block->m_fileName == "float1")
	{
		stringstream ss;
		ss << block->GetFirstOutputNode()->m_value;
		return ss.str() + "\n";
	}
	else if (block->m_fileName == "float2")
	{
		std::string toReturn;
		for (int i = 0; i < 2; ++i)
		{
			stringstream ss;
			ss << block->GetFirstOutputNode()->m_valueTwo[i];
			toReturn += ss.str() + "\n";
		}
		return toReturn;
	}
	else if (block->m_fileName == "float3")
	{
		std::string toReturn;
		for (int i = 0; i < 3; ++i)
		{
			stringstream ss;
			ss << block->GetFirstOutputNode()->m_valueThree[i];
			toReturn += ss.str() + "\n";
		}
		return toReturn;
	}
	else if (block->m_fileName == "float4")
	{
		std::string toReturn;
		for (int i = 0; i < 4; ++i)
		{
			stringstream ss;
			ss << block->GetFirstOutputNode()->m_valueFour[i];
			toReturn += ss.str() + "\n";
		}
		return toReturn;
	}
	else if (block->m_fileName == "texture")
	{
		return block->GetFirstOutputNode()->m_texturePath + "\n";
	}

	return "";
}

void ShaderEditorManager::DestroyEditor()
{
	DestroyVector(m_lines);
	DestroyVector(m_blocks);
	DestroyVector(m_materialInputs);
}

std::string ShaderEditorManager::GenerateMaterialName()
{
	std::string toReturn;
	for (int i = 0; i < std::strlen(m_materialToSaveName.data()); ++i)
		toReturn += ::tolower(m_materialToSaveName.data()[i]);
	return toReturn;
}

void ShaderEditorManager::LoadAllMaterialsToArray()
{
	m_materials.clear();
	m_materialNames.clear();

	std::vector<std::string> allFiles = GetFilenamesInDirectory("Materials", false);
	for (auto& file : allFiles)
	{
		size_t posNormal = file.find(".material");
		if (posNormal != std::string::npos)
		{
			size_t posPins = file.find(".materialpins");
			if (posPins != std::string::npos)
			{
				file.erase(file.begin() + posNormal, file.end());
				m_materialNames.push_back(file);

				m_materials.push_back({file});
			}
		}
	}

	std::sort(m_materialNames.begin(), m_materialNames.end());
}

void ShaderEditorManager::LoadMaterialInputs()
{
	m_materialInputs.clear();
	for (const auto& block : m_blocks)
	{
		if (block->m_outputNodes.size() == 1 && block->m_inputNodes.size() == 0)
		{
			m_materialInputs.push_back(block);
		}
	}
}

void ShaderEditorManager::GenerateCodeToFile(std::string filename)
{
	std::string func{};

	GenerateVariableNames();

	for (int i = 0; i < m_pbrBlock->m_inputNodes.size(); ++i)
	{
		if (UIShaderEditorOutput* out = m_pbrBlock->m_inputNodes.at(i)->m_connectedOutputNode)
		{
			std::string funcName{ m_pbrBlock->m_inputTypes.at(i) + " Get" + m_pbrBlock->m_inputNames.at(i) + "()" };
			std::string funcBody{ "\n{\n" };

			for (const auto& block : m_blocks)
			{
				m_usedVariableNamesInGenerator.empty();
				if (out == block->GetFirstOutputNode())
				{
					m_originalGeneratorBlock = block;
					m_originalRequiredType = m_pbrBlock->m_inputNodes.at(i)->m_returnType;
					funcBody += GenerateBlockCode(block);
					break;
				}
			}

			func += (funcName + funcBody);
			if (funcBody == "\n{\n" && m_originalGeneratorBlock->GetFunctionName() == "texture")
			{
				func += "\treturn " + m_originalGeneratorBlock->m_variableName + ";";
			}
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
	
	//std::ofstream dst("function.txt", std::ios::binary);
	GeneratePBRClassCode(filename);
	if (filename == "")
	{
		filename = "pbr_used.ps";
	}
	else
	{
		filename = "GeneratedShaders/" + filename + ".ps";
	}
	std::ofstream dst(filename, std::ios::binary);

	if (std::ifstream src{ "ShaderEditor/base_textures_header.txt", std::ios::binary })
	{
		dst << src.rdbuf(); //base_textures_header
	}
	dst << GetTextureDeclarations(); //Get Texture2D ... declaration of variable
	if (std::ifstream src{ "ShaderEditor/base_declarations.txt", std::ios::binary })
	{
		dst << src.rdbuf(); //base_declarations
	}
	dst << GetFunctionDeclarations();
	if (std::ifstream src{ "ShaderEditor/base_body_start.txt", std::ios::binary })
	{
		dst << src.rdbuf(); //base_body_start
	}
	dst << GetTextureDefinitions();
	if (std::ifstream src{ "ShaderEditor/base_body_continue.txt", std::ios::binary })
	{
		dst << src.rdbuf(); //base_body_continue
	}
	dst << GetFunctionDefinitions();
	if (std::ifstream src{ "ShaderEditor/generated_header.txt", std::ios::binary })
	{
		dst << src.rdbuf(); //generated_header
	}
	dst << func; //Get generated functions
	dst.close();
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

	m_choosingWindowSearch = "";

	if (TryCreateScalarBlocks(name))
	{ }
	else if (ifstream in{ "ShaderFunctions/" + name + ".txt"})
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
		m_blockIDCounter++;
		AddShaderBlock(new UIShaderEditorBlock({ { m_choosingWindowPosXScreenspace, m_choosingWindowPosYScreenspace }, functionName, returnType, argumentTypes, m_blockIDCounter }), argumentTypes.size(), 1);
		m_blocks.at(m_blocks.size() - 1)->m_fileName = name;
	}
	LoadMaterialInputs();
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

void ShaderEditorManager::SearchThroughChoosingWindow()
{
	ChoosingWindowItems._Construct(ChoosingWindowItemsOriginal.begin(), ChoosingWindowItemsOriginal.end());

	for (int i = ChoosingWindowItems.size() - 1; i >= 0; i--)
	{
		std::string str{ ChoosingWindowItems.at(i) };
		std::string mainStr{};

		for (int i = 0; i < std::strlen(m_choosingWindowSearch.data()); ++i)
			mainStr += ::toupper(m_choosingWindowSearch.data()[i]);

		std::size_t found = str.find(mainStr);

		if (found == std::string::npos)
		{
			ChoosingWindowItems.erase(ChoosingWindowItems.begin() + i);
		}
	}

	std::sort(ChoosingWindowItems.begin(), ChoosingWindowItems.end(),
		[](const std::string& a, const std::string& b) {return a < b; });
}

void ShaderEditorManager::DeleteCurrentShaderBlock()
{
	for (int i = m_blocks.size() - 1; i > -1; i--)
	{
		auto& block = m_blocks.at(i);
		if (block->m_focused)
		{
			//TODO Potentially memory leak
			for (const auto& line : m_lines)
			{
				for (const auto& pin : block->m_inputNodes)
				{
					if (line->GetInput() == pin)
					{
						line->GetOutput()->m_toDeleteLine = true;
					}
				}
			}

			for (auto& pin : block->m_outputNodes)
			{
				for (const auto& otherBlock : m_blocks)
				{
					for (const auto& otherPin : otherBlock->m_inputNodes)
					{
						if (pin && otherPin && pin == otherPin->m_connectedOutputNode)
						{
							otherPin->m_connectedOutputNode = nullptr;
						}
					}
				}
				if (pin)
				{
					delete pin;
					pin = nullptr;
				}
			}
			for (auto& pin : block->m_inputNodes)
			{
				if (pin)
				{
					pin->m_connectedOutputNode = nullptr;
					delete pin;
					pin = nullptr;
				}
			}
			if (block)
			{
				delete block;
				block = nullptr;
				m_blocks.erase(m_blocks.begin() + i);
			}
		}
	}
	//m_focusedBlock = nullptr;
}

void ShaderEditorManager::ResetMouseHoveredOnImGui()
{
	m_mouseHoveredImGui = false;
}

void ShaderEditorManager::UpdateMouseHoveredOnImGui(bool hovered)
{
	if (hovered)
		m_mouseHoveredImGui = true;
}

void ShaderEditorManager::SetRefToClickedOutside(bool* clickedOutside)
{
	m_focusOnChoosingWindowsShader = clickedOutside;
}

void ShaderEditorManager::CopyBlocks()
{
	m_copiedBlocks.clear();
	for (const auto& block : m_blocks)
	{
		if (block->m_focused)
		{
			m_copiedBlocks.push_back(block);
		}
	}
}

void ShaderEditorManager::PasteBlocks()
{
	ResetFocusOnAllBlocks();
	for (const auto& block : m_copiedBlocks)
	{
		CreateBlock(block->m_fileName);
		m_blocks.at(m_blocks.size() - 1)->Move(block->GetPosition().x + 0.25f * m_scale, block->GetPosition().y + 0.1f * m_scale);
		m_blocks.at(m_blocks.size() - 1)->m_focused = true;
	}
}

void ShaderEditorManager::SetPickingColorElement(UIShaderEditorOutput* out)
{
	m_pickingColorObject = out;
}

UIShaderEditorOutput* ShaderEditorManager::GetPickingColorElement()
{
	return m_pickingColorObject;
}

void ShaderEditorManager::AddShaderBlock(UIShaderEditorBlock* block, int inCount, int outCount)
{
	if (block->Initialize(m_D3D, inCount, outCount))
	{
		block->SetScale(m_scale);
		for (const auto& in : block->m_inputNodes)
			in->SetScale(m_scale);
		for (const auto& out : block->m_outputNodes)
			out->SetScale(m_scale);

		m_blocks.push_back(block);
	}
}

void ShaderEditorManager::AddShaderBlock(UIShaderEditorBlock * block)
{
	m_blocks.push_back(block);
}

bool ShaderEditorManager::RenderBlocks(ID3D11DeviceContext* deviceContext)
{
	for (const auto& block : m_blocks)
	{
		if (block && !block->Render(deviceContext))
			return false;
	}

	if (m_pbrBlock)
	{
		if (m_pbrBlock && !m_pbrBlock->Render(deviceContext))
			return false;
	}

	for (int i = m_lines.size() - 1; i > -1; --i)
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
		else if (!m_lines.at(i))
		{
			m_lines.erase(m_lines.begin() + i);
			continue;
		}

		if (!m_lines.at(i)->Render(deviceContext))
			return false;
	}

	if (m_markingArea)
	{
		XMMATRIX worldMatrix = XMMatrixIdentity();
		worldMatrix = XMMatrixMultiply(worldMatrix, XMMatrixTranslation(0.0f, 0.0f, 0.0f));
		worldMatrix = XMMatrixMultiply(worldMatrix, XMMatrixScaling(m_scale, m_scale, m_scale));

		if (!m_markingArea->Render(m_D3D->GetDeviceContext(), 0, worldMatrix, worldMatrix * 0, worldMatrix * 0))
			return false;
	}

	return true;
}