#pragma once
#include "D3D1XBaseTexture.h"
class CD3D1X2DTexture : public CD3D1XBaseTexture, public ID3D1XShaderViewable
{
  public:
    /*
        \param parent pointer to raster which created this texture resource.
        \param resourceTypeName name of resource type used as a hint in
       RenderDoc/MSVS Graphics Debugger and such. \param createMipMaps if this
       set to true than full mip-chain will be generated \param shaderAccess if
       this set to true it will be possible to set this resource to shaders
    */
    CD3D1X2DTexture( RwRaster *parent, D3D11_BIND_FLAG bindFlags,
                     const std::string &resourceTypeName = "2DTexture",
                     bool createMipMaps = false, bool shaderAccess = false,
                     bool canReallocate = false );
    ~CD3D1X2DTexture();
    virtual void              SetDebugName( const std::string &name );
    ID3D11ShaderResourceView *GetShaderResourceView() const override
    {
        return m_pShaderView;
    }
    void  Reallocate();
    void *LockToRead();
    void  UnlockFromRead();

  protected:
    UINT                      m_nWidth{};
    UINT                      m_nHeight{};
    UINT                      m_dwBindFlags{};
    ID3D11ShaderResourceView *m_pShaderView    = nullptr;
    bool                      m_bCanReallocate = false;
    bool                      m_bHasMipMaps    = false;
    bool                      m_bShaderAccess  = false;

  private:
    ID3D11Resource *         m_pLockedResource = nullptr;
    D3D11_MAPPED_SUBRESOURCE m_lockedSubRes{};
};
