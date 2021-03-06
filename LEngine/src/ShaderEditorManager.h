#pragma once

#ifndef _SHADEREDITORMANAGER_H_
#define _SHADEREDITORMANAGER_H_

#include "UIShaderEditorBlock.h"
#include "UIShaderPBRBlock.h"
#include <vector>
#include "MouseClass.h"
#include "d3dclass.h"
#include "UILine.h"
#include "MaterialPrefab.h"

class ShaderEditorManager
{
private:
	enum class ETryDragResult {
		Succeed, Failed, DoNotCreate
	};

public:
	ShaderEditorManager(D3DClass* d3d, MouseClass* mouse);

	void UpdateBlocks(bool mouseOnly = false);
	void AddShaderBlock(UIShaderEditorBlock* block, int inCount, int outCount);
	void AddShaderBlock(UIShaderEditorBlock* block);
	void GenerateCodeToFile(std::string filename = "");
	
	bool WillRenderChoosingWindow() const;
	int* GetChoosingWindowHandler();
	void CreateBlock(std::string name);
	float GetWindowPositionX() const;
	float GetWindowPositionY() const;
	
	void PressedOutsideOfChoosingWindow();
	void SearchThroughChoosingWindow();

	void DeleteCurrentShaderBlock();
	void ResetMouseHoveredOnImGui();
	void UpdateMouseHoveredOnImGui(bool hovered);
	bool IsMouseHoveredOnImGui() const { return m_mouseHoveredImGui; };
	void SetRefToClickedOutside(bool* clickedOutside);

	void CopyBlocks();
	void PasteBlocks();

	void SetPickingColorElement(UIShaderEditorOutput* out);
	UIShaderEditorOutput* GetPickingColorElement();

	bool MouseAbovePreview() const;
	std::pair<float, float> GetCurrentMousePosition(bool ignoreViewport = false) const;

	//SAVE MATERIAL TO FILE
	bool SaveMaterial(std::string filename);
	bool IsWorkingOnSavedMaterial();
	const std::string GetCurrentMaterialName() const;

	//LOAD MATERIAL FROM FILE
	bool LoadMaterial(std::string filename);
	bool LoadMaterial(int index);
	std::vector<std::string>& GetAllMaterialNames();
	std::vector<MaterialPrefab>& GetAllMaterials();
	std::vector<UIShaderEditorBlock*>& GetMaterialInputs();
	std::vector<std::string> GetUsedTextures() const;
	void LoadAllMaterialsToArray();

private:
	bool RenderBlocks(ID3D11DeviceContext* deviceContext);
	bool UpdatePBRBlock(bool mouseOnly = false);
	bool UpdatePinsOfAllBlocks();
	void DrawLine(UIShaderEditorInput* in, UIShaderEditorOutput* out);
	void ResetFocusOnAllBlocks();
	///<summary> Might fail if there are multiple blocks focused </summary>
	ShaderEditorManager::ETryDragResult TryToResetFocusOnAllBlocks();
	ShaderEditorManager::ETryDragResult TryToResetFocusOnAllBlocks(UIShaderEditorBlock* const other);
	void CreateChoosingWindowItemsArray();
	bool CheckConnectionRules(UIShaderEditorInput* in, UIShaderEditorOutput* out) const;
	bool TryCreateScalarBlocks(std::string name);

	void CopyBlockValues(UIShaderEditorBlock* const src, UIShaderEditorBlock* const dst) const;
	void CopyCreatedBlocksConnections(std::vector<UIShaderEditorBlock*> src, std::vector<UIShaderEditorBlock*> dst);
	bool FindOutputNode(UIShaderEditorOutput * const out, std::vector<UIShaderEditorBlock*> const blocks, std::pair<int, int>& blockInputIndexes) const;

	//GENERATING SHADER CODE
	std::string GenerateBlockCode(UIShaderEditorBlock* block);
	void FillTexturePathsArray();
	std::string GetTextureDeclarations();
	std::string GetTextureDefinitions() const;
	std::string GetFunctionDeclarations() const;
	std::string GetFunctionDefinitions() const;
	vector<std::string> GetFilenamesInDirectory(std::string dir, bool withDir = true) const;
	std::string ConvertReturnType(std::string outName, std::string typeIn, std::string typeOut) const;

	void ShowFunctionChoosingWindow();

	void GeneratePBRClassCode(std::string filename = "");
	void GenerateVariableNames();
	void GenerateTexVariableName(UIShaderEditorBlock* block, int index) const;

	UIBase::RectangleVertices GetMarkingBounds();
	void TryToMarkManyBlocks(float minX, float maxX, float minY, float maxY);
	void TryToMarkManyBlocks(UIBase::RectangleVertices bounds);
	void MoveMultipleBlocks(UIShaderEditorBlock* currentBlock, std::pair<float, float> mouseMov);

	//SAVE/LOAD MATERIALS
	UIShaderEditorOutput* FindOutputNode(std::string name);
	std::string SaveBlockValueMaterial(UIShaderEditorBlock* block);
	void DestroyEditor();
	std::string GenerateMaterialName();

	//MATERIALS
	void LoadMaterialInputs();

public:
	std::vector<const char*> ChoosingWindowItems;
	int ChoosingWindowItemsSize{ -1 };
	std::string m_choosingWindowSearch;
	const int k_choosingWindowSearchSize{ 10 };
	UIShaderEditorBlock* m_focusedBlock{ nullptr };
	bool m_focusedPBR{ false };
	bool m_wasLeftButtonUp{ false };
	bool m_refreshModel{ false };
	int m_refreshModelTicks{ -1 };
	std::string m_materialToSaveName;

	//Additional options to save
	bool m_isEmissive{ false };

private:
	D3DClass* m_D3D;
	MouseClass* m_mouse;
	vector<UIShaderEditorBlock*> m_blocks;
	UIShaderEditorBlock* m_originalGeneratorBlock{nullptr};
	std::string m_originalRequiredType;
	std::vector<const char*> ChoosingWindowItemsOriginal;

	bool m_mouseHoveredImGui{ false };
	bool* m_focusOnChoosingWindowsShader{ nullptr };

	std::vector<UILine*> m_lines;
	UIShaderPBRBlock* m_pbrBlock{ nullptr };
	std::vector<UIShaderEditorBlock*> m_copiedBlocks;

	std::vector<std::string> m_usedVariableNamesInGenerator;
	std::vector<std::string> m_generatedTextureAdresses;
	std::vector<std::string> m_materialNames;
	std::vector<MaterialPrefab> m_materials;
	std::vector<UIShaderEditorBlock*> m_materialInputs;
	std::vector<std::string> m_usedTextures;
	std::vector<std::string> m_universalFunctions;

	std::string m_currentMaterialName;
	int m_blockIDCounter{ -1 };

	float m_scale{ 1.0f };
	bool m_alreadyMarkingArea{ false };
	float m_mouseDragStartX{ 0.0f };
	float m_mouseDragStartY{ 0.0f };
	UIBase* m_markingArea;

	UIShaderEditorOutput* m_pickingColorObject{ nullptr };
	bool m_draggingScreen{ false };
	bool m_choosingWindow{ false };
	int m_choosingWindowHandler{ 0 };
	int m_choosingWindowPosX{ 0 };
	int m_choosingWindowPosY{ 0 };
	float m_choosingWindowPosXScreenspace{ 0 };
	float m_choosingWindowPosYScreenspace{ 0 };

	//TEMPLATE
	template<class T>
	void DestroyVector(std::vector<T>& vec);
};

#endif

template<class T>
inline void ShaderEditorManager::DestroyVector(std::vector<T>& vec)
{
	//TODO Does it make sense? Erase index by index and then clear anyway
	for (int i = vec.size() - 1; i > -1; --i)
		vec.erase(vec.begin() + i);
	vec.clear();
}
