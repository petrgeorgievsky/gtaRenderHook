#include <DebugUtils/DebugLogger.h>
#include <Engine/Common/IDeviceState.h>
#include <Windows.h>
#include <injection_utils/InjectorHelpers.h>
#include <ipc/ipc_utils.h>
#include <render_client/render_client.h>
#include <rw_engine/anim_hierarcy_rw36.h>
#include <rw_engine/rh_backend/raster_backend.h>
#include <rw_engine/rp_geometry_rw36.h>
#include <rw_engine/rw_api_injectors.h>
#include <rw_engine/rw_frame/rw_frame.h>
#include <rw_engine/rw_macro_constexpr.h>
#include <rw_engine/rw_rh_pipeline.h>
#include <rw_engine/rw_rh_skin_pipeline.h>
#include <rw_engine/system_funcs/rw_device_system_globals.h>
#include <rw_game_hooks.h>

using namespace rh::rw::engine;

RwIOPointerTable rh::rw::engine::g_pIO_API = {
    reinterpret_cast<RwStreamFindChunk_FN>( 0x64FAC0 ),
    reinterpret_cast<RwStreamRead_FN>( 0x6454B0 ) };
RwRasterPointerTable rh::rw::engine::g_pRaster_API = {
    reinterpret_cast<RwRasterCreate_FN>( 0x655490 ),
    reinterpret_cast<RwRasterDestroy_FN>( 0x6552E0 ) };
RwTexturePointerTable rh::rw::engine::g_pTexture_API = {
    reinterpret_cast<RwTextureCreate_FN>( 0x64DE60 ),
    reinterpret_cast<RwTextureSetName_FN>( 0x64DF40 ),
    reinterpret_cast<RwTextureSetName_FN>( 0x64DFB0 ) };

struct RwVideoModeVice
{
    int32_t  width;
    int32_t  height;
    int32_t  depth;
    uint32_t flags;
};

RwVideoModeVice *MyRwEngineGetVideoModeInfo( RwVideoModeVice *modeinfo,
                                             int32_t          modeIndex )
{

    RwVideoMode videoMode{};
    if ( !SystemHandler( rwDEVICESYSTEMGETMODEINFO, &videoMode, nullptr,
                         modeIndex ) )
        return nullptr;
    modeinfo->width  = videoMode.width;
    modeinfo->height = videoMode.height;
    modeinfo->depth  = videoMode.depth;
    modeinfo->flags  = videoMode.flags;
    return modeinfo;
}

static int32_t D3D8AtomicAllInOneNode( void * /*self*/,
                                       const RxPipelineNodeParam *params )
{
    static RpGeometryRw36 geometry_interface_35{};
    RpAtomic *            atomic;

    atomic           = static_cast<RpAtomic *>( params->dataParam );
    RpGeometry *geom = atomic->geometry;
    geometry_interface_35.Init( geom );
    if ( InstanceAtomic( atomic, &geometry_interface_35 ) !=
         RenderStatus::Instanced )
        return 0;

    auto ltm = RwFrameGetLTM(
        static_cast<RwFrame *>( rwObject::GetParent( atomic ) ) );
    DrawAtomic(
        atomic, &geometry_interface_35, [&ltm, atomic]( ResEnty *res_entry ) {
            auto &       renderer = gRenderClient->RenderState.MeshDrawCalls;
            DrawCallInfo info{};
            info.DrawCallId     = reinterpret_cast<uint64_t>( atomic );
            info.MeshId         = res_entry->meshData;
            info.WorldTransform = DirectX::XMFLOAT4X3{
                ltm->right.x, ltm->up.x, ltm->at.x, ltm->pos.x,
                ltm->right.y, ltm->up.y, ltm->at.y, ltm->pos.y,
                ltm->right.z, ltm->up.z, ltm->at.z, ltm->pos.z,
            };
            auto mesh_list = geometry_interface_35.GetMeshList();
            auto materials =
                renderer.AllocateDrawCallMaterials( mesh_list.size() );
            for ( auto i = 0; i < mesh_list.size(); i++ )
                materials[i] = ConvertMaterialData( mesh_list[i].material );
            renderer.RecordDrawCall( info );
        } );
    return 1;
}

static int32_t D3D8SkinAtomicAllInOneNode( void * /*self*/,
                                           const RxPipelineNodeParam *params )
{
    static RpGeometryRw36 geometry_interface_35{};
    RpAtomic *            atomic;

    atomic           = static_cast<RpAtomic *>( params->dataParam );
    RpGeometry *geom = atomic->geometry;
    // RpMeshHeader *meshHeader = geom->mesh;

    geometry_interface_35.Init( geom );
    if ( InstanceSkinAtomic( atomic, &geometry_interface_35 ) !=
         RenderStatus::Instanced )
        return 0;

    auto ltm = RwFrameGetLTM(
        static_cast<RwFrame *>( rwObject::GetParent( atomic ) ) );
    rh::rw::engine::DrawAtomic(
        atomic, &geometry_interface_35,
        [&ltm, atomic]( rh::rw::engine::ResEnty *res_entry ) {
            auto &renderer = gRenderClient->RenderState.SkinMeshDrawCalls;
            SkinDrawCallInfo info{};
            info.DrawCallId     = reinterpret_cast<uint64_t>( atomic );
            info.MeshId         = res_entry->meshData;
            info.WorldTransform = DirectX::XMFLOAT4X3{
                ltm->right.x, ltm->up.x, ltm->at.x, ltm->pos.x,
                ltm->right.y, ltm->up.y, ltm->at.y, ltm->pos.y,
                ltm->right.z, ltm->up.z, ltm->at.z, ltm->pos.z,
            };
            auto mesh_list = geometry_interface_35.GetMeshList();
            auto materials =
                renderer.AllocateDrawCallMaterials( mesh_list.size() );
            for ( auto i = 0; i < mesh_list.size(); i++ )
                materials[i] = ConvertMaterialData( mesh_list[i].material );
            static AnimHierarcyRw36 g_anim{};
            PrepareBoneMatrices( info.BoneTransform, atomic, g_anim );
            renderer.RecordDrawCall( info );
        } );

    return 1;
}
int32_t true_hook() { return 1; }
struct CVector
{
    float x, y, z;
};
class CPointLight
{
  public:
    CVector       m_vecPosn;
    CVector       m_vecDirection;
    float         m_fRange;
    float         m_fColorRed;
    float         m_fColorGreen;
    float         m_fColorBlue;
    unsigned char m_nType;
    unsigned char m_nFogType;
    bool          m_bGenerateShadows;

  private:
    char _pad2B;

  public:
};

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

int32_t water_render()
{
    // TODO: MOVE OUT OF HERE

    // Setup timecycle(move out of here)
    auto &sky_state = rh::rw::engine::gRenderClient->RenderState.SkyState;
    int & m_nCurrentSkyBottomBlue  = *(int *)0x9B6DF4;
    int & m_nCurrentSkyBottomGreen = *(int *)0x97F208;
    int & m_nCurrentSkyBottomRed   = *(int *)0xA0D958;
    int & m_nCurrentSkyTopBlue     = *(int *)0x978D1C;
    int & m_nCurrentSkyTopGreen    = *(int *)0xA0FD70;
    int & m_nCurrentSkyTopRed      = *(int *)0xA0CE98;

    auto *vec_to_sun_arr   = (RwV3d *)0x792C70;
    int & current_tc_value = *(int *)0xA0CFF8;

    sky_state.mSkyTopColor[0]    = float( m_nCurrentSkyTopRed ) / 255.0f;
    sky_state.mSkyTopColor[1]    = float( m_nCurrentSkyTopGreen ) / 255.0f;
    sky_state.mSkyTopColor[2]    = float( m_nCurrentSkyTopBlue ) / 255.0f;
    sky_state.mSkyTopColor[3]    = 1.0f;
    sky_state.mSkyBottomColor[0] = float( m_nCurrentSkyBottomRed ) / 255.0f;
    sky_state.mSkyBottomColor[1] = float( m_nCurrentSkyBottomGreen ) / 255.0f;
    sky_state.mSkyBottomColor[2] = float( m_nCurrentSkyBottomBlue ) / 255.0f;
    sky_state.mSkyBottomColor[3] = 1.0f;
    // TODO: Use original ambient color
    sky_state.mAmbientColor[0] = float( m_nCurrentSkyTopRed ) / 255.0f;
    sky_state.mAmbientColor[1] = float( m_nCurrentSkyTopGreen ) / 255.0f;
    sky_state.mAmbientColor[2] = float( m_nCurrentSkyTopBlue ) / 255.0f;
    sky_state.mAmbientColor[3] = 1.0f;

    sky_state.mSunDir[0] = vec_to_sun_arr[current_tc_value].x;
    sky_state.mSunDir[1] = vec_to_sun_arr[current_tc_value].y;
    sky_state.mSunDir[2] = vec_to_sun_arr[current_tc_value].z;
    sky_state.mSunDir[3] = 1.0f;

    return 1;
}

RpMaterial *RpMaterialStreamReadImpl( void *stream )
{
    return ( reinterpret_cast<RpMaterial *(__cdecl *)( void * )>( 0x655920 ) )(
        stream );
}

RwTexture *RwTextureRead( const char *name, const char *maskName )
{
    return (
        reinterpret_cast<RwTexture *(__cdecl *)( const char *, const char * )>(
            0x64E110 ) )( name, maskName );
}

std::unordered_map<std::string, std::string> g_specular_storage{};

void InitSpecStorage()
{
    static bool is_initialized = false;
    if ( is_initialized )
        return;
    namespace fs  = std::filesystem;
    auto dir_path = fs::current_path() / "materials";
    if ( !fs::exists( dir_path ) )
        return;
    for ( auto &p : fs::directory_iterator( dir_path ) )
    {
        const fs::path &file_path = p.path();

        if ( file_path.extension() == ".mat" )
        {
            auto file = fopen( file_path.generic_string().c_str(), "rt" );
            char specFileName[80];
            char normalFileName[80];
            if ( file )
            {
                auto res = fscanf( file, "%79s\n", specFileName );
                if ( res == EOF )
                    fclose( file );
                fclose( file );

                g_specular_storage[file_path.stem().generic_string()] =
                    std::string( specFileName );
            }
        }
    }
    is_initialized = true;
}

RpMaterial *RpMaterialStreamRead( void *stream )
{
    auto material = RpMaterialStreamReadImpl( stream );
    if ( material == nullptr )
        return nullptr;
    auto &mat_ext = BackendMaterialPlugin::GetData( material );
    if ( material->texture )
    {
        InitSpecStorage();
        auto string_s_name  = std::string( material->texture->name );
        auto spec_name_iter = g_specular_storage.find( string_s_name );
        if ( spec_name_iter != g_specular_storage.end() )
            mat_ext.mSpecTex =
                RwTextureRead( spec_name_iter->second.c_str(), nullptr );
    }
    return material;
}
BOOL emptystuff() { return false; }

BOOL WINAPI DllMain( HINSTANCE hModule, DWORD ul_reason_for_call,
                     LPVOID lpReserved )
{
    switch ( ul_reason_for_call )
    {
    case DLL_PROCESS_ATTACH:
    {
        rh::debug::DebugLogger::Init( "gtavc_logs.log",
                                      rh::debug::LogLevel::Error );
        rh::rw::engine::IPCSettings::mMode =
            rh::rw::engine::IPCRenderMode::CrossProcessClient;
        rh::rw::engine::IPCSettings::mProcessName = "gta_vc_render_driver.exe";
        RwPointerTable gtavc_ptr_table{};
        gtavc_ptr_table.mRwDevicePtr                  = 0x6DDE3C;
        gtavc_ptr_table.mRwRwDeviceGlobalsPtr         = 0x7DD708;
        gtavc_ptr_table.mIm3DOpen                     = 0x6431A0;
        gtavc_ptr_table.mIm3DClose                    = 0x6431A7;
        gtavc_ptr_table.m_fpCheckNativeTextureSupport = 0x602EAB;
        gtavc_ptr_table.mRasterRegisterPluginPtr      = 0x655370;
        gtavc_ptr_table.mCameraRegisterPluginPtr      = 0x64AAE0;
        gtavc_ptr_table.mMaterialRegisterPluginPtr    = 0x6558C0;
        gtavc_ptr_table.mAllocateResourceEntry        = 0x669330;

        gtavc_ptr_table.mFreeResourceEntry       = 0x669240; //
        gtavc_ptr_table.mAtomicGetHAnimHierarchy = 0x649950; //
        gtavc_ptr_table.mGeometryGetSkin         = 0x649960; //
        gtavc_ptr_table.mGetSkinToBoneMatrices   = 0x6499E0; //
        gtavc_ptr_table.mGetVertexBoneWeights    = 0x6499D0; //
        gtavc_ptr_table.mGetVertexBoneIndices    = 0;

        gtavc_ptr_table.mIm3DTransform              = 0x65AE90;
        gtavc_ptr_table.mIm3DRenderIndexedPrimitive = 0x65AF90;
        gtavc_ptr_table.mIm3DRenderLine             = 0x649C00;
        gtavc_ptr_table.mIm3DEnd                    = 0x65AF60;

        RwGameHooks::Patch( gtavc_ptr_table );
        //

        SetPointer( 0x6DF9AC,
                    reinterpret_cast<void *>( D3D8AtomicAllInOneNode ) );
        SetPointer(
            0x6DF8EC,
            reinterpret_cast<void *>( D3D8SkinAtomicAllInOneNode ) ); // skin
        // Hide default sky
        RedirectJump( 0x53F650, reinterpret_cast<void *>( true_hook ) );
        RedirectJump( 0x53F380, reinterpret_cast<void *>( true_hook ) );
        // Enable Z-Test for clouds
        uint8_t ztest = 1;
        Patch( 0x53FCD3, &ztest, sizeof( ztest ) );

        /// PBR tests
        RedirectCall( 0x66DD06,
                      reinterpret_cast<void *>( RpMaterialStreamRead ) );

        RedirectJump( 0x642B70,
                      reinterpret_cast<void *>( MyRwEngineGetVideoModeInfo ) );
        // Im3D
        // SetPointer( 0x6DF754, reinterpret_cast<void *>( rxD3D8SubmitNode ) );

        // check dxt support
        RedirectJump( 0x61E310, reinterpret_cast<void *>( true_hook ) );
        // Transparent water is buggy with RTX
        RedirectCall( 0x4A65AE, reinterpret_cast<void *>( water_render ) );
        // Buggy cutscene shadows
        RedirectJump( 0x625D80, reinterpret_cast<void *>( emptystuff ) );
        // Lights
        RedirectJump( 0x567700, reinterpret_cast<void *>( AddPtLight ) );

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
