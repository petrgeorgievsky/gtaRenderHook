#include "idle_hook.h"
#include <Engine/D3D11Impl/Buffers/D3D11ConstantBuffer.h>
#include <Engine/D3D11Impl/D3D11PrimitiveBatch.h>
#include <MemoryInjectionUtils/InjectorHelpers.h>
#include <RwRenderEngine.h>
#include <common.h>
#include <gbuffer/gbuffer_pipeline.h>
#include <rw_engine/global_definitions.h>
#include <rw_engine/rw_api_injectors.h>
#include <rw_engine/rw_im2d/rw_im2d.h>
#include <rw_engine/rw_macro_constexpr.h>
#include <rw_engine/rw_rh_pipeline.h>
#include <rw_game_hooks.h>

using namespace rh::engine;
using namespace rh::rw::engine;

bool true_ret_hook()
{
    return true;
}
bool false_ret_hook()
{
    return false;
}

#define RwFrameGetLTM( frame ) \
    ( reinterpret_cast<RwMatrix *(__cdecl *) ( RwFrame * )>( 0x7F0990 ) )( frame )

void DrawEntity( void *entity_ptr )
{
    ( reinterpret_cast<void( __cdecl * )( void * )>( 0x553260 ) )( entity_ptr );
}

static RpGeometryRw36 geometry_interface_35{};
static RwBool D3D8AtomicAllInOneNode( RxPipelineNodeInstance * /*self*/,
                                      const RxPipelineNodeParam *params )
{
    RpAtomic *atomic;

    atomic = static_cast<RpAtomic *>( params->dataParam );
    RpGeometry *geom = atomic->geometry;
    //RpMeshHeader *meshHeader = geom->mesh;

    geometry_interface_35.Init( geom );
    if ( RwRHInstanceAtomic( atomic, &geometry_interface_35 ) != RenderStatus::Instanced )
        return false;

    //IRenderingContext *context = static_cast<IRenderingContext *>(
    //    g_pRHRenderer->GetCurrentContext() );

    const RwMatrix *ltm = RwFrameGetLTM( static_cast<RwFrame *>( rwObject::GetParent( atomic ) ) );
    DirectX::XMMATRIX objTransformMatrix = {ltm->right.x,
                                            ltm->right.y,
                                            ltm->right.z,
                                            0,
                                            ltm->up.x,
                                            ltm->up.y,
                                            ltm->up.z,
                                            0,
                                            ltm->at.x,
                                            ltm->at.y,
                                            ltm->at.z,
                                            0,
                                            ltm->pos.x,
                                            ltm->pos.y,
                                            ltm->pos.z,
                                            1};
    struct PerModelCB
    {
        DirectX::XMMATRIX worldMatrix;
        DirectX::XMMATRIX worldITMatrix;
    } perModelCB;
    perModelCB.worldMatrix = objTransformMatrix;
    perModelCB.worldITMatrix = DirectX::XMMatrixInverse( nullptr, objTransformMatrix );
    // g_pRwRenderEngine->RenderStateSet( rwRENDERSTATEVERTEXALPHAENABLE, 1 );

    /*context->UpdateBuffer( gBaseConstantBuffer, g_cameraContext, sizeof(
*g_cameraContext ) ); context->UpdateBuffer( gPerModelConstantBuffer,
&perModelCB, sizeof( perModelCB ) ); context->BindConstantBuffers(
RHEngine::RHShaderStage::Vertex | RHEngine::RHShaderStage::Pixel |
RHEngine::RHShaderStage::Compute,
                  {{0, gBaseConstantBuffer}, {1,
gPerModelConstantBuffer}} );

DrawAtomic( atomic, &geometry_interface_35, context, g_gbPipeline );*/

    return true;
}
static RwBool rxD3D8SubmitNode( RxPipelineNodeInstance *, const RxPipelineNodeParam * )
{
    return true;
}

void empty_void() {}
void *rwD3D9RasterDtor( void *object )
{
    return object;
}

BOOL APIENTRY DllMain( HMODULE /*hModule*/, DWORD ul_reason_for_call, LPVOID /*lpReserved*/ )
{
    switch ( ul_reason_for_call ) {
    case DLL_PROCESS_ATTACH:
        g_pIO_API = {reinterpret_cast<RwStreamFindChunk_FN>( 0x7ED2D0 ),
                     reinterpret_cast<RwStreamRead_FN>( 0x7EC9D0 )};
        g_pGlobal_API = {reinterpret_cast<RwSystemFunc>( 0x7F5F70 ),
                         reinterpret_cast<uint32_t *>( 0xB4E9E0 ),
                         reinterpret_cast<RwResourcesAllocateResEntry_FN>( 0x807ED0 )};
        g_pRaster_API = {reinterpret_cast<RwRasterCreate_FN>( 0x7FB230 ),
                         reinterpret_cast<RwRasterDestroy_FN>( 0x7FB020 )};
        g_pTexture_API = {reinterpret_cast<RwTextureCreate_FN>( 0x7F37C0 ),
                          reinterpret_cast<RwTextureSetName_FN>( 0x7F38A0 ),
                          reinterpret_cast<RwTextureSetName_FN>( 0x7F3910 )};
        rh::debug::DebugLogger::Init( "gta_rt_rh_logs.log", rh::debug::LogLevel::Info );
        g_pRwRenderEngine = std::unique_ptr<RwRenderEngine>(
            new RwRenderEngine( RenderingAPI::DX11 ) );

        {
            RwPointerTable gtasa_ptr_table{};
            gtasa_ptr_table.m_fpRenderSystem = 0x8E249C;
            gtasa_ptr_table.m_fpIm3DOpen = 0x80A225; // 0x7EFE20;
            gtasa_ptr_table.m_fpCheckNativeTextureSupport = 0x745530;
            gtasa_ptr_table.m_fpCheckEnviromentMapSupport = 0x5D8980;
            gtasa_ptr_table.m_fpSetVideoMode = 0x7F8640;
            gtasa_ptr_table.m_fpSetRefreshRate = 0x7F8580;
            gtasa_ptr_table.m_fpSetRenderState = 0x8E24A8;
            gtasa_ptr_table.m_fpGetRenderState = 0x8E24AC;
            gtasa_ptr_table.m_fpIm2DRenderLine = 0x8E24B0;
            gtasa_ptr_table.m_fpIm2DRenderPrim = 0x8E24B8;
            gtasa_ptr_table.m_fpIm2DRenderIndexedPrim = 0x8E24BC;
            //  pipeline ptr
            RwGameHooks::Patch( gtasa_ptr_table );
        }

        IdleHook::Patch();

        {
            RedirectJump( 0x4C9A80, reinterpret_cast<void *>( rwD3D9RasterDtor ) );
            RedirectCall( 0x748A30,
                          reinterpret_cast<void *>( empty_void ) ); // CGammaInitialise
            RedirectCall( 0x5BD779,
                          reinterpret_cast<void *>( empty_void ) ); // CPostEffects10Initialise
            RedirectCall( 0x53EA0D,
                          reinterpret_cast<void *>( empty_void ) ); // 2CRealTimeShadowManager6Update
            RedirectCall( 0x53EA12,
                          reinterpret_cast<void *>( empty_void ) ); // CMirrors16BeforeMainRender
            RedirectCall( 0x53EABA,
                          reinterpret_cast<void *>( empty_void ) ); // CMirrors18RenderMirrorBuffer
            RedirectCall( 0x53EAC4,
                          reinterpret_cast<void *>(
                              empty_void ) ); // CVisibilityPlugins21RenderWeaponPedsForPC
            SetPointer( 0x8D633C, reinterpret_cast<void *>( D3D8AtomicAllInOneNode ) );
            SetPointer( 0x8DED0C,
                        reinterpret_cast<void *>( D3D8AtomicAllInOneNode ) ); // skin
            SetPointer( 0x8E297C, reinterpret_cast<void *>( rxD3D8SubmitNode ) );
        }
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}
