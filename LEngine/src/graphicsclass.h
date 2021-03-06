#ifndef _GRAPHICSCLASS_H_
#define _GRAPHICSCLASS_H_

#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx11.h"
#include "d3dclass.h"
#include "cameraclass.h"
#include "modelclass.h"
#include "ShaderSpecularClass.h"
#include "UIBase.h"
#include "TextEngine.h"
#include "UIBackground.h"
#include "UIButton.h"
#include "UITick.h"
#include "MouseClassContainer.h"
#include "UISlider.h"
#include "ShaderPBRClass.h"
#include "ShaderPBRGenerated.h"
#include "UITexturePreview.h"
#include "SkyboxShaderClass.h"
#include "RenderTextureClass.h"
#include "UITexture.h"
#include "BlurShaderClass.h"
#include <ScreenGrab.h>
#include <iostream>
#include <fstream>
#include <numeric>
#include <vector>
#include <chrono>
#include <WICTextureLoader.h>
#include "ShadowMapClass.h"
#include "LightClass.h"
#include "SingleColorClass.h"
#include "GBufferShader.h"
#include "BloomShaderClass.h"
#include <random>
#include "PostProcessShader.h"
#include "VignetteShader.h"
#include "LUTShader.h"
#include "ShaderEditorManager.h"
#include "json11.hpp"
#include "ModelPickerShader.h"
#include "FXAAShader.h"
#include <array>
#include "PointLight.h"
#include "ShadowShader.h"

/////////////
// GLOBALS //
/////////////
const bool FULL_SCREEN = false;
const bool VSYNC_ENABLED = true;
const float SCREEN_DEPTH = 1000.0f;
const float SCREEN_NEAR = 0.01f;
const bool BLUR_BILINEAR = false;
const bool ENABLE_DEBUG = false;
const bool DRAW_SKYBOX = true;

const int CONVOLUTION_DIFFUSE_SIZE = 256;
const int ENVIRONMENT_SPECULAR_SIZE = 128;
const int SSAO_KERNEL_SIZE = 64;
const int SSAO_NOISE_SIZE = 16;

const int MAX_TEXTURE_INPUT = 4;

static const std::array<const char*, 3> GrainTypeArray { "Small", "Unregular", "Unregular white" };
static const char* CurrentGrainType = GrainTypeArray[0];

static const std::array<const char*, 4> AntialiasingTypeArray = { "OFF", "MSAA", "FXAA", "SSAA" };
static const char* CurrentAntialiasingType = AntialiasingTypeArray[0];

static const std::array<const char*, 3> AntialiasingCountArrayMSAA = { "x2", "x4", "x8" };
static const char* CurrentAntialiasingCountMSAA = AntialiasingCountArrayMSAA[0];

static const std::array<const char*, 3> AntialiasingCountArraySSAA = { "x2", "x4", "x8 - NOT RECOMMENDED" };
static const char* CurrentAntialiasingCountSSAA = AntialiasingCountArraySSAA[0];

constexpr float MODEL_DRAG_SPEED = 1.0f;

class GraphicsClass
{
public:
	bool ENABLE_GUI = true;
	bool RENDER_MATERIAL_EDITOR = true;
	bool DRAW_AABB = false;

	struct PreviewModelData
	{
		ModelClass* model{ nullptr };
		XMFLOAT2 screenPos{ 0.0f, 0.0f };
		XMFLOAT3 worldPos{ 0.0f, 0.0f, 0.0f };
		XMFLOAT2 baseSize{ 0.0f, 0.0f };
		Bounds bounds;
		float yCameraPos{ 0.0f };
	};

	struct MouseRaycastResult
	{
	public:
		XMFLOAT3 origin{};
		XMFLOAT3 dirfrac{};

		MouseRaycastResult(XMFLOAT3 Origin, XMFLOAT3 Dirfrac) : origin(Origin), dirfrac(Dirfrac) {}
	};

	struct ModelPickerBools
	{
	public:
		bool xAxis{ false };
		bool yAxis{ false };
		bool zAxis{ false };

		void Reset() {
			xAxis = false;
			yAxis = false;
			zAxis = false;
		}
	};

	struct ModelPickerPosition {
	public:
		float x{ 0.0f };
		float y{ 0.0f };

		ModelPickerPosition() = default;
		ModelPickerPosition(float xPosition, float yPosition) : x(xPosition), y(yPosition) {}
	};

	enum class ShaderWindowDirection : int
	{
		Up = 0,
		Down = 1
	};

private:
	enum class GrainType : int
	{
		Small = 0,
		Unregular = 1,
		UnregularWhite = 2,

		Count = 3
	};

	enum class EMaterialInputResult : int
	{
		Continue,
		StopOthers,
		Break
	};

	enum class AntialiasingType : int
	{
		None = 0,
		MSAA = 1,
		FXAA = 2,
		SSAA = 3
	};

	struct BloomSettings
	{
		//float weights[5] = { 0.481f, 0.417f, 0.272f, 0.08f, 0.0f };
		//float weights[5] = { 1.0f, 0.9f, 0.55f, 0.18f, 0.1f };
		std::array<float, BlurShaderClass::k_numberOfWeights> weights { 0.5f, 0.1456f, 0.11538f, 0.10714f, 0.06319f };
		float intensity[3] = { 0.2126f, 0.7152f, 0.0722f };
	};

	struct ChromaticAberrationOffset
	{
		float red{ 0.00364f };
		float green{ -0.00159f };
		float blue{ 0.00682f };
	};

	struct GrainSettings
	{
		float intensity{ 0.11f };
		float size{ 6.83f };
		bool hasColor{ false };
		GrainType type{ GrainType::Small };
	};

	struct AntialiasingSettings
	{
		int sampleCount{ 2 };
		AntialiasingType type{ AntialiasingType::None };
	};

	class ModelPicker
	{
	public:
		enum class Axis {
			X, Y, Z
		};

		ModelPicker(D3DClass* d3d, CameraClass* camera)
		{
			m_cameraModelPicker = camera;

			m_colorShader = new ModelPickerShader;
			m_colorShader->Initialize(d3d->GetDevice(), *d3d->GetHWND(), L"modelpicker.vs", L"modelpicker.ps", d3d->GetBaseInputType());

			m_axisX = new ModelClass;
			m_axisX->Initialize(d3d, modelName.c_str());
			m_axisX->SetScale({ scale.x - 0.001f, scale.y - 0.001f, scale.z - 0.001f });

			m_axisY = new ModelClass;
			m_axisY->Initialize(d3d, modelName.c_str());
			m_axisY->SetScale(scale);

			m_axisZ = new ModelClass;
			m_axisZ->Initialize(d3d, modelName.c_str());
			m_axisZ->SetScale({ scale.x - 0.001f, scale.y, scale.z });
		}

		bool Render(ID3D11DeviceContext* deviceContext, XMMATRIX& worldMatrix, XMMATRIX& viewMatrix, XMMATRIX& projectionMatrix, ModelClass* mainModel)
		{
			SetScales(mainModel);
			{
				const XMFLOAT3 position = GetPosition(mainModel, Axis::X);

				m_colorShader->ChangeColor(1.0f, 0.0f, 0.0f);
				XMMATRIX view = viewMatrix;
				XMMATRIX proj = projectionMatrix;
				XMMATRIX xMatrix = worldMatrix;
				xMatrix = XMMatrixMultiply(xMatrix, DirectX::XMMatrixScaling(m_axisX->GetScale().x, m_axisX->GetScale().y, m_axisX->GetScale().z));
				xMatrix = XMMatrixMultiply(xMatrix, XMMatrixRotationX(0.0174532925f * 90.0f));
				xMatrix = XMMatrixMultiply(xMatrix, XMMatrixTranslation(position.x, position.y, position.z));
				m_axisX->Render(deviceContext);
				if (!m_colorShader->Render(deviceContext, m_axisX->GetIndexCount(), xMatrix, view, proj))
					return false;
			}

			{
				const XMFLOAT3 position = GetPosition(mainModel, Axis::Z);

				m_colorShader->ChangeColor(0.0f, 1.0f, 0.0f);
				XMMATRIX view = viewMatrix;
				XMMATRIX proj = projectionMatrix;
				XMMATRIX yMatrix = worldMatrix;
				yMatrix = XMMatrixMultiply(yMatrix, DirectX::XMMatrixScaling(m_axisX->GetScale().x, m_axisX->GetScale().y, m_axisX->GetScale().z));
				yMatrix = XMMatrixMultiply(yMatrix, XMMatrixRotationY(0.0174532925f * 90.0f));
				yMatrix = XMMatrixMultiply(yMatrix, XMMatrixTranslation(position.x, position.y, position.z));
				m_axisY->Render(deviceContext);
				if (!m_colorShader->Render(deviceContext, m_axisY->GetIndexCount(), yMatrix, view, proj))
					return false;
			}

			{
				const XMFLOAT3 position = GetPosition(mainModel, Axis::Y);

				m_colorShader->ChangeColor(0.0f, 0.0f, 1.0f);
				XMMATRIX view = viewMatrix;
				XMMATRIX proj = projectionMatrix;
				XMMATRIX zMatrix = worldMatrix;
				zMatrix = XMMatrixMultiply(zMatrix, DirectX::XMMatrixScaling(m_axisZ->GetScale().x, m_axisZ->GetScale().y, m_axisZ->GetScale().z));
				zMatrix = XMMatrixMultiply(zMatrix, XMMatrixRotationZ(0.0174532925f * 90.0f));
				zMatrix = XMMatrixMultiply(zMatrix, XMMatrixTranslation(position.x, position.y, position.z));
				m_axisZ->Render(deviceContext);
				if (!m_colorShader->Render(deviceContext, m_axisZ->GetIndexCount(), zMatrix, view, proj))
					return false;
			}

			return true;
		}

		XMFLOAT3 GetMinBounds(ModelClass* mainModel, Axis axis) const
		{
			const XMFLOAT3 basePosition{ mainModel->GetPositionXYZ().x + mainModel->GetBounds().GetCenterX(),
				mainModel->GetPositionXYZ().y + mainModel->GetBounds().GetCenterY(),
				mainModel->GetPositionXYZ().z + mainModel->GetBounds().GetCenterZ() };

			float lengthX{ 0.0f };
			float lengthY{ 0.0f };
			float lengthZ{ 0.0f };

			if (axis == Axis::X)
			{
				//lengthX = 0.0f;
				lengthY = m_axisX->GetBounds().GetSizeY() * 0.5f * scale.y;
				lengthZ = m_axisX->GetBounds().GetSizeZ() * 0.5f * scale.z;
			}
			else if (axis == Axis::Y)
			{
				lengthX = m_axisY->GetBounds().GetSizeY() * 0.5f * scale.y;
				//lengthY = 0.0f;
				lengthZ = m_axisY->GetBounds().GetSizeZ() * 0.5f * scale.z;
			}
			else if (axis == Axis::Z)
			{
				lengthX = m_axisZ->GetBounds().GetSizeZ() * 0.5f * scale.z;
				lengthY = m_axisZ->GetBounds().GetSizeY() * 0.5f * scale.y;
				//lengthZ = 0.0f;
			}

			return{ basePosition.x - lengthX, basePosition.y - lengthY, basePosition.z - lengthZ };
		}

		XMFLOAT3 GetMaxBounds(ModelClass* mainModel, Axis axis) const
		{
			const XMFLOAT3 basePosition	{ mainModel->GetPositionXYZ().x + mainModel->GetBounds().GetCenterX(),
				mainModel->GetPositionXYZ().y + mainModel->GetBounds().GetCenterY(),
				mainModel->GetPositionXYZ().z + mainModel->GetBounds().GetCenterZ() };

			float lengthX{ 0.0f };
			float lengthY{ 0.0f };
			float lengthZ{ 0.0f };

			if (axis == Axis::X)
			{
				lengthX = m_axisX->GetBounds().GetSizeX() * 1.0f * scale.x;
				lengthY = m_axisX->GetBounds().GetSizeY() * 0.5f * scale.y;
				lengthZ = m_axisX->GetBounds().GetSizeZ() * 0.5f * scale.z;
			}
			else if (axis == Axis::Y)
			{
				lengthX = m_axisY->GetBounds().GetSizeY() * 0.5f * scale.y;
				lengthY = m_axisY->GetBounds().GetSizeX() * 1.0f * scale.x;
				lengthZ = m_axisY->GetBounds().GetSizeZ() * 0.5f * scale.z;
			}
			else if (axis == Axis::Z)
			{
				lengthX = m_axisZ->GetBounds().GetSizeZ() * 0.5f * scale.z;
				lengthY = m_axisZ->GetBounds().GetSizeY() * 0.5f * scale.y;
				lengthZ = m_axisZ->GetBounds().GetSizeX() * 1.0f * scale.x;
			}

			return{ basePosition.x + lengthX, basePosition.y + lengthY, basePosition.z + lengthZ };
		}

		XMFLOAT2 GetMinScreenspace(GraphicsClass* graphics, ModelClass* model, const Axis axis) const
		{
			return WorldToScreenspace(graphics, GetMinBounds(model, axis), model);
		}

		XMFLOAT2 GetMaxScreenspace(GraphicsClass* graphics, ModelClass* model, const Axis axis) const
		{
			return WorldToScreenspace(graphics, GetMaxBounds(model, axis), model);
		}

		XMFLOAT2 WorldToScreenspace(GraphicsClass* graphics, const XMFLOAT3 worldPos, ModelClass* const model) const
		{
			//XMMATRIX worldMatrix;
			//XMMATRIX viewMatrix;
			//XMMATRIX projectionMatrix;
			//XMVECTOR pos = { worldPos.x, worldPos.y, worldPos.z };
			//graphics->m_D3D->GetWorldMatrix(worldMatrix);
			//graphics->m_Camera->GetViewMatrix(viewMatrix);
			//graphics->m_D3D->GetProjectionMatrix(projectionMatrix);

			////Transform to view space (camera space)
			//worldMatrix = XMMatrixMultiply(worldMatrix, viewMatrix);
			//worldMatrix = XMMatrixMultiply(worldMatrix, projectionMatrix);
			//pos = XMVector3Transform(pos, worldMatrix);

			////Perspective projection
			//XMFLOAT2 screen = { pos.m128_f32[0], pos.m128_f32[1] };
			//screen.x /= pos.m128_f32[2]; //Divide by camera.z
			//screen.y /= pos.m128_f32[2]; //Divide by camera.z
			//							 //screen.y *= 2.0f;

			//if (std::abs(screen.x) > graphics->m_screenWidth || std::abs(screen.y) > graphics->m_screenHeight)
			//	return{ static_cast<float>(graphics->m_screenWidth), static_cast<float>(graphics->m_screenHeight) };

			//constexpr float canvasWidth = 2.0f;
			//constexpr float canvasHeight = 2.0f;

			//screen.x = (screen.x + 1.0f) / 2.0f;
			////screen.x = (screen.x + canvasWidth / 2.0f) / canvasWidth;
			//screen.y = (screen.y + canvasHeight / 2.0f) / canvasHeight;

			//return screen;

			XMMATRIX worldMatrix;
			XMMATRIX viewMatrix;
			XMMATRIX projectionMatrix;
			graphics->m_D3D->GetWorldMatrix(worldMatrix);
			graphics->m_Camera->GetViewMatrix(viewMatrix);
			graphics->m_D3D->GetProjectionMatrix(projectionMatrix);

			worldMatrix = DirectX::XMMatrixMultiply(worldMatrix, DirectX::XMMatrixScaling(model->GetScale().x, model->GetScale().y, model->GetScale().z));
			worldMatrix = DirectX::XMMatrixMultiply(worldMatrix, DirectX::XMMatrixRotationX(model->GetRotation().x * 0.0174532925f));
			worldMatrix = DirectX::XMMatrixMultiply(worldMatrix, DirectX::XMMatrixRotationY(model->GetRotation().y * 0.0174532925f));
			worldMatrix = DirectX::XMMatrixMultiply(worldMatrix, DirectX::XMMatrixRotationZ(model->GetRotation().z * 0.0174532925f));
			worldMatrix = DirectX::XMMatrixMultiply(worldMatrix, XMMatrixTranslation(model->GetPosition().x, model->GetPosition().y, model->GetPosition().z));

			XMVECTOR clipSpacePos = XMVector4Transform({ worldPos.x, worldPos.y, worldPos.z, 1.0f }, worldMatrix);
			clipSpacePos = XMVector4Transform(clipSpacePos, viewMatrix);
			clipSpacePos = XMVector4Transform(clipSpacePos, projectionMatrix);
			clipSpacePos.m128_f32[0] /= clipSpacePos.m128_f32[3];
			clipSpacePos.m128_f32[1] /= clipSpacePos.m128_f32[3];

			return{ clipSpacePos.m128_f32[0], clipSpacePos.m128_f32[1] };
		}

		void SetScales(ModelClass* model)
		{
			const XMFLOAT3 cameraPos = m_cameraModelPicker->GetPosition();
			const XMFLOAT3 modelPos = model->GetPositionXYZ();
			float dist = (cameraPos.x - modelPos.x) * (cameraPos.x - modelPos.x) + (cameraPos.y - modelPos.y) * (cameraPos.y - modelPos.y) + (cameraPos.z - modelPos.z) * (cameraPos.z - modelPos.z);
			dist = sqrt(dist);

			scaleMult = dist * 0.15f;
			scale = { pickerLength * scaleMult, 0.025f * scaleMult, 0.025f * scaleMult };

			m_axisX->SetScale({ scale.x - 0.001f, scale.y - 0.001f, scale.z - 0.001f });
			m_axisY->SetScale(scale);
			m_axisZ->SetScale({ scale.x - 0.001f, scale.y, scale.z });
		}

	private:
		XMFLOAT3 GetPosition(ModelClass* mainModel, Axis axis) {
			const float length = m_axisX->GetBounds().GetSizeX() * scale.x * 0.5f;
			return{ mainModel->GetPositionXYZ().x + mainModel->GetBounds().GetCenterX() + (axis == Axis::X ? length : 0.0f),
				mainModel->GetPositionXYZ().y + mainModel->GetBounds().GetCenterY() + (axis == Axis::Y ? length : 0.0f),
				mainModel->GetPositionXYZ().z + mainModel->GetBounds().GetCenterZ() + (axis == Axis::Z ? length : 0.0f) };
		}

	public:
		ModelClass* m_axisX;
		ModelClass* m_axisY;
		ModelClass* m_axisZ;
	private:
		ModelPickerShader* m_colorShader;
		CameraClass* m_cameraModelPicker;
		const std::string modelName = "modelPicker.obj";
		float scaleMult = 15.0f;
		const float pickerLength = 0.35f;
		XMFLOAT3 scale = { pickerLength * scaleMult, 0.025f * scaleMult, 0.025f * scaleMult };
	};

public:
	bool Initialize(int, int, HWND);
	void Shutdown();
	bool Frame();
	void SetCameraPosition(XMFLOAT3 position);
	void SetCameraRotation(XMFLOAT3 rotation);
	void MoveCameraForward(float val);
	void MoveCameraBackward(float val);
	void MoveCameraLeft(float val);
	void MoveCameraRight(float val);
	void MoveCameraUp(float val);
	void MoveCameraDown(float val);
	void RotateCamera(XMVECTOR rotation);
	void AddPreviewCameraRotation(float x, float y);

	void UpdateUI();
	void UpdateShaderEditorMouseOnly();
	void SetMouseRef(MouseClass* mouse);
	MouseClass* GetMouse();
		
	D3DClass* GetD3D();

	TextEngine::FontData* AddText(float&& posX, float&& posY, std::string&& text, float&& scale = 1.0f, TextEngine::Align &&align = TextEngine::Align::LEFT, XMVECTOR&& color = DirectX::Colors::White);

	void ChangeRenderWindow();

	void DeleteCurrentShaderBlock();
	bool IsChoosingShaderWindowActive();
	void ChangeChoosingWindowShaderFocus(ShaderWindowDirection direction);
	void FocusOnChoosingWindowsShader();
	void AcceptCurrentChoosingWindowShader();

	void CopyBlocks();
	void PasteBlocks();

	bool MouseAboveEditorPreview() const;
	std::pair<float, float> GetCurrentMousePosition() const;

	void SaveScene(const std::string name);
	bool ImGuiHovered() const { return m_shaderEditorManager->IsMouseHoveredOnImGui(); };

	void ToggleGUI();
	void ToggleMSAA();

	//Managing models
	bool CopyModel();
	bool PasteModel();

	///// HELPER FUNCTIONS /////
	///<summary>Return a when value == 0, return b when value is >= 1</summary> ///
	static float lerp(float a, float b, float val);
	inline static __int64 GetTimeMillis();
	static float dot(XMFLOAT3 a, XMFLOAT3 b);

#pragma region Model picker
public:
	void TryPickObjects();
	template<class T> bool TryPickObjectsIterate(std::vector<T*>& models, const XMFLOAT3 camPos);
	void TryRayPick();
	void UpdateRayPick();
	void ResetRayPick();

private:
	float CalculateMouseArrowDotProduct(ModelClass* const model, const ModelPicker::Axis axis, const XMFLOAT2 mouseNormalized);
	void CreateModelPickerMVP(ModelClass* const model, XMMATRIX & worldMatrix, XMMATRIX & viewMatrix, XMMATRIX & projectionMatrix);
	MouseRaycastResult RaycastToModel(ModelClass* const model);
#pragma endregion

private:
	bool Render();
	bool RenderScene();
	bool RenderLightModels();
	bool RenderMaterialPreview();
	bool RenderGUI();
	void ReinitializeMainModel();
	void RefreshModelTick();

	bool RenderSkybox();
	//LIGHTING
	std::vector<PointLight*> m_pointLights;
	//BILINEAR SCREEN BLUR
	bool RenderSceneToTexture();
	bool DownsampleTexture();
	bool DownsampleTexture(RenderTextureClass* dst, RenderTextureClass* const src, const float size = 1.0f, const int depthStencilViewSize = -1);
	bool UpscaleTexture();
	bool BlurFilterScreenSpaceTexture(bool vertical, const RenderTextureClass* textureToBlur, RenderTextureClass* textureToReturn, int screenWidth, int depthBufferSize = -1); //Vertical = true; Horizontal = false
	bool BlurFilterScreenSpaceTexture(bool vertical, const ID3D11ShaderResourceView* textureToBlur, RenderTextureClass* textureToReturn, int screenWidth, int depthBufferSize = -1); //Vertical = true; Horizontal = false
	bool BlurFilterScreenSpace(bool vertical); //Vertical = true; Horizontal = false
	bool BlurFilterScreenSpace(bool vertical, const RenderTextureClass* textureToBlur, RenderTextureClass* textureToReturn, int screenWidth); //Vertical = true; Horizontal = false
	bool BlurFilterScreenSpace(bool vertical, const ID3D11ShaderResourceView* textureToBlur, RenderTextureClass* textureToReturn, int screenWidth); //Vertical = true; Horizontal = false
	//IBL DIFFUSE CONVOLUTION
	bool DownsampleSkybox();
	bool DownsampleSkyboxFace(const wchar_t* inputFilename, const wchar_t* outputFilename, bool isDDS);
	bool BlurFilter(bool vertical, UITexture* srcTex, RenderTextureClass* dstTex, int width); //Vertical = true; Horizontal = false
	bool BlurFilter(bool vertical, RenderTextureClass* srcTex, RenderTextureClass* dstTex, int width); //Vertical = true; Horizontal = false
	bool BlurFilter(bool vertical, ID3D11ShaderResourceView* srcTex, RenderTextureClass* dstTex, int width);
	bool ConvoluteShader(ID3D11ShaderResourceView* srcTex, RenderTextureClass* dstTex);
	//SHADOW MAPPING
	bool CreateShadowMap(RenderTextureClass*& targetTex);
	bool RenderDepthScene();
	bool RenderShadow();
	//IBL SPECULAR
	bool PrepareEnvironmentPrefilteredMap(ID3D11ShaderResourceView* srcTex, RenderTextureClass* dstTex);
	bool PrepareLutBrdf(RenderTextureClass* dstTex);
	bool CreateSingleEnvironmentMap();
	//SSAO
	bool RenderGBufferMain(GBufferShader *& gBuffer, RenderTextureClass *targetTex);
	bool RenderGBufferPosition(RenderTextureClass *targetTex, GBufferShader* shaderToExecute);
	bool RenderGBufferNormal(RenderTextureClass *targetTex, GBufferShader* shaderToExecute);
	bool RenderGBufferAlbedo(RenderTextureClass *targetTex, GBufferShader* shaderToExecute);
	bool RenderSSAONoiseTexture(RenderTextureClass *targetTex, GBufferShader* shaderToExecute);
	bool RenderSSAOTexture(RenderTextureClass *targetTex, GBufferShader* shaderToExecute);
	//ImGUI
	inline void RenderTextureViewImGui(ID3D11Resource*& resource, ID3D11ShaderResourceView*& resourceView, const char* label);
	inline void RenderTextureViewImGuiEditor(ID3D11Resource*& resource, ID3D11ShaderResourceView*& resourceView, const char* label, std::string& path, bool skipLabel = false);
	EMaterialInputResult RenderInputForMaterial(UIShaderEditorBlock* block, bool changeName = false, bool isActive = false);
	//Applying post-processes
	bool ApplySSAO(ID3D11ShaderResourceView* ssaoMap);
	bool ApplyBloom(ID3D11ShaderResourceView* bloomTexture);
	bool ApplyLUT(ID3D11ShaderResourceView* lutTexture);
	bool ApplyChromaticAberration();
	bool ApplyGrain();
	bool RenderVignette();
	bool RenderPostprocess(ID3D11ShaderResourceView* mainFrameBuffer);

	//FXAA
	bool RenderFXAATexture(RenderTextureClass * targetTex);

	//Fill shader editor manager with data
	bool CreateShaderEditor();

	//Saving/Loading scene
	void LoadPointLightSave(PointLight* model, json11::Json json);
	void LoadModelSave(ModelClass* model, json11::Json json);
	void CreateModelSave(std::string modelName, json11::Json json);
	void LoadScene(const std::string name);
	void CreateAABB(ModelClass* baseModel);
	void CreateAABBBox(const Bounds bounds);
	bool TestAABBIntersection(XMFLOAT3 lb, XMFLOAT3 rt, XMFLOAT3 origin, XMFLOAT3 dirfrac, float& distance);
	bool TryPickModelPickerArrow(ModelClass* model, const ModelPicker::Axis axis, XMFLOAT3&& origin, XMFLOAT3 dirfrac);

	//OBB tests
	bool RaySlabIntersect(ModelClass* model, float start, float dir, float min, float max, float& tfirst, float& tlast);
	bool TestOBBIntersection(ModelClass* model, XMFLOAT3 origin, XMFLOAT3 dir, XMFLOAT3 lb, XMFLOAT3 rt, float& dist);

private:
	D3DClass* m_D3D;
	CameraClass* m_Camera;
	std::vector<ModelClass*> m_sceneModels;
	ModelClass* m_copiedModel;
	ModelClass* m_selectedModel;
	ModelClass* m_Model;
	ModelClass* m_cubeModel;
	ModelClass* m_skyboxModel;
	ModelClass* m_raycastModel;
	PreviewModelData m_previewData;
	ShaderSpecularClass* m_specularShader;
	SkyboxShaderClass* m_skyboxShader;
	TextEngine* m_textEngine;
	UITexturePreview* m_texturePreviewRoughness;
	UITexturePreview* m_texturePreviewMetalness;
	UITexturePreview* m_texturePreviewNormal;
	UITexturePreview* m_texturePreviewAlbedo;
	std::array<ID3D11ShaderResourceView*, MAX_TEXTURE_INPUT> m_emptyTexView{ nullptr };
	ID3D11ShaderResourceView* m_emptyTexViewEditor;

	UIBackground* m_debugBackground;
	MouseClassContainer* m_mouse;
	UISlider* m_roughnessSlider;
	UISlider* m_metalnessSlider;

	LightClass* m_directionalLight;
	RenderTextureClass* m_shaderPreview;
	RenderTextureClass* m_msaaSkybox;
	RenderTextureClass* m_msaaResolveTexture;

	ModelPicker* m_modelPicker;

	//BILINEAR SCREEN BLUR
	RenderTextureClass* m_renderTexture;
	RenderTextureClass* m_renderTextureMainScene;
	RenderTextureClass* m_renderTextureDownsampled;
	RenderTextureClass* m_renderTextureUpscaled;
	RenderTextureClass* m_renderTextureHorizontalBlur;
	RenderTextureClass* m_renderTextureVerticalBlur;
	UITexture* m_renderTexturePreview;
	BlurShaderClass* m_blurShaderVertical;
	BlurShaderClass* m_blurShaderHorizontal;

	//CONVOLUTION IBL DIFFUSE
	RenderTextureClass* m_skyboxDownsampled;
	RenderTextureClass* m_skyboxBlurHorizontal;
	RenderTextureClass* m_skyboxBlurVertical;
	ModelPickerShader* m_singleColorShader;
	SkyboxShaderClass* m_convoluteShader;
	ModelClass* m_convoluteQuadModel;

	//IBL SPECULAR SETTINGS
	SkyboxShaderClass* m_specularIBLShader;
	RenderTextureClass* m_environmentTextureMap;
	RenderTextureClass* m_brdfLUT;
	BaseShaderClass* m_brdfLutShader;

	//PREVIEW SKYBOX
	RenderTextureClass* m_skyboxTextureLeft;
	RenderTextureClass* m_skyboxTextureRight;
	RenderTextureClass* m_skyboxTextureUp;
	RenderTextureClass* m_skyboxTextureDown;
	RenderTextureClass* m_skyboxTextureForward;
	RenderTextureClass* m_skyboxTextureBack;

	UITexture* m_skyboxPreviewLeft;
	UITexture* m_skyboxPreviewRight;
	UITexture* m_skyboxPreviewUp;
	UITexture* m_skyboxPreviewDown;
	UITexture* m_skyboxPreviewForward;
	UITexture* m_skyboxPreviewBack;

	//Material editor
	ShaderEditorManager* m_shaderEditorManager;

	//SHADOW MAP
	ShadowMapClass* m_shadowMapShader;
	ShadowShader* m_shadowShader;
	RenderTextureClass* m_shadowMapTexture;
	ModelClass* m_shadowQuadModel;
	ModelClass* m_groundQuadModel;

	//SSAO
	//GBufferShader* m_GBufferShader;
	GBufferShader* m_GBufferShaderPosition;
	GBufferShader* m_GBufferShaderNormal;
	GBufferShader* m_GBufferShaderSSAO;
	RenderTextureClass* m_positionBuffer;
	RenderTextureClass* m_normalBuffer;
	RenderTextureClass* m_albedoBuffer;
	RenderTextureClass* m_ssaoNoiseTexture;
	RenderTextureClass* m_ssaoTexture;
	RenderTextureClass* m_postSSAOTexture;
	//XMFLOAT3 m_ssaoKernel[SSAO_KERNEL_SIZE];
	//XMFLOAT2 m_ssaoNoise[16];

	//BLOOM
	BloomShaderClass* m_bloomShader;
	RenderTextureClass* m_bloomHelperTexture;
	RenderTextureClass* m_bloomHorizontalBlur;
	RenderTextureClass* m_bloomVerticalBlur;
	BloomSettings m_bloomSettings;

	//VIGNETTE
	VignetteShader* m_vignetteShader;

	//LUT
	LUTShader* m_lutShader;

	//CHROMATIC ABERRATION
	ChromaticAberrationOffset m_chromaticOffset;
	float m_chromaticIntensity{ 0.585f };

	//GRAIN
	GrainSettings m_grainSettings;

	//ANTI-ALIASING
	FXAAShader* m_fxaaShader;
	RenderTextureClass* m_antialiasedTexture;
	RenderTextureClass* m_ssaaTexture;
	RenderTextureClass* m_ssaaTextureHelper;
	RenderTextureClass* m_ssaaTextureBlurred_2;
	RenderTextureClass* m_ssaaTextureBlurredHor;
	RenderTextureClass* m_ssaaTextureBlurredVer;
	AntialiasingSettings m_antialiasingSettings;

	//ImGUI
	int m_internalTextureViewIndex = -1;
	bool m_focusOnChoosingWindowsShader{ false };
	bool m_hideShaderWindowOnNextTry{ false };

	//POST-PROCESS STACK
	PostProcessShader* m_postProcessShader;

	//Model-picker
	ModelPickerBools m_modelPickerBools;
	ModelPickerPosition m_modelPickerPosition;

	//Materials
	std::map<std::string, MaterialPrefab*> m_materialList;

	//////////////////////////////
	// Post-process using flags //
	//////////////////////////////
	bool m_postprocessSSAO = false;
	bool m_postprocessBloom = false;
	bool m_postprocessVignette = false;
	bool m_postprocessLUT = false;
	bool m_postprocessChromaticAberration = false;
	bool m_postprocessGrain = false;
//Anti-aliasing postprocess
	bool m_postprocessFXAA = false;
	bool m_postprocessSSAA = false;
	bool m_postprocessMSAA = false;

	float m_rotationY = 0.0f;
	int m_screenWidth = 0;
	int m_screenHeight = 0;
	int m_exitCount = 0;
	int m_convolutionFace = 0;
};

template <typename T>
T clamp(const T val, const T min, const T max)
{
	if (val > max)
		return max;
	if (val < min)
		return min;

	return val;
}

#endif