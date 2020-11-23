#include "RwRenderEngine.h"
#include "rw_engine/global_definitions.h"
#include "rw_engine/rw_api_injectors.h"
#include "rw_engine/rw_rh_convert_funcs.h"
#include "rw_engine/rw_standard_render_commands/camerabeginupdatecmd.h"
#include "rw_engine/rw_standard_render_commands/cameraclearcmd.h"
#include "rw_engine/rw_standard_render_commands/cameraendupdatecmd.h"
#include "rw_engine/rw_standard_render_commands/imagefindrasterformat.h"
#include "rw_engine/rw_standard_render_commands/nativetexturereadcmd.h"
#include "rw_engine/rw_standard_render_commands/rastercreatecmd.h"
#include "rw_engine/rw_standard_render_commands/rasterdestroycmd.h"
#include "rw_engine/rw_standard_render_commands/rasterlockcmd.h"
#include "rw_engine/rw_standard_render_commands/rastersetimagecmd.h"
#include "rw_engine/rw_standard_render_commands/rastershowrastercmd.h"
#include "rw_engine/rw_standard_render_commands/rasterunlockcmd.h"

#include <DebugUtils/DebugLogger.h>

using namespace rh::rw::engine;

std::unique_ptr<RwRenderEngine> rh::rw::engine::g_pRwRenderEngine;
bool rh::rw::engine::g_bRwSwapchainChanged = false;
CameraContext *rh::rw::engine::g_cameraContext = new CameraContext();
std::unique_ptr<rh::engine::IRenderer> rh::engine::g_pRHRenderer;

std::vector<RwRenderEngineGenericCB> RwRenderEngine::mPostInitCallbacks = {};
std::vector<RwRenderEngineGenericCB> RwRenderEngine::mPreShutdownCallbacks = {};
std::vector<RwRenderEngineOutputResizeCB> RwRenderEngine::mOutputResizeCallbacks = {};

bool RwRenderEngine::Focus( bool )
{
    return false;
}

bool RwRenderEngine::GetMode( int &n )
{
    return rh::engine::g_pRHRenderer->GetDeviceState().GetCurrentDisplayMode(
        reinterpret_cast<unsigned int &>( n ) );
}

bool RwRenderEngine::Standards( int *standards, int numStandardsFunctions )
{
    RwInt32 i;
    RwInt32 numDriverFunctions;
    RwStandardFunc *standardFunctions;
    RwStandard rwStandards[] = {

        /* Camera ops */
        {rwSTANDARDCAMERABEGINUPDATE,
         []( void *, void *pInOut, RwInt32 ) -> RwBool {
             RwCameraBeginUpdateCmd begin_update_cmd( static_cast<RwCamera *>( pInOut ) );
             return begin_update_cmd.Execute();
         }},
        {rwSTANDARDCAMERAENDUPDATE,
         []( void *, void *pInOut, RwInt32 ) -> RwBool {
             RwCameraEndUpdateCmd end_update_cmd( static_cast<RwCamera *>( pInOut ) );
             return end_update_cmd.Execute();
         }},
        {rwSTANDARDCAMERACLEAR,
         []( void *pOut, void *pInOut, RwInt32 nI ) -> RwBool {
             RwCameraClearCmd clear_cmd( static_cast<RwCamera *>( pOut ),
                                         static_cast<RwRGBA *>( pInOut ),
                                         nI );
             return clear_cmd.Execute();
         }},
        /* Raster/Pixel operations */
        {rwSTANDARDRASTERSHOWRASTER,
         []( void *pOut, void *, RwInt32 nI ) -> RwBool {
             RwRasterShowRasterCmd show_raster_cmd( static_cast<RwRaster *>( pOut ), nI );
             return show_raster_cmd.Execute();
         }},
        {rwSTANDARDRGBTOPIXEL,
         []( void *, void *, RwInt32 ) -> RwBool {
             rh::debug::DebugLogger::Log( "RGBTOPIXEL call" );
             return true;
         }},
        {rwSTANDARDPIXELTORGB,
         []( void *, void *, RwInt32 ) -> RwBool {
             rh::debug::DebugLogger::Log( "PIXELTORGB call" );
             return true;
         }},
        {rwSTANDARDRASTERSETIMAGE,
         []( void *pOut, void *pInOut, RwInt32 ) -> RwBool {
             RwRasterSetImageCmd set_img_cmd( static_cast<RwRaster *>( pOut ),
                                              static_cast<RwImage *>( pInOut ) );
             return set_img_cmd.Execute();
         }},
        {rwSTANDARDIMAGEGETRASTER,
         []( void *, void *, RwInt32 ) -> RwBool {
             rh::debug::DebugLogger::Log( "IMAGEGETRASTER call" );
             return true;
         }},

        /* Raster creation and destruction */
        {rwSTANDARDRASTERDESTROY,
         []( void *, void *pInOut, RwInt32 ) -> RwBool {
             RwRasterDestroyCmd destroy_cmd( static_cast<RwRaster *>( pInOut ) );
             return destroy_cmd.Execute();
         }},
        {rwSTANDARDRASTERCREATE,
         []( void *, void *pInOut, RwInt32 nI ) -> RwBool {
             RwRasterCreateCmd create_cmd( static_cast<RwRaster *>( pInOut ),
                                           static_cast<uint32_t>( nI ) );
             return create_cmd.Execute();
         }},

        /* Finding about a raster type */
        {rwSTANDARDIMAGEFINDRASTERFORMAT,
         []( void *pOut, void *pInOut, RwInt32 nI ) -> RwBool {
             RwImageFindRasterFormatCmd find_raster_cmd( static_cast<RwRaster *>( pOut ),
                                                         static_cast<RwImage *>( pInOut ),
                                                         static_cast<uint32_t>( nI ) );
             return find_raster_cmd.Execute();
         }},

        /* Texture operations */
        {rwSTANDARDTEXTURESETRASTER,
         []( void *, void *, RwInt32 ) -> RwBool {
             rh::debug::DebugLogger::Log( "TEXTURESETRASTER call" );
             return true;
         }},

        /* Locking and releasing */
        {rwSTANDARDRASTERLOCK,
         []( void *pOut, void *pInOut, RwInt32 nI ) -> RwBool {
             // RHDebug::DebugLogger::Log( "RASTERLOCK call" );
             RwRasterLockCmd lock_cmd( static_cast<RwRaster *>( pInOut ), nI );
             void *&pOutData = *static_cast<void **>( pOut );
             return lock_cmd.Execute( pOutData );
         }},
        {rwSTANDARDRASTERUNLOCK,
         []( void *, void *pInOut, RwInt32 ) -> RwBool {
             // RHDebug::DebugLogger::Log( "RASTERUNLOCK call" );
             RwRasterUnlockCmd unlock_cmd( static_cast<RwRaster *>( pInOut ) );
             return unlock_cmd.Execute();
         }},
        {rwSTANDARDRASTERLOCKPALETTE,
         []( void *, void *, RwInt32 ) -> RwBool {
             rh::debug::DebugLogger::Log( "RASTERLOCKPALETTE call" );
             return true;
         }},
        {rwSTANDARDRASTERUNLOCKPALETTE,
         []( void *, void *, RwInt32 ) -> RwBool {
             rh::debug::DebugLogger::Log( "RASTERUNLOCKPALETTE call" );
             return true;
         }},

        /* Raster operations */
        {rwSTANDARDRASTERCLEAR,
         []( void *, void *, RwInt32 ) -> RwBool {
             rh::debug::DebugLogger::Log( "RASTERCLEAR call" );
             return true;
         }},
        {rwSTANDARDRASTERCLEARRECT,
         []( void *, void *, RwInt32 ) -> RwBool {
             rh::debug::DebugLogger::Log( "RASTERCLEARRECT call" );
             return true;
         }},

        /* !! */
        {rwSTANDARDRASTERRENDER,
         []( void *, void *, RwInt32 ) -> RwBool {
             rh::debug::DebugLogger::Log( "RASTERRENDER call" );
             return true;
         }},
        {rwSTANDARDRASTERRENDERSCALED,
         []( void *, void *, RwInt32 ) -> RwBool {
             rh::debug::DebugLogger::Log( "RASTERRENDERSCALED call" );
             return true;
         }},
        {rwSTANDARDRASTERRENDERFAST,
         []( void *, void *, RwInt32 ) -> RwBool {
             rh::debug::DebugLogger::Log( "RASTERRENDERFAST call" );
             return true;
         }},

        /* Setting the context */
        /*{ rwSTANDARDSETRASTERCONTEXT, _rwD3D9SetRasterContext },*/

        /* Creating sub rasters */
        /*{ rwSTANDARDRASTERSUBRASTER, _rwD3D9RasterSubRaster },*/

        /* Hint for rendering order */
        {rwSTANDARDHINTRENDERF2B,
         []( void *, void *, RwInt32 ) -> RwBool {
             // RHDebug::DebugLogger::Log( "HINTRENDERF2B call" );
             return true;
         }},

        /* Native texture serialization */
        {rwSTANDARDNATIVETEXTUREGETSIZE,
         []( void *, void *, RwInt32 ) -> RwBool {
             rh::debug::DebugLogger::Log( "NATIVETEXTUREGETSIZE call" );
             return true;
         }},
        {rwSTANDARDNATIVETEXTUREWRITE,
         []( void *, void *, RwInt32 ) -> RwBool {
             rh::debug::DebugLogger::Log( "NATIVETEXTUREWRITE call" );
             return true;
         }},
        {rwSTANDARDNATIVETEXTUREREAD,
         []( void *pOut, void *pInOut, RwInt32 ) -> RwBool {
             RwNativeTextureReadCmd tex_read_cmd( static_cast<RwStream *>( pOut ),
                                                  static_cast<RwTexture **>( pInOut ) );
             return tex_read_cmd.Execute();
         }},

        /* Raster Mip Levels */
        /*{ rwSTANDARDRASTERGETMIPLEVELS, _rwD3D9RasterGetMipLevels }*/
    };

    // RWFUNCTION(RWSTRING("D3D9DeviceSystemStandards"));

    standardFunctions = reinterpret_cast<RwStandardFunc *>( standards );
    numDriverFunctions = sizeof( rwStandards ) / sizeof( RwStandard );

    /* Clear out all of the standards initially */
    for ( i = 0; i < numStandardsFunctions; i++ ) {
        standardFunctions[i] = []( void *, void *, RwInt32 ) -> RwBool {
            rh::debug::DebugLogger::Log( "RwStandardFnc call" );
            return true;
        };
    }

    /* Fill in all of the standards */
    while ( numDriverFunctions-- ) {
        if ( ( rwStandards->nStandard < numStandardsFunctions )
             && ( rwStandards->nStandard >= 0 ) ) {
            standardFunctions[rwStandards[numDriverFunctions].nStandard]
                = rwStandards[numDriverFunctions].fpStandard;
        }
    }
    return true;
}

bool RwRenderEngine::GetTexMemSize( int & )
{
    return false;
}

bool RwRenderEngine::GetNumSubSystems( unsigned int &num )
{
    return rh::engine::g_pRHRenderer->GetDeviceState().GetAdaptersCount( num );
}

bool RwRenderEngine::GetSubSystemInfo( RwSubSystemInfo &info, unsigned int n )
{
    rh::engine::String str;

    if ( !rh::engine::g_pRHRenderer->GetDeviceState().GetAdapterInfo( n, str ) )
        return false;

    strncpy_s( info.name, FromRHString( str ).c_str(), 80 );

    return true;
}

bool RwRenderEngine::GetCurrentSubSystem( int & )
{
    return false;
}

bool RwRenderEngine::SetSubSystem( unsigned int n )
{
    return rh::engine::g_pRHRenderer->GetDeviceState().SetCurrentAdapter( n );
}

bool RwRenderEngine::GetNumOutputs( unsigned int a, int &n )
{
    return rh::engine::g_pRHRenderer->GetDeviceState()
        .GetOutputCount( a, reinterpret_cast<unsigned int &>( n ) );
}

bool RwRenderEngine::GetOutputInfo( std::string &info, unsigned int n )
{
    return rh::engine::g_pRHRenderer->GetDeviceState().GetOutputInfo( n, info );
}

bool RwRenderEngine::GetCurrentOutput( int &n )
{
    return rh::engine::g_pRHRenderer->GetDeviceState().GetCurrentOutput(
        reinterpret_cast<unsigned int &>( n ) );
}

bool RwRenderEngine::SetOutput( unsigned int n )
{
    return rh::engine::g_pRHRenderer->GetDeviceState().SetCurrentOutput( n );
}

bool RwRenderEngine::GetMaxTextureSize( int & )
{
    return false;
}

bool RwRenderEngine::BaseEventHandler( int, int *, void *, int )
{
    return false;
}

void RwRenderEngine::SetMultiSamplingLevels( int ) {}

int RwRenderEngine::GetMaxMultiSamplingLevels()
{
    return 0;
}

bool RwRenderEngine::RenderStateSet( RwRenderState state, UINT value )
{
    auto *rendering_context = static_cast<rh::engine::D3D11RenderingContext *>(
        rh::engine::g_pRHRenderer->GetCurrentContext() );
    /**/
    switch ( state ) {
    case rwRENDERSTATETEXTURERASTER: {
        auto *raster = reinterpret_cast<RwRaster *>( value );
        if ( raster ) {
            m_p2DRenderer->BindTexture( GetInternalRaster( raster ) );
        } else
            m_p2DRenderer->BindTexture( nullptr );
        break;
    }
    case rwRENDERSTATETEXTUREADDRESS:

        break;
    case rwRENDERSTATETEXTUREADDRESSU:
        break;
    case rwRENDERSTATETEXTUREADDRESSV:
        break;
    case rwRENDERSTATETEXTUREPERSPECTIVE:
        break;
    case rwRENDERSTATEZTESTENABLE:
        rendering_context->GetStateCache()->GetDepthStencilStateCache()->SetDepthEnable( value
                                                                                         != 0 );
        break;
    case rwRENDERSTATESHADEMODE:
        break;
    case rwRENDERSTATEZWRITEENABLE:
        rendering_context->GetStateCache()->GetDepthStencilStateCache()->SetDepthWriteEnable(
            value != 0 );
        break;
    case rwRENDERSTATETEXTUREFILTER:
        break;
    case rwRENDERSTATESRCBLEND:
        rendering_context->GetStateCache()
            ->GetBlendStateCache()
            ->SetSrcBlendOp( RwBlendFunctionToRHBlendOp( static_cast<RwBlendFunction>( value ) ),
                             0 );
        break;
    case rwRENDERSTATEDESTBLEND:
        rendering_context->GetStateCache()
            ->GetBlendStateCache()
            ->SetDestBlendOp( RwBlendFunctionToRHBlendOp( static_cast<RwBlendFunction>( value ) ),
                              0 );
        break;
    case rwRENDERSTATEVERTEXALPHAENABLE:
        rendering_context->GetStateCache()->GetBlendStateCache()->SetBlendEnable( value != 0, 0 );
        break;
    case rwRENDERSTATEBORDERCOLOR:
        break;
    case rwRENDERSTATEFOGENABLE:
        break;
    case rwRENDERSTATEFOGCOLOR:
        break;
    case rwRENDERSTATEFOGTYPE:
        break;
    case rwRENDERSTATEFOGDENSITY:
        break;
    case rwRENDERSTATECULLMODE:
        rendering_context->GetStateCache()->GetRasterizerStateCache()->SetCullMode(
            RwCullModeToRHCullMode( static_cast<RwCullMode>( value ) ) );
        break;
    case rwRENDERSTATESTENCILENABLE:
        break;
    case rwRENDERSTATESTENCILFAIL:
        break;
    case rwRENDERSTATESTENCILZFAIL:
        break;
    case rwRENDERSTATESTENCILPASS:
        break;
    case rwRENDERSTATESTENCILFUNCTION:
        break;
    case rwRENDERSTATESTENCILFUNCTIONREF:
        break;
    case rwRENDERSTATESTENCILFUNCTIONMASK:
        break;
    case rwRENDERSTATESTENCILFUNCTIONWRITEMASK:
        break;
    case rwRENDERSTATEALPHATESTFUNCTION:
        rendering_context->GetStateCache()->GetBlendStateCache()->SetAlphaTestEnable( value != 0 );
        break;
    case rwRENDERSTATEALPHATESTFUNCTIONREF:
        break;
    default:
        break;
    }
    return true;
}

bool RwRenderEngine::RenderStateGet( RwRenderState, UINT & )
{
    return false;
}

bool RwRenderEngine::RasterLock( RwRaster * /*raster*/, UINT /*flags*/, void ** /*data*/ )
{
    return false;
}

bool RwRenderEngine::RasterUnlock( RwRaster * /*raster*/ )
{
    return false;
}

bool RwRenderEngine::Im2DRenderPrimitive( RwPrimitiveType primType,
                                          RwIm2DVertex *vertices,
                                          RwUInt32 numVertices )
{
    m_p2DRenderer->Draw( rh::engine::g_pRHRenderer->GetCurrentContext(),
                         static_cast<rh::engine::PrimitiveType>( primType ),
                         reinterpret_cast<rh::engine::Simple2DVertex *>( vertices ),
                         numVertices );

    return true;
}

bool RwRenderEngine::Im2DRenderIndexedPrimitive( RwPrimitiveType primType,
                                                 RwIm2DVertex *vertices,
                                                 RwUInt32 numVertices,
                                                 RwImVertexIndex *indices,
                                                 RwInt32 numIndices )
{
    m_p2DRenderer->DrawIndexed( rh::engine::g_pRHRenderer->GetCurrentContext(),
                                static_cast<rh::engine::PrimitiveType>( primType ),
                                reinterpret_cast<rh::engine::Simple2DVertex *>( vertices ),
                                numVertices,
                                reinterpret_cast<RwImVertexIndex *>( indices ),
                                static_cast<uint32_t>( numIndices ) );
    return false;
}

RwBool RwRenderEngine::Im3DSubmitNode()
{
    return RwBool();
}

void RwRenderEngine::SetTexture( RwTexture * /*tex*/, int /*Stage*/ ) {}

bool RwRenderEngine::AtomicAllInOneNode( RxPipelineNode * /*self*/,
                                         const RxPipelineNodeParam * /*params*/ )
{
    return false;
}

bool RwRenderEngine::SkinAllInOneNode( RxPipelineNode * /*self*/,
                                       const RxPipelineNodeParam * /*params*/ )
{
    return false;
}

void RwRenderEngine::DefaultRenderCallback( RwResEntry * /*repEntry*/,
                                            void * /*object*/,
                                            RwUInt8 /*type*/,
                                            RwUInt32 /*flags*/ )
{}

RwBool RwRenderEngine::DefaultInstanceCallback( void * /*object*/,
                                                RxD3D9ResEntryHeader * /*resEntryHeader*/,
                                                RwBool /*reinstance*/ )
{
    return RwBool();
}

bool RwRenderEngine::EventHandlingSystem( RwRenderSystemRequest request,
                                          int *pOut,
                                          void *pInOut,
                                          int nIn )
{
    switch ( request ) {
    case RwRenderSystemRequest::rwDEVICESYSTEMOPEN:
        return Open( *static_cast<HWND *>( pInOut ) );
    case RwRenderSystemRequest::rwDEVICESYSTEMCLOSE:
        return Close();
    case RwRenderSystemRequest::rwDEVICESYSTEMSTART:
        return Start();
    case RwRenderSystemRequest::rwDEVICESYSTEMSTOP:
        return Stop();
    case RwRenderSystemRequest::rwDEVICESYSTEMGETNUMMODES:
        return GetNumModes( *pOut );
    case RwRenderSystemRequest::rwDEVICESYSTEMGETMODEINFO:
        return GetModeInfo( *reinterpret_cast<RwVideoMode *>( pOut ), nIn );
    case RwRenderSystemRequest::rwDEVICESYSTEMUSEMODE:
        return UseMode( static_cast<unsigned int>( nIn ) );
    case RwRenderSystemRequest::rwDEVICESYSTEMFOCUS:
        return Focus( nIn != 0 );
    case RwRenderSystemRequest::rwDEVICESYSTEMGETMODE:
        return GetMode( *pOut );
    case RwRenderSystemRequest::rwDEVICESYSTEMSTANDARDS:
        return Standards( pOut, nIn );
    case RwRenderSystemRequest::rwDEVICESYSTEMGETNUMSUBSYSTEMS:
        return GetNumSubSystems( *reinterpret_cast<unsigned int *>( pOut ) );
    case RwRenderSystemRequest::rwDEVICESYSTEMGETSUBSYSTEMINFO:
        return GetSubSystemInfo( *reinterpret_cast<RwSubSystemInfo *>( pOut ),
                                 static_cast<RwUInt32>( nIn ) );
    case RwRenderSystemRequest::rwDEVICESYSTEMGETCURRENTSUBSYSTEM:
        return GetCurrentSubSystem( *pOut );
    case RwRenderSystemRequest::rwDEVICESYSTEMSETSUBSYSTEM:
        return SetSubSystem( static_cast<RwUInt32>( nIn ) );
    case RwRenderSystemRequest::rwDEVICESYSTEMGETTEXMEMSIZE:
        return GetTexMemSize( *pOut );
    case RwRenderSystemRequest::rwDEVICESYSTEMREGISTER:
    case RwRenderSystemRequest::rwDEVICESYSTEMINITPIPELINE:
    case RwRenderSystemRequest::rwDEVICESYSTEMFINALIZESTART:
    case RwRenderSystemRequest::rwDEVICESYSTEMINITIATESTOP:
    case RwRenderSystemRequest::rwDEVICESYSTEMRXPIPELINEREQUESTPIPE:
    case RwRenderSystemRequest::rwDEVICESYSTEMGETMETRICBLOCK:
        break;
    case RwRenderSystemRequest::rwDEVICESYSTEMGETMAXTEXTURESIZE:
        return GetMaxTextureSize( *pOut );
    case RwRenderSystemRequest::rwDEVICESYSTEMGETID:
        *pOut = 2;
        return true;
    }

    return g_pGlobal_API.fpDefaultSystem( static_cast<int>( request ), pOut, pInOut, nIn );
}

void RwRenderEngine::RegisterPostInitCallback( RwRenderEngineGenericCB cb )
{
    mPostInitCallbacks.emplace_back( cb );
}

void RwRenderEngine::RegisterPreShutdownCallback( RwRenderEngineGenericCB cb )
{
    mPreShutdownCallbacks.emplace_back( cb );
}

void RwRenderEngine::RegisterOutputResizeCallback( RwRenderEngineOutputResizeCB cb )
{
    mOutputResizeCallbacks.emplace_back( cb );
}

bool RwRenderEngine::Open( HWND window )
{
    HINSTANCE hInst = GetModuleHandle( nullptr );

    rh::engine::g_pRHRenderer = rh::engine::RendererFactory::CreateRenderer( m_renderingAPI,
                                                                             window,
                                                                             hInst );

    return rh::engine::g_pRHRenderer != nullptr;
}

bool RwRenderEngine::Close()
{
    rh::engine::g_pRHRenderer.reset();

    return true;
}

bool RwRenderEngine::Start()
{
    if ( !rh::engine::g_pRHRenderer->InitDevice() )
        return false;

    m_p2DRenderer = rh::engine::RendererFactory::CreateSimple2DRenderer( m_renderingAPI );
    for ( auto cb : mPostInitCallbacks )
        cb();
    return true;
}

bool RwRenderEngine::Stop()
{
    m_p2DRenderer = nullptr;
    for ( auto cb : mPreShutdownCallbacks )
        cb();

    rh::engine::g_pRHRenderer->ShutdownDevice();

    return true;
}

bool RwRenderEngine::GetNumModes( int &n )
{
    return rh::engine::g_pRHRenderer->GetDeviceState()
               .GetDisplayModeCount( 0, reinterpret_cast<unsigned int &>( n ) )
           && rh::engine::g_pRHRenderer->GetDeviceState().SetCurrentOutput( 0 );
}

bool RwRenderEngine::GetModeInfo( RwVideoMode &videomode, int n )
{
    rh::engine::DisplayModeInfo info{};

    if ( !rh::engine::g_pRHRenderer->GetDeviceState().GetDisplayModeInfo( static_cast<unsigned int>(
                                                                              n ),
                                                                          info ) )
        return false;

    videomode.width = static_cast<int>( info.width );
    videomode.height = static_cast<int>( info.height );
    videomode.depth = 32;
    videomode.refRate = static_cast<int>( info.refreshRate );
    videomode.format = rwRASTERFORMATDEFAULT;
    videomode.flags = rwVIDEOMODEEXCLUSIVE;
    return true;
}

bool RwRenderEngine::UseMode( unsigned int n )
{
    rh::engine::g_pRHRenderer->GetDeviceState().SetCurrentOutput( 0 );
    rh::engine::DisplayModeInfo display_info;
    rh::engine::g_pRHRenderer->GetDeviceState().GetDisplayModeInfo( n, display_info );
    if ( !rh::engine::g_pRHRenderer->GetDeviceState().SetCurrentDisplayMode( n ) )
        return false;
    for ( auto cb : mOutputResizeCallbacks )
        cb( display_info.width, display_info.height );
    g_bRwSwapchainChanged = true;
    return true;
}
