
#ifndef D3D1XTexture_h__
#define D3D1XTexture_h__
class CD3DRenderer;
enum class eD3D1XTextureType {
	TT_Texture,
	TT_RenderTarget,
	TT_DepthStencil
};
class CD3D1XTexture
{
public:
	CD3D1XTexture(CD3DRenderer* pRenderer, RwRaster* pParent, bool mipMaps);
	~CD3D1XTexture();

	ID3D11Texture2D*			&getTexture()	{ return m_pTexture; }
	ID3D11ShaderResourceView*	&getSRV()		{ return m_shaderRV; }
	ID3D11RenderTargetView*		&getRTRV()		{ return m_renderTargetRV; } 
	ID3D11DepthStencilView*		&getDSRV()		{ return m_depthStencilRV; } 
	bool						&isRendering()  { return m_isRendering; }
	void						beginRendering() { m_isRendering = true; }
	void						endRendering() { m_isRendering = false; }
	void*						LockToRead();
	void						UnlockFromRead();
	bool						&isLockedToRead() { return m_isLockedToRead; }
private:
	D3D11_MAPPED_SUBRESOURCE			m_mappedSubRes;
	eD3D1XTextureType					m_type;
	CD3DRenderer*						m_pRenderer			= nullptr;
	RwRaster*							m_pParent			= nullptr;
	ID3D11Texture2D*					m_pTexture			= nullptr;
	ID3D11Texture2D*					m_pStagingTexture	= nullptr;
	ID3D11ShaderResourceView*           m_shaderRV			= nullptr;
	union {
		ID3D11RenderTargetView*				m_renderTargetRV = nullptr;
		ID3D11DepthStencilView*				m_depthStencilRV;
	};
	bool m_isRendering	= false,
		 m_isLockedToRead=false;
};
#endif // !D3D1XTexture_h__


