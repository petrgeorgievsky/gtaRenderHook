#pragma once
#include "Definitions.h"
namespace RHFormat
{
    inline std::string D3D9NativeRasterFlags( RwUInt8 flags )
    {
        std::string res;

        if ( flags & ( 1 << 0 ) )
            res += " HAS_ALPHA |";
        if ( flags & ( 1 << 1 ) )
            res += " IS_CUBE |";
        if ( flags & ( 1 << 2 ) )
            res += " USE_AUTOMIPMAPGEN |";
        if ( flags & ( 1 << 3 ) )
            res += " IS_COMPRESSED";

        // if you know better way replace it, I'm lazy
        if ( !res.empty() && *( res.end() - 1 ) == '|' )
            res.end() = res.end()--;

        return res;
    }

    inline rh::engine::ImageBufferFormat GetRHFormatFromRWFORMAT( D3DFORMAT format, RwRasterFormat rwFormat, RwPlatformID platform, uint32_t dxt_compression )
    {
        switch ( platform )
        {
        case rwID_PCD3D7:
            break;
        case rwID_PCOGL:
            break;
        case rwID_MAC:
            break;
        case rwID_PS2:
            break;
        case rwID_XBOX:
            break;
        case rwID_GAMECUBE:
            break;
        case rwID_SOFTRAS:
            break;
        case rwID_PCD3D8:
            switch ( dxt_compression )
            {
            case 0:
                if ( rwFormat & rwRASTERFORMATPAL4 || rwFormat & rwRASTERFORMATPAL8 )
                {
                    return rh::engine::ImageBufferFormat::BGRA8;
                }
                else
                {
                    switch ( rwFormat )
                    {
                    case rwRASTERFORMATDEFAULT:
                        return rh::engine::ImageBufferFormat::BGRA8;
                    case rwRASTERFORMAT1555:
                        return rh::engine::ImageBufferFormat::BGR5A1;
                    case rwRASTERFORMAT565:
                        return rh::engine::ImageBufferFormat::B5G6R5;
                    case rwRASTERFORMAT4444:
                        return rh::engine::ImageBufferFormat::BGRA4;
                    case rwRASTERFORMATLUM8:
                        return rh::engine::ImageBufferFormat::A8;
                    case rwRASTERFORMAT8888:
                        return rh::engine::ImageBufferFormat::BGRA8;
                    case rwRASTERFORMAT888:
                        return rh::engine::ImageBufferFormat::BGR8;
                    case rwRASTERFORMAT555:
                        return rh::engine::ImageBufferFormat::BGR5A1;
                    default:
                        break;
                    }
                }
                break;
            case 1:
                return rh::engine::ImageBufferFormat::BC1;
            case 2:
            case 3:
                return rh::engine::ImageBufferFormat::BC2;
            case 4:
            case 5:
                return rh::engine::ImageBufferFormat::BC3;
            }
            break;
        case rwID_PCD3D9:
            switch ( format )
            {
            case D3DFMT_A8R8G8B8:
                return rh::engine::ImageBufferFormat::BGRA8;
            case D3DFMT_X8R8G8B8:
                return rh::engine::ImageBufferFormat::BGR8;
            case D3DFMT_DXT1:
                return rh::engine::ImageBufferFormat::BC1;
            case D3DFMT_DXT2:
            case D3DFMT_DXT3:
                return rh::engine::ImageBufferFormat::BC2;
            case D3DFMT_DXT4:
            case D3DFMT_DXT5:
                return rh::engine::ImageBufferFormat::BC3;
            // These formats are pure d3d11 formats, currently unused
            // TODO: Implement D3DFMT_DXT4 ext to handle them
/*#pragma warning(push, 0)
            case MAKEFOURCC( 'A', 'T', 'I', '1' ):
                return rh::engine::RHImageBufferFormat::BC4;
            case MAKEFOURCC( 'A', 'T', 'I', '2' ):
                return rh::engine::RHImageBufferFormat::BC5;
            case MAKEFOURCC( 'B', 'C', '6', 'H' ):
                return rh::engine::RHImageBufferFormat::BC6H;
            case MAKEFOURCC( 'D', 'X', 'T', '7' ):
                return rh::engine::RHImageBufferFormat::BC7;
#pragma warning(pop)*/
            }
            break;
        default:
            break;
        }
        return rh::engine::ImageBufferFormat::Unknown;
    }
}
