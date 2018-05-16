#include "stdafx.h"
#include "D3D1X2DTexture.h"
#include "D3DSpecificHelpers.h"
#include "RwD3D1XEngine.h"
#include "D3D1XEnumParser.h"
#include "D3DRenderer.h"

CD3D1X2DTexture::CD3D1X2DTexture(RwRaster * parent, D3D11_BIND_FLAG bindFlags,
	const std::string & resourceTypeName, bool createMipMaps, bool shaderAccess):
	CD3D1XBaseTexture(parent, eTextureDimension::TT_2D, resourceTypeName)
{
	RwD3D1XRaster* d3dRaster = GetD3D1XRaster(m_pParent);
	ID3D11Device* dev = GET_D3D_DEVICE;
	// Base texture 2D creation
	D3D11_TEXTURE2D_DESC desc{};
	desc.Width = m_pParent->width;
	desc.Height = m_pParent->height;
	desc.Format = CD3D1XEnumParser::ConvertToTextureBufferSupportedFormat(d3dRaster->format);
	desc.ArraySize = 1;
	desc.MipLevels = createMipMaps ? max((int)log2(min(desc.Width, desc.Height)) - 2, 0) : 1;
	desc.SampleDesc.Quality = 0;
	desc.SampleDesc.Count = 1;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = (shaderAccess ? D3D11_BIND_SHADER_RESOURCE : 0) | bindFlags;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;

	if (!CALL_D3D_API(dev->CreateTexture2D(&desc, NULL, 
		reinterpret_cast<ID3D11Texture2D**>(&m_pTextureResource)), 
		"Failed to create 2D texture"))
		return;
	// If texture has no shader access(e.g. depth stencil surface for camera), than we don't need shader resource view
	if (!shaderAccess)
		return;

	D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc{};
	
	SRVDesc.Format = CD3D1XEnumParser::ConvertToSRVSupportedFormat(d3dRaster->format);
	SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	SRVDesc.Texture2D.MipLevels = UINT32_MAX;

	if (!CALL_D3D_API(dev->CreateShaderResourceView(m_pTextureResource, &SRVDesc, &m_pShaderView),
		"Failed to create shader resource view for 2D texture"))
		return;
	
}

CD3D1X2DTexture::~CD3D1X2DTexture()
{
	if (m_pShaderView) {
		m_pShaderView->Release();
		m_pShaderView = nullptr;
	}
}

void CD3D1X2DTexture::SetDebugName(const std::string & name)
{
	CD3D1XBaseTexture::SetDebugName(name);
	if (m_pShaderView)
		g_pDebug->SetD3DName(m_pShaderView, name + "(" + m_resourceTypeName + ", ShaderResourceView)");
}
