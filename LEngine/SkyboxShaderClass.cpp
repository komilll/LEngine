#include "SkyboxShaderClass.h"


bool SkyboxShaderClass::SetShaderParameters(ID3D11DeviceContext *deviceContext, XMMATRIX &worldMatrix, XMMATRIX &viewMatrix, XMMATRIX &projectionMatrix)
{
	if (BaseShaderClass::SetShaderParameters(deviceContext, worldMatrix, viewMatrix, projectionMatrix) == false)
		return false;

	/////// RESOURCES ///////
	//Pixel shader resources
	deviceContext->PSSetShaderResources(0, 1, &m_skyboxTextureView);
	return true;
}

bool SkyboxShaderClass::CreateColorSkybox(ID3D11Device* device, int width, int height)
{
	ID3D11Resource* srcTex[6];
	CreateDDSTextureFromFile(device, L"seafloor.dds", &srcTex[0], NULL);
	CreateDDSTextureFromFile(device, L"seafloor.dds", &srcTex[1], NULL);
	CreateDDSTextureFromFile(device, L"seafloor.dds", &srcTex[2], NULL);
	CreateDDSTextureFromFile(device, L"seafloor.dds", &srcTex[3], NULL);
	CreateDDSTextureFromFile(device, L"seafloor.dds", &srcTex[4], NULL);
	CreateDDSTextureFromFile(device, L"seafloor.dds", &srcTex[5], NULL);

	D3D11_TEXTURE2D_DESC texArrayDesc;
	texArrayDesc.Width = width;
	texArrayDesc.Height = height;
	texArrayDesc.MipLevels = 1;
	texArrayDesc.ArraySize = 6;
	texArrayDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	texArrayDesc.SampleDesc.Count = 1;
	texArrayDesc.SampleDesc.Quality = 0;
	texArrayDesc.Usage = D3D11_USAGE_DEFAULT;
	texArrayDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	texArrayDesc.CPUAccessFlags = 0;
	texArrayDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

	ID3D11Texture2D* texArray = 0;
	HRESULT result = device->CreateTexture2D(&texArrayDesc, NULL, &texArray);
	if (FAILED(result))
		return false;

	// Copy individual texture elements into texture array.
	ID3D11DeviceContext* deviceContext;
	device->GetImmediateContext(&deviceContext);
	D3D11_BOX sourceRegion;

	//Here i copy the mip map levels of the textures
	//for (UINT x = 0; x < 6; x++)
	//{
	//	for (UINT mipLevel = 0; mipLevel < texArrayDesc.MipLevels; mipLevel++)
	//	{
	//		sourceRegion.left = 0;
	//		sourceRegion.right = (texArrayDesc.Width >> mipLevel);
	//		sourceRegion.top = 0;
	//		sourceRegion.bottom = (texArrayDesc.Height >> mipLevel);
	//		sourceRegion.front = 0;
	//		sourceRegion.back = 1;

	//		//test for overflow
	//		if (sourceRegion.bottom == 0 || sourceRegion.right == 0)
	//			break;

	//		deviceContext->CopySubresourceRegion(texArray, D3D11CalcSubresource(mipLevel, x, texArrayDesc.MipLevels), 0, 0, 0, srcTex[x], mipLevel, &sourceRegion);
	//	}
	//}

	// Create a resource view to the texture array.
	D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc;
	viewDesc.Format = texArrayDesc.Format;
	viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	viewDesc.TextureCube.MostDetailedMip = 0;
	viewDesc.TextureCube.MipLevels = texArrayDesc.MipLevels;

	result = device->CreateShaderResourceView(texArray, &viewDesc, &m_skyboxTextureView);
	if (FAILED(result))
		return false;
	ID3D11Resource* resource;
	m_skyboxTextureView->GetResource(&resource);
	SaveDDSTextureToFile(deviceContext, resource, L"test.dds");
}