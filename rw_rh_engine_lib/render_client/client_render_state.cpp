//
// Created by peter on 10.02.2021.
//

#include "client_render_state.h"
#include <rw_engine/rw_rh_convert_funcs.h>
namespace rh::rw::engine
{
void ImmediateState::Update( RwRenderState nState, void *pParam )
{
    switch ( nState )
    {
    case rwRENDERSTATENARENDERSTATE: break;
    case rwRENDERSTATETEXTURERASTER:
    {
        if ( pParam == nullptr )
        {
            Raster = gNullRasterId;
            return;
        }
        auto *backend_ext =
            GetBackendRasterExt( static_cast<RwRaster *>( pParam ) );
        Raster = backend_ext->mImageId;
        // weird hack to fix GTA 3 blend in menus
        // TODO: check for image alpha before doing this
        BlendEnable = true;
        break;
    }
    case rwRENDERSTATETEXTUREADDRESS: break;
    case rwRENDERSTATETEXTUREADDRESSU: break;
    case rwRENDERSTATETEXTUREADDRESSV: break;
    case rwRENDERSTATETEXTUREPERSPECTIVE: break;
    case rwRENDERSTATEZTESTENABLE:
    {
        ZTestEnable = pParam != nullptr;
        break;
    }
    case rwRENDERSTATESHADEMODE: break;
    case rwRENDERSTATEZWRITEENABLE:
    {
        ZWriteEnable = pParam != nullptr;
        break;
    }
    case rwRENDERSTATETEXTUREFILTER: break;
    case rwRENDERSTATESRCBLEND:
    {
        ColorBlendSrc = static_cast<uint8_t>(
            RwBlendFunctionToRHBlendOp( static_cast<RwBlendFunction>(
                reinterpret_cast<uint32_t>( pParam ) ) ) );
        break;
    }
    case rwRENDERSTATEDESTBLEND:
    {
        ColorBlendDst = static_cast<uint8_t>(
            RwBlendFunctionToRHBlendOp( static_cast<RwBlendFunction>(
                reinterpret_cast<uint32_t>( pParam ) ) ) );
        break;
    }
    case rwRENDERSTATEVERTEXALPHAENABLE:
    {
        BlendEnable = pParam != nullptr;
        break;
    }
    case rwRENDERSTATEBORDERCOLOR: break;
    case rwRENDERSTATEFOGENABLE: break;
    case rwRENDERSTATEFOGCOLOR: break;
    case rwRENDERSTATEFOGTYPE: break;
    case rwRENDERSTATEFOGDENSITY: break;
    case rwRENDERSTATECULLMODE: break;
    case rwRENDERSTATESTENCILENABLE: break;
    case rwRENDERSTATESTENCILFAIL: break;
    case rwRENDERSTATESTENCILZFAIL: break;
    case rwRENDERSTATESTENCILPASS: break;
    case rwRENDERSTATESTENCILFUNCTION: break;
    case rwRENDERSTATESTENCILFUNCTIONREF: break;
    case rwRENDERSTATESTENCILFUNCTIONMASK: break;
    case rwRENDERSTATESTENCILFUNCTIONWRITEMASK: break;
    case rwRENDERSTATEALPHATESTFUNCTION: break;
    case rwRENDERSTATEALPHATESTFUNCTIONREF: break;
    }
}
} // namespace rh::rw::engine