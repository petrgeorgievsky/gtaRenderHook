//
// Created by peter on 16.02.2021.
//
#include "im_state.h"
#include <common_headers.h>
#include <rw_engine/rh_backend/raster_backend.h>
#include <rw_engine/rw_rh_convert_funcs.h>

namespace rh::rw::engine
{

ImmediateState::ImmediateState() { Raster = BackendRasterPlugin::NullRasterId; }

void ImmediateState::Update( int32_t nState, void *pParam )
{
    switch ( nState )
    {
    case rwRENDERSTATENARENDERSTATE: break;
    case rwRENDERSTATETEXTURERASTER:
    {
        if ( pParam == nullptr )
        {
            Raster = BackendRasterPlugin::NullRasterId;
            return;
        }
        auto &backend_ext =
            BackendRasterPlugin::GetData( static_cast<RwRaster *>( pParam ) );
        Raster = backend_ext.mImageId;
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