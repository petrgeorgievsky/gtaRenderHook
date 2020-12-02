#include "call_redirection_util.h"
#include "game/PointLights.h"
#include "game/Renderer.h"
#include "game/Shadows.h"
#include "game_patches/base_model_pipeline.h"
#include "game_patches/car_path_bug_fix.h"
#include "gta3_geometry_proxy.h"
#include <ConfigUtils/ConfigurationManager.h>
#include <DebugUtils/DebugLogger.h>
#include <DebugUtils/Win32UncaughtExceptionHandler.h>
#include <MemoryInjectionUtils/InjectorHelpers.h>
#include <filesystem>
#include <ipc/ipc_utils.h>
#include <material_storage.h>
#include <render_loop.h>
#include <rw_engine/rh_backend/raster_backend.h>
#include <rw_engine/rw_api_injectors.h>
#include <rw_engine/rw_frame/rw_frame.h>
#include <rw_engine/rw_macro_constexpr.h>
#include <rw_engine/rw_rh_skin_pipeline.h>
#include <rw_game_hooks.h>

using namespace rh::rw::engine;

RwIOPointerTable rh::rw::engine::g_pIO_API = {
    reinterpret_cast<RwStreamFindChunk_FN>( 0x5AA800 ),
    reinterpret_cast<RwStreamRead_FN>( 0x5A3D90 ) };
RwRasterPointerTable rh::rw::engine::g_pRaster_API = {
    reinterpret_cast<RwRasterCreate_FN>( 0x5ADBF0 ),
    reinterpret_cast<RwRasterDestroy_FN>( 0x5ADA40 ) };
RwTexturePointerTable rh::rw::engine::g_pTexture_API = {
    reinterpret_cast<RwTextureCreate_FN>( 0x5A7590 ),
    reinterpret_cast<RwTextureSetName_FN>( 0x5A7670 ),
    reinterpret_cast<RwTextureSetName_FN>( 0x5A76E0 ) };

bool true_ret_hook() { return true; }

class Clouds
{
    void RenderBackground( int16_t topred, int16_t topgreen, int16_t topblue,
                           int16_t botred, int16_t botgreen, int16_t botblue,
                           int16_t alpha )
    {
    }
};

int32_t rwD3D8FindCorrectRasterFormat( RwRasterType type, uint32_t flags )
{
    uint32_t format = flags & rwRASTERFORMATMASK;
    switch ( type )
    {
    case rwRASTERTYPENORMAL:
    case rwRASTERTYPETEXTURE:
        if ( ( format & rwRASTERFORMATPIXELFORMATMASK ) ==
             rwRASTERFORMATDEFAULT )
        {
            /* Check if we are requesting a default pixel format palette texture
             */
            if ( format & rwRASTERFORMATPAL8 )
            {
                format |= rwRASTERFORMAT8888;
            }
            if ( ( format & rwRASTERFORMATPAL8 ) == 0 )
            {
                format |= rwRASTERFORMAT8888;
            }
            else
            {
                format = rwRASTERFORMAT8888;
            }
        }
        else
        {
            /* No support for 4 bits palettes */
            if ( format & rwRASTERFORMATPAL4 )
            {
                /* Change it to a 8 bits palette */
                format &= static_cast<uint32_t>( ~rwRASTERFORMATPAL4 );

                format |= rwRASTERFORMATPAL8;
            }
            if ( format & rwRASTERFORMATPAL8 )
            {
                /* Change it to a 8 bits palette */
                format &= static_cast<uint32_t>( ~rwRASTERFORMATPAL8 );

                // format = rwRASTERFORMATPAL8;
            }
        }
        break;
    case rwRASTERTYPECAMERATEXTURE:
        if ( ( format & rwRASTERFORMATPIXELFORMATMASK ) ==
             rwRASTERFORMATDEFAULT )
        {
            format |= rwRASTERFORMAT888;
        }
        break;

    case rwRASTERTYPECAMERA:
        /* Always force default */
        if ( ( format & rwRASTERFORMATPIXELFORMATMASK ) ==
             rwRASTERFORMATDEFAULT )
        {
            format = rwRASTERFORMAT8888;
        }
        break;

    case rwRASTERTYPEZBUFFER:
        /* Always force default */
        if ( ( format & rwRASTERFORMATPIXELFORMATMASK ) ==
             rwRASTERFORMATDEFAULT )
        {
            format = rwRASTERFORMAT32;
        }
        break;
    default: break;
    }
    return static_cast<int32_t>( format );
}

char **_psGetVideoModeList()
{
    int32_t                    numModes;
    int32_t                    i;
    static std::vector<char *> videomode_list;
    if ( !videomode_list.empty() )
    {
        return videomode_list.data();
    }
    if ( !SystemHandler( rwDEVICESYSTEMGETNUMMODES, &numModes, nullptr, 0 ) )
        return videomode_list.data();

    videomode_list.resize( numModes, nullptr );

    for ( i = 0; i < numModes; i++ )
    {
        RwVideoMode vm{};
        if ( !SystemHandler( rwDEVICESYSTEMGETMODEINFO, &vm, nullptr, i ) )
        {
            videomode_list[i] = nullptr;
            continue;
        }
        videomode_list[i] = new char[4096];
        sprintf( videomode_list[i], "%d X %d X %d HZ", vm.width, vm.height,
                 vm.refRate );
    }

    return videomode_list.data();
}

class TxdStore
{
  public:
    static int32_t FindTxdSlot( const char *name )
    {
        return InMemoryFuncCall<int32_t, 0x527810>( name );
    }
    static void PushCurrentTxd() { return InMemoryFuncCall<void, 0x527B40>(); }
    static void PopCurrentTxd() { return InMemoryFuncCall<void, 0x527B50>(); }
    static void SetCurrentTxd( int32_t id )
    {
        return InMemoryFuncCall<void, 0x527B00>( id );
    }
};

RpMaterial *RpMaterialStreamReadImpl( void *stream )
{
    return InMemoryFuncCall<RpMaterial *, 0x5AE060>( stream );
}

RwTexture *RwTextureRead( const char *name, const char *maskName )
{
    return InMemoryFuncCall<RwTexture *, 0x5A7840>( name, maskName );
}

RpMaterial *RpMaterialStreamRead( void *stream )
{
    auto material = RpMaterialStreamReadImpl( stream );
    if ( material == nullptr )
        return nullptr;
    auto mat_ext = GetBackendMaterialExt( material );
    if ( !material->texture )
        return material;

    auto &m_ext_sys = MaterialExtensionSystem::GetInstance();

    auto string_s_name = std::string( material->texture->name );
    auto desc          = m_ext_sys.GetMatDesc( string_s_name );
    if ( !desc )
        return material;

    if ( auto txd_slot =
             TxdStore::FindTxdSlot( desc->mTextureDictName.c_str() );
         txd_slot != -1 )
    {
        TxdStore::PushCurrentTxd();
        TxdStore::SetCurrentTxd( txd_slot );

        // Read textures
        mat_ext->mSpecTex =
            RwTextureRead( desc->mSpecularTextureName.data(), nullptr );

        TxdStore::PopCurrentTxd();
    }

    return material;
}

BOOL WINAPI DllMain( HINSTANCE hModule, DWORD ul_reason_for_call,
                     LPVOID lpReserved )
{
    switch ( ul_reason_for_call )
    {
    case DLL_PROCESS_ATTACH:
    {
        /// Init logging
        rh::debug::DebugLogger::Init( "gta3_logs.log",
                                      rh::debug::LogLevel::Info );
        rh::debug::InitExceptionHandler();

        rh::rw::engine::IPCSettings::mMode =
            rh::rw::engine::IPCRenderMode::CrossProcessClient;

        rh::rw::engine::IPCSettings::mProcessName = "gta_3_render_driver.exe";

        /// Init config
        auto       cfg_mgr  = rh::engine::ConfigurationManager::Instance();
        const auto cfg_path = "gta3_rh_config.cfg";
        if ( !cfg_mgr.LoadFromFile( cfg_path ) )
            cfg_mgr.SaveToFile( cfg_path );

        RwPointerTable gta3_ptr_table{};

        gta3_ptr_table.mRwDevicePtr                  = 0x618B50;
        gta3_ptr_table.mRwRwDeviceGlobalsPtr         = 0x6F1D08;
        gta3_ptr_table.mIm3DOpen                     = 0x5A17DF;
        gta3_ptr_table.mIm3DClose                    = 0x5A17D8;
        gta3_ptr_table.m_fpCheckNativeTextureSupport = 0x584F4B;
        gta3_ptr_table.m_fpSetRefreshRate            = 0x5B9890;
        gta3_ptr_table.mRasterRegisterPluginPtr      = 0x5ADAD0;
        gta3_ptr_table.mCameraRegisterPluginPtr      = 0x5A55B0;
        gta3_ptr_table.mMaterialRegisterPluginPtr    = 0x5AE000;
        gta3_ptr_table.mAllocateResourceEntry        = 0x5C3430;
        gta3_ptr_table.mFreeResourceEntry            = 0x5C3340;
        gta3_ptr_table.mAtomicGetHAnimHierarchy      = 0x5B1330;
        gta3_ptr_table.mGeometryGetSkin              = 0x5B1340;
        gta3_ptr_table.mGetSkinToBoneMatrices        = 0x5B1390;
        gta3_ptr_table.mGetVertexBoneWeights         = 0;
        gta3_ptr_table.mGetVertexBoneIndices         = 0;
        gta3_ptr_table.mIm3DTransform                = 0x5B69E0;
        gta3_ptr_table.mIm3DRenderIndexedPrimitive   = 0x5B6AE0;
        gta3_ptr_table.mIm3DRenderLine               = 0x5B6C40;
        gta3_ptr_table.mIm3DEnd                      = 0x5B6AB0;

        RwGameHooks::Patch( gta3_ptr_table );
        BaseModelPipelines::Patch();

        /// TODO: Move to another cpp/hpp file
        // RedirectJump( 0x581830,
        //             reinterpret_cast<void *>( _psGetVideoModeList ) );

        RedirectCall( 0x5C9126,
                      reinterpret_cast<void *>( RpMaterialStreamRead ) );

        RedirectJump( 0x510980,
                      reinterpret_cast<void *>( PointLights::AddLight ) );

        RedirectJump( 0x59A610, reinterpret_cast<void *>(
                                    rwD3D8FindCorrectRasterFormat ) );

        // Hide default sky
        RedirectJump( 0x4F7FE0, reinterpret_cast<void *>( true_ret_hook ) );

        RedirectJump( 0x4A8A60,
                      reinterpret_cast<void *>( Renderer::ScanWorld ) );
        RedirectJump( 0x4A7930,
                      reinterpret_cast<void *>( Renderer::PreRender ) );
        RedirectJump( 0x48E0F0, reinterpret_cast<void *>( Renderer::Render ) );

        CPathFind::Patch();
        Shadows::Patch();

        InitClient();
        //
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
