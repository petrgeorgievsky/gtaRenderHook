#include "gta_sa_internal_classes/CColorSet.h"
#include "idle_hook.h"
#include <DebugUtils/DebugLogger.h>
#include <MemoryInjectionUtils/InjectorHelpers.h>
#include <algorithm>
#include <ipc/ipc_utils.h>
#include <render_client/render_client.h>
#include <rw_engine/rw_api_injectors.h>
#include <rw_engine/rw_frame/rw_frame.h>
#include <rw_engine/rw_im2d/rw_im2d.h>
#include <rw_engine/rw_macro_constexpr.h>
#include <rw_engine/rw_rh_pipeline.h>
#include <rw_engine/rw_rh_skin_pipeline.h>
#include <rw_game_hooks.h>

using namespace rh::engine;
using namespace rh::rw::engine;

bool true_ret_hook() { return true; }
bool false_ret_hook() { return false; }

static RpGeometryRw36 geometry_interface_35{};

static int32_t D3D8AtomicAllInOneNode( void * /*self*/,
                                       const RxPipelineNodeParam *params )
{
    RpAtomic *atomic;

    atomic           = static_cast<RpAtomic *>( params->dataParam );
    RpGeometry *geom = atomic->geometry;

    geometry_interface_35.Init( geom );
    if ( RwRHInstanceAtomic( atomic, &geometry_interface_35 ) !=
         RenderStatus::Instanced )
        return 0;

    const RwMatrix *ltm =
        rh::rw::engine::RwFrameGetLTM( rwFrame::GetParent( atomic ) );
    rh::rw::engine::DrawAtomic(
        atomic, &geometry_interface_35,
        [&ltm, atomic]( rh::rw::engine::ResEnty *res_entry ) {
            auto &renderer  = gRenderClient->RenderState.MeshDrawCalls;
            auto  mesh_list = geometry_interface_35.GetMeshList();
            auto  materials =
                renderer.AllocateDrawCallMaterials( mesh_list.size() );

            for ( auto i = 0; i < mesh_list.size(); i++ )
                materials[i] = ConvertMaterialData( mesh_list[i].material );
            DrawCallInfo info{};
            info.DrawCallId     = reinterpret_cast<uint64_t>( atomic );
            info.MeshId         = res_entry->meshData;
            info.WorldTransform = DirectX::XMFLOAT4X3{
                ltm->right.x, ltm->up.x, ltm->at.x, ltm->pos.x,
                ltm->right.y, ltm->up.y, ltm->at.y, ltm->pos.y,
                ltm->right.z, ltm->up.z, ltm->at.z, ltm->pos.z,
            };

            renderer.RecordDrawCall( info );
        } );

    return 1;
}

static int32_t D3D8SkinAtomicAllInOneNode( void * /*self*/,
                                           const RxPipelineNodeParam *params )
{
    RpAtomic *atomic;

    atomic           = static_cast<RpAtomic *>( params->dataParam );
    RpGeometry *geom = atomic->geometry;
    // RpMeshHeader *meshHeader = geom->mesh;

    geometry_interface_35.Init( geom );
    if ( RwRHInstanceSkinAtomic( atomic, &geometry_interface_35 ) !=
         RenderStatus::Instanced )
        return 0;

    const RwMatrix *ltm = ::RwFrameGetLTM(
        static_cast<RwFrame *>( rwObject::GetParent( atomic ) ) );
    DrawAtomic(
        atomic, &geometry_interface_35, [&ltm, atomic]( ResEnty *res_entry ) {
            auto &renderer  = gRenderClient->RenderState.SkinMeshDrawCalls;
            auto  mesh_list = geometry_interface_35.GetMeshList();
            auto  materials =
                renderer.AllocateDrawCallMaterials( mesh_list.size() );

            for ( auto i = 0; i < mesh_list.size(); i++ )
                materials[i] = ConvertMaterialData( mesh_list[i].material );

            SkinDrawCallInfo info{};
            info.DrawCallId     = reinterpret_cast<uint64_t>( atomic );
            info.MeshId         = res_entry->meshData;
            info.WorldTransform = DirectX::XMFLOAT4X3{
                ltm->right.x, ltm->up.x, ltm->at.x, ltm->pos.x,
                ltm->right.y, ltm->up.y, ltm->at.y, ltm->pos.y,
                ltm->right.z, ltm->up.z, ltm->at.z, ltm->pos.z,
            };
            static AnimHierarcyRw36 g_anim{};
            PrepareBoneMatrices( info.BoneTransform, atomic, g_anim );
            renderer.RecordDrawCall( info );
        } );
    return 1;
}

void empty_void() {}
struct CVector
{
    float x, y, z;
};

struct CPointLight
{
    CVector         m_vecPosn;
    CVector         m_vecDirection;
    float           m_fRange;
    float           m_fColorRed;
    float           m_fColorGreen;
    float           m_fColorBlue;
    void *          m_pEntityToLight;
    unsigned __int8 m_nType;
    unsigned __int8 m_nFogType;
    char            m_bGenerateShadows;
    char            _pad0;
};

void prepare_timecyc()
{

    auto &sky_state = rh::rw::engine::gRenderClient->RenderState.SkyState;
    auto &cur_cs    = *(CColourSet *)0xB7C4A0;

    auto *vec_to_sun_arr   = (RwV3d *)0xB7CA50;
    int & current_tc_value = *(int *)0xB79FD0;

    sky_state.mSkyTopColor[0]    = float( cur_cs.m_nSkyTopRed ) / 255.0f;
    sky_state.mSkyTopColor[1]    = float( cur_cs.m_nSkyTopGreen ) / 255.0f;
    sky_state.mSkyTopColor[2]    = float( cur_cs.m_nSkyTopBlue ) / 255.0f;
    sky_state.mSkyTopColor[3]    = 1.0f;
    sky_state.mSkyBottomColor[0] = float( cur_cs.m_nSkyBottomRed ) / 255.0f;
    sky_state.mSkyBottomColor[1] = float( cur_cs.m_nSkyBottomGreen ) / 255.0f;
    sky_state.mSkyBottomColor[2] = float( cur_cs.m_nSkyBottomBlue ) / 255.0f;
    sky_state.mSkyBottomColor[3] = 1.0f;

    sky_state.mAmbientColor[0] =
        ( sky_state.mSkyTopColor[0] + sky_state.mSkyBottomColor[0] ) / 2.0f;
    sky_state.mAmbientColor[1] =
        ( sky_state.mSkyTopColor[1] + sky_state.mSkyBottomColor[1] ) / 2.0f;
    sky_state.mAmbientColor[2] =
        ( sky_state.mSkyTopColor[2] + sky_state.mSkyBottomColor[2] ) / 2.0f;
    sky_state.mAmbientColor[3] = 1.0f;
    sky_state.mSunDir[0]       = vec_to_sun_arr[current_tc_value].x;
    sky_state.mSunDir[1]       = vec_to_sun_arr[current_tc_value].y;
    sky_state.mSunDir[2]       = vec_to_sun_arr[current_tc_value].z;
    sky_state.mSunDir[3]       = 1.0f;
}
void *rwD3D9RasterDtor( void *object ) { return object; }

int32_t AddPtLight( char a1, float x, float y, float z, float dx, int dy,
                    int dz, float rad, float r, float g, float b, char fogtype,
                    char extrashadows )
{
    using namespace rh::rw::engine;
    assert( gRenderClient );
    auto &light_state = gRenderClient->RenderState.Lights;

    PointLight l{};
    l.mPos[0]   = x;
    l.mPos[1]   = y;
    l.mPos[2]   = z;
    l.mRadius   = rad;
    l.mColor[0] = r;
    l.mColor[1] = g;
    l.mColor[2] = b;
    l.mColor[3] = 1;

    light_state.RecordPointLight( std::move( l ) );

    return 1;
}

void RenderSkyPolys() {}

int32_t rwDevicePluginInit() { return 1; }

BOOL APIENTRY DllMain( HMODULE /*hModule*/, DWORD ul_reason_for_call,
                       LPVOID /*lpReserved*/ )
{
    switch ( ul_reason_for_call )
    {
    case DLL_PROCESS_ATTACH:
        g_pIO_API = { reinterpret_cast<RwStreamFindChunk_FN>( 0x7ED2D0 ),
                      reinterpret_cast<RwStreamRead_FN>( 0x7EC9D0 ) };
        /*g_pGlobal_API = {reinterpret_cast<RwSystemFunc>( 0x7F5F70 ),
                         reinterpret_cast<uint32_t *>( 0xB4E9E0 ),
                         reinterpret_cast<RwResourcesAllocateResEntry_FN>(
           0x807ED0 )};*/
        g_pRaster_API  = { reinterpret_cast<RwRasterCreate_FN>( 0x7FB230 ),
                          reinterpret_cast<RwRasterDestroy_FN>( 0x7FB020 ) };
        g_pTexture_API = { reinterpret_cast<RwTextureCreate_FN>( 0x7F37C0 ),
                           reinterpret_cast<RwTextureSetName_FN>( 0x7F38A0 ),
                           reinterpret_cast<RwTextureSetName_FN>( 0x7F3910 ) };
        rh::debug::DebugLogger::Init( "gtasa_logs.log",
                                      rh::debug::LogLevel::Info );

        rh::rw::engine::IPCSettings::mMode =
            rh::rw::engine::IPCRenderMode::CrossProcessClient;
        rh::rw::engine::IPCSettings::mProcessName = "gta_sa_render_driver.exe";

        {
            RwPointerTable gtasa_ptr_table{};
            gtasa_ptr_table.mRwRwDeviceGlobalsPtr = 0xC9BCC0;
            gtasa_ptr_table.mRwDevicePtr          = 0x8E2498;
            gtasa_ptr_table.mIm3DOpen             = 0x7F3401;
            gtasa_ptr_table.mIm3DClose            = 0x7F33FA; // 0x7EFE20;
            gtasa_ptr_table.m_fpCheckNativeTextureSupport = 0x745530;
            gtasa_ptr_table.m_fpCheckEnviromentMapSupport = 0x5D8980;
            gtasa_ptr_table.m_fpSetVideoMode              = 0x7F8640;
            gtasa_ptr_table.m_fpSetRefreshRate            = 0x7F8580;
            gtasa_ptr_table.mRasterRegisterPluginPtr      = 0x7FB0B0;
            gtasa_ptr_table.mCameraRegisterPluginPtr      = 0x7EE450;
            gtasa_ptr_table.mMaterialRegisterPluginPtr    = 0x74DBF0;
            gtasa_ptr_table.mAllocateResourceEntry        = 0x807ED0;
            gtasa_ptr_table.mFreeResourceEntry            = 0x807DE0;
            gtasa_ptr_table.mAtomicGetHAnimHierarchy      = 0x7C7540;
            gtasa_ptr_table.mGeometryGetSkin              = 0x7C7550;
            gtasa_ptr_table.mGetSkinToBoneMatrices        = 0x7C7810;
            gtasa_ptr_table.mGetVertexBoneWeights         = 0x7C77F0;
            gtasa_ptr_table.mGetVertexBoneIndices         = 0x7C7800;

            gtasa_ptr_table.mIm3DTransform              = 0x7EF450;
            gtasa_ptr_table.mIm3DRenderIndexedPrimitive = 0x7EF550;
            gtasa_ptr_table.mIm3DRenderPrimitive        = 0x7EF6B0;
            gtasa_ptr_table.mIm3DRenderLine             = 0x7EF900;
            gtasa_ptr_table.mIm3DEnd                    = 0x7EF520;

            //  pipeline ptr
            RwGameHooks::Patch( gtasa_ptr_table );
        }

        IdleHook::Patch();

        {
            // RedirectJump( 0x4C9A80,
            //             reinterpret_cast<void *>( rwD3D9RasterDtor ) );
            RedirectJump( 0x7F5F60,
                          reinterpret_cast<void *>( rwDevicePluginInit ) );

            RedirectCall(
                0x748A30,
                reinterpret_cast<void *>( empty_void ) ); // CGammaInitialise
            // RedirectCall( 0x5BD779,
            //              reinterpret_cast<void *>(
            //                  empty_void ) ); // CPostEffects10Initialise
            RedirectCall( 0x53EA0D,
                          reinterpret_cast<void *>(
                              empty_void ) ); // 2CRealTimeShadowManager6Update
            RedirectCall( 0x53EA12,
                          reinterpret_cast<void *>(
                              prepare_timecyc ) ); // CMirrors16BeforeMainRender
            RedirectCall( 0x53EABA,
                          reinterpret_cast<void *>(
                              empty_void ) ); // CMirrors18RenderMirrorBuffer

            // Lod Rendering hot bug fix
            RedirectCall( 0x553C78, reinterpret_cast<void *>( empty_void ) );
            RedirectCall( 0x553C9A, reinterpret_cast<void *>( empty_void ) );
            RedirectCall( 0x553CD1, reinterpret_cast<void *>( empty_void ) );
            RedirectCall( 0x553CEC, reinterpret_cast<void *>( empty_void ) );
            RedirectCall( 0x53E096, reinterpret_cast<void *>( empty_void ) );
            RedirectCall( 0x53E0B9, reinterpret_cast<void *>( empty_void ) );
            RedirectCall( 0x53E0D3, reinterpret_cast<void *>( empty_void ) );
            RedirectCall( 0x53E0EF, reinterpret_cast<void *>( empty_void ) );

            RedirectCall(
                0x53EAC4,
                reinterpret_cast<void *>(
                    empty_void ) ); // CVisibilityPlugins21RenderWeaponPedsForPC
            SetPointer( 0x8D633C,
                        reinterpret_cast<void *>( D3D8AtomicAllInOneNode ) );
            SetPointer( 0x8DED0C,
                        reinterpret_cast<void *>(
                            D3D8SkinAtomicAllInOneNode ) ); // skin

            // SetPointer( 0x6DF754, reinterpret_cast<void *>( rxD3D8SubmitNode
            // ) );

            // postprocess
            RedirectCall( 0x53E227, reinterpret_cast<void *>( true_ret_hook ) );
            // Lights
            RedirectJump( 0x7000E0, reinterpret_cast<void *>( AddPtLight ) );
            // Clouds/Sky
            RedirectJump( 0x714650,
                          reinterpret_cast<void *>( RenderSkyPolys ) );
            // Enable Z-Test for clouds
            uint8_t ztest = 1;
            Patch( 0x71397E, &ztest, sizeof( ztest ) );
            /* SetPointer( 0x8E297C,
                         reinterpret_cast<void *>( rxD3D8SubmitNode ) );*/

            InitClient();
        }
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH: break;
    case DLL_PROCESS_DETACH:
    {
        ShutdownClient();
        break;
    }
    }
    return TRUE;
}