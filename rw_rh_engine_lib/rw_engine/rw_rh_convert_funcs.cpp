#include "rw_rh_convert_funcs.h"
#include <Engine/Common/types/blend_op.h>
#include <Engine/Common/types/cull_mode.h>
#include <Engine/Common/types/image_buffer_format.h>
#include <Engine/Common/types/sampler_addressing.h>

rh::engine::ImageBufferFormat rh::rw::engine::RwFormatToRHImageBufferFormat( RwRasterFormat format )
{
    switch ( format ) {
    case rwRASTERFORMAT1555:
        return rh::engine::ImageBufferFormat::BGR5A1;
    case rwRASTERFORMAT565:
        return rh::engine::ImageBufferFormat::B5G6R5;
    case rwRASTERFORMAT4444:
        return rh::engine::ImageBufferFormat::BGRA4;
    case rwRASTERFORMATLUM8:
        return rh::engine::ImageBufferFormat::A8;
    case rwRASTERFORMATDEFAULT:
    case rwRASTERFORMAT8888:
        return rh::engine::ImageBufferFormat::BGRA8;
    case rwRASTERFORMAT888:
        return rh::engine::ImageBufferFormat::BGR8;
    case rwRASTERFORMAT16:
        return rh::engine::ImageBufferFormat::Unknown;
    case rwRASTERFORMAT24:
        return rh::engine::ImageBufferFormat::Unknown;
    case rwRASTERFORMAT32:
        return rh::engine::ImageBufferFormat::Unknown;
    case rwRASTERFORMAT555:
        return rh::engine::ImageBufferFormat::BGR5A1;
    default:
        return rh::engine::ImageBufferFormat::Unknown;
    }
}

rh::engine::ImageBufferFormat rh::rw::engine::RwNativeFormatToRHImageBufferFormat(
    D3DFORMAT format, RwRasterFormat rwFormat, RwPlatformID platform, uint32_t dxt_compression )
{
    switch ( platform ) {
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
        switch ( dxt_compression ) {
        case 0:
            if ( rwFormat & rwRASTERFORMATPAL4 || rwFormat & rwRASTERFORMATPAL8 ) {
                return rh::engine::ImageBufferFormat::BGRA8;
            } else {
                switch ( rwFormat ) {
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
        switch ( format ) {
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
return RHEngine::RHImageBufferFormat::BC4;
case MAKEFOURCC( 'A', 'T', 'I', '2' ):
return RHEngine::RHImageBufferFormat::BC5;
case MAKEFOURCC( 'B', 'C', '6', 'H' ):
return RHEngine::RHImageBufferFormat::BC6H;
case MAKEFOURCC( 'D', 'X', 'T', '7' ):
return RHEngine::RHImageBufferFormat::BC7;
#pragma warning(pop)*/
        default:
            break;
        }
        break;
    default:
        break;
    }
    return rh::engine::ImageBufferFormat::Unknown;
}

rh::engine::SamplerAddressing rh::rw::engine::RwTextureAddressModeToRHSamplerAddressing(
    RwTextureAddressMode mode )
{
    switch ( mode ) {
    case RwTextureAddressMode::rwTEXTUREADDRESSBORDER:
        return rh::engine::SamplerAddressing::Border;
    case RwTextureAddressMode::rwTEXTUREADDRESSCLAMP:
        return rh::engine::SamplerAddressing::Clamp;
    case RwTextureAddressMode::rwTEXTUREADDRESSMIRROR:
        return rh::engine::SamplerAddressing::Mirror;
    case RwTextureAddressMode::rwTEXTUREADDRESSWRAP:
        return rh::engine::SamplerAddressing::Wrap;
    default:
        return rh::engine::SamplerAddressing::Unknown;
    }
}

rh::engine::BlendOp rh::rw::engine::RwBlendFunctionToRHBlendOp( RwBlendFunction func )
{
    switch ( func ) {
    case rwBLENDZERO:
        return rh::engine::BlendOp::Zero;
    case rwBLENDONE:
        return rh::engine::BlendOp::One;
    case rwBLENDSRCCOLOR:
        return rh::engine::BlendOp::SrcColor;
    case rwBLENDINVSRCCOLOR:
        return rh::engine::BlendOp::InvSrcColor;
    case rwBLENDSRCALPHA:
        return rh::engine::BlendOp::SrcAlpha;
    case rwBLENDINVSRCALPHA:
        return rh::engine::BlendOp::InvSrcAlpha;
    case rwBLENDDESTALPHA:
        return rh::engine::BlendOp::DestAlpha;
    case rwBLENDINVDESTALPHA:
        return rh::engine::BlendOp::InvDestAlpha;
    case rwBLENDDESTCOLOR:
        return rh::engine::BlendOp::DestColor;
    case rwBLENDINVDESTCOLOR:
        return rh::engine::BlendOp::InvDestColor;
    case rwBLENDSRCALPHASAT:
        return rh::engine::BlendOp::SrcAlphaSat;
    default:
        break;
    }
    return rh::engine::BlendOp();
}

rh::engine::CullMode rh::rw::engine::RwCullModeToRHCullMode( RwCullMode mode )
{
    switch ( mode ) {
    case rwCULLMODECULLNONE:
        return rh::engine::CullMode::None;
    case rwCULLMODECULLBACK:
        return rh::engine::CullMode::Back;
    case rwCULLMODECULLFRONT:
        return rh::engine::CullMode::Front;
    default:
        break;
    }
    return rh::engine::CullMode::None;
}
