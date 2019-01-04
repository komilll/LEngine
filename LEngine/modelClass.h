#pragma once
#ifndef _MODEL_CLASS_H_
#define _MODEL_CLASS_H_

#include <d3d11.h>
#include <DirectXMath.h>

class ModelClass
{
private:
	struct VertexType
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT4 color;
		DirectX::XMFLOAT2 tex;
	};

public:
	ModelClass();

	bool Initialize(ID3D11Device *device, ID3D11DeviceContext *deviceContext);
	bool RenderModel();
	int GetIndexCount();

private:
	bool InitializeModel();
	void RenderBuffers();

private:
	ID3D11Buffer *m_vertexBuffer, *m_indexBuffer;

	ID3D11Device *m_device;
	ID3D11DeviceContext *m_deviceContext;
	int m_indexCount;
};

#endif // !_MODEL_CLASS_H_