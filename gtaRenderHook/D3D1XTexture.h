
#ifndef D3D1XTexture_h__
#define D3D1XTexture_h__
class CD3DRenderer;
class CD3D1XShader;
enum class eD3D1XTextureType
{
    TT_Texture,
    TT_RenderTarget,
    TT_DepthStencil,
    TT_3DTexture
};
/*!
    \class CD3D1XTexture
    \brief Base D3D texture class.

    This class represents texture buffer and views over this buffer.
    TODO: Separate in different classes, perhaps add somekind of texture factory
*/
class CD3D1XTexture
{
public:
    /*!
        Initializes texture, allocates buffers and generates views based on raster flags.
    */
    CD3D1XTexture( RwRaster* pParent, bool mipMaps, bool hasPalette = false );
    /*!
        Releases resources
    */
    ~CD3D1XTexture();
    /*!
        Returns hardware 2D texture buffer pointer
    */
    ID3D11Texture2D*					GetTexture() const { return m_pTexture; }
    /*!
        Returns hardware 3D texture buffer pointer
    */
    ID3D11Texture3D*					Get3DTexture() const { return m_p3DTexture; }
    /*!
        Returns shader resource view pointer
    */
    ID3D11ShaderResourceView*			GetSRV() const { return m_shaderRV; }
    /*!
        Returns render target view pointer
    */
    ID3D11RenderTargetView*				GetRTRV() const { return m_renderTargetRV; }
    /*!
        Returns depth stencil view pointer
    */
    ID3D11DepthStencilView*				GetDSRV() const { return m_depthStencilRV; }
    /*!
        Returns unordered access view pointer
    */
    ID3D11UnorderedAccessView*			GetUAV() const { return m_unorderedAV; }
    /*!
        Returns true if rasters has palette
    */
    bool						&hasPalette() { return m_hasPalette; }
    /*!
        Tells texture is being rendered to
    */
    void						BeginRendering() { m_isRendering = true; }
    /*!
        Tells texture is stop being rendered to and ready to use
    */
    void						EndRendering() { m_isRendering = false; }
    /*!
        Returns palette buffer pointer
    */
    RwRGBA*						GetPalettePtr() { return m_palette; }
    /*!
        Returns if texture is currently locked for reading
    */
    bool						&IsLockedToRead() { return m_isLockedToRead; }
    /*!
        Initializes camera raster texture
    */
    void						InitCameraRaster( ID3D11Device * dev );
    /*!
        Initializes camera texture raster texture
    */
    void						InitCameraTextureRaster( ID3D11Device * dev, D3D11_TEXTURE2D_DESC *desc, RwD3D1XRaster * d3dRaster, bool mipmaps );
    /*!
        Locks texture to preform some operation on texture buffer
    */
    void*						LockToRead();
    /*!
        Unlocks texture from reading
    */
    void						UnlockFromRead();
    /*!
        Reallocates texture buffer to new size
    */
    void						Resize( UINT newWidth, UINT newHeight );
    /*!
        Reloads texture
    */
    void						Reload();
    UINT	m_nWidth;
    UINT	m_nHeight;

private:
    D3D11_MAPPED_SUBRESOURCE			m_mappedSubRes;
    eD3D1XTextureType					m_type;
    RwRaster*							m_pParent = nullptr;
    ID3D11Texture2D*					m_pTexture = nullptr;
    ID3D11Texture3D*					m_p3DTexture = nullptr;
    ID3D11Texture2D*					m_pStagingTexture = nullptr;
    ID3D11ShaderResourceView*           m_shaderRV = nullptr;
    union
    {
        ID3D11RenderTargetView*				m_renderTargetRV = nullptr;
        ID3D11DepthStencilView* 			m_depthStencilRV;
        ID3D11UnorderedAccessView*			m_unorderedAV;
    };
    RwRGBA  m_palette[256];
    BYTE* m_dataPtr;
    bool m_isRendering = false,
        m_isLockedToRead = false,
        m_hasPalette = false;
};
#endif // !D3D1XTexture_h__


