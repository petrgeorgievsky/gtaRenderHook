
#ifndef D3D1XTexture_h__
#define D3D1XTexture_h__
class CD3DRenderer;
class CD3D1XShader;
enum class eD3D1XTextureType {
	TT_Texture,
	TT_RenderTarget,
	TT_DepthStencil,
	TT_3DTexture
};
class CD3D1XTexture
{
public:
	CD3D1XTexture(RwRaster* pParent, bool mipMaps, bool hasPalette=false);
	~CD3D1XTexture();

	CComPtr<ID3D11Texture2D>			GetTexture() const { return m_pTexture; }
	CComPtr<ID3D11Texture3D>			Get3DTexture() const { return m_p3DTexture; }
	ID3D11ShaderResourceView*	GetSRV() const { return m_shaderRV.p; }
	ID3D11RenderTargetView*		GetRTRV() const { return m_renderTargetRV.p; }
	ID3D11DepthStencilView*		GetDSRV() const { return m_depthStencilRV.p; }
	CComPtr<ID3D11UnorderedAccessView>	GetUAV() const { return m_unorderedAV; }
	bool						&isRendering()  { return m_isRendering; }
	bool						&hasPalette()  { return m_hasPalette; }
	void						BeginRendering() { m_isRendering = true; }
	void						EndRendering() { m_isRendering = false; }
	RwRGBA*						GetPalettePtr() { return m_palette; }
	bool						&IsLockedToRead() { return m_isLockedToRead; }
	void						InitCameraRaster		(ID3D11Device * dev);
	void						InitCameraTextureRaster	(ID3D11Device * dev, D3D11_TEXTURE2D_DESC *desc, RwD3D1XRaster * d3dRaster,bool mipmaps);
	void*						LockToRead();
	void						UnlockFromRead();
	void Resize(UINT newWidth, UINT newHeight);
	void						Reload();
	UINT	m_nWidth;
	UINT	m_nHeight;

private:
	D3D11_MAPPED_SUBRESOURCE			m_mappedSubRes;
	eD3D1XTextureType					m_type;
	RwRaster*							m_pParent			= nullptr;
	CComPtr<ID3D11Texture2D>			m_pTexture = nullptr;
	CComPtr<ID3D11Texture3D>			m_p3DTexture = nullptr;
	CComPtr<ID3D11Texture2D>			m_pStagingTexture = nullptr;
	CComPtr<ID3D11ShaderResourceView>           m_shaderRV = nullptr;
	union {
		CComPtr<ID3D11RenderTargetView>				m_renderTargetRV = nullptr;
		CComPtr<ID3D11DepthStencilView> 			m_depthStencilRV;
		CComPtr<ID3D11UnorderedAccessView>			m_unorderedAV;
	};
	RwRGBA  m_palette[256];
	byte* m_dataPtr;
	bool m_isRendering	= false,
		 m_isLockedToRead=false,
		 m_hasPalette   = false;
};
#endif // !D3D1XTexture_h__


