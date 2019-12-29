#pragma once
class CD3D1XEnumParser
{
public:
    /*!
        Converts format of raster based on flags
    */
    static void							ConvertRasterFormat( RwRaster* raster, RwUInt32 flags );
    /*!
        Converts cull mode to d3d specific enum
    */
    static D3D11_CULL_MODE				ConvertCullMode( RwCullMode mode );
    /*!
        Converts cull mode to rw specific enum
    */
    static RwCullMode					ConvertCullMode( D3D11_CULL_MODE mode );
    /*!
        Converts primitive topology to d3d specific enum
    */
    static D3D11_PRIMITIVE_TOPOLOGY		ConvertPrimTopology( RwPrimitiveType prim );
    /*!
        Converts primitive topology from rw mesh header flags
    */
    static RwPrimitiveType				ConvertPrimTopology( RpMeshHeaderFlags flags );
    /*!
        Converts blend function to d3d specific enum
    */
    static D3D11_BLEND					ConvertBlendFunc( RwBlendFunction func );
    /*!
        Converts blend function to rw specific enum
    */
    static RwBlendFunction				ConvertBlendFunc( D3D11_BLEND func );
    /*!
        Converts stencil operation to d3d specific enum
    */
    static D3D11_STENCIL_OP				ConvertStencilOp( RwStencilOperation op );
    /*!
        Converts stencil function to d3d specific enum
    */
    static D3D11_COMPARISON_FUNC		ConvertStencilFunc( RwStencilFunction func );
    /*!
        Converts texture address mode to d3d specific enum
    */
    static D3D11_TEXTURE_ADDRESS_MODE	ConvertTextureAddressMode( RwTextureAddressMode mode );
    /*!
        Converts texture address mode to rw specific enum
    */
    static RwTextureAddressMode			ConvertTextureAddressMode( D3D11_TEXTURE_ADDRESS_MODE mode );
    /*!
        Converts texture filter mode to d3d specific enum
    */
    static D3D11_FILTER					ConvertTextureFilterMode( RwTextureFilterMode mode );
    /*!
        Converts texture filter mode to rw specific enum
    */
    static RwTextureFilterMode			ConvertTextureFilterMode( D3D11_FILTER mode );
    /*!
        Converts from texture buffer / depth stencil buffer format to format that can be used as shader resource view.
    */
    static DXGI_FORMAT	ConvertToSRVSupportedFormat( DXGI_FORMAT fmt );
    /*!
        Converts from depth stencil buffer format to format that can be used as texture buffer.
    */
    static DXGI_FORMAT	ConvertToTextureBufferSupportedFormat( DXGI_FORMAT fmt );
private:
    static const D3D11_PRIMITIVE_TOPOLOGY m_primConvertTable[];
};

