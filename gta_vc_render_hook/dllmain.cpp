#include "game/PointLights.h"
#include "game/Renderer.h"
#include "game/Shadows.h"
#include "game_patches/base_model_pipeline.h"
#include "game_patches/material_system_patches.h"
#include "game_patches/rwd3d8_patches.h"
#include "game_patches/skin_model_pipeline.h"
#include <ConfigUtils/ConfigurationManager.h>
#include <DebugUtils/DebugLogger.h>
#include <Engine/Common/IDeviceState.h>
#include <injection_utils/InjectorHelpers.h>
#include <ipc/ipc_utils.h>
#include <render_client/render_client.h>
#include <rw_engine/rw_api_injectors.h>
#include <rw_game_hooks.h>

using namespace rh::rw::engine;

int32_t true_hook() { return 1; }

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
        /// Init config
        auto       cfg_mgr  = rh::engine::ConfigurationManager::Instance();
        const auto cfg_path = "rh_config.cfg";
        if ( !cfg_mgr.LoadFromFile( cfg_path ) )
            cfg_mgr.SaveToFile( cfg_path );

        // setup pointers
        rh::rw::engine::g_pIO_API = {
            reinterpret_cast<RwStreamFindChunk_FN>( 0x64FAC0 ),
            reinterpret_cast<RwStreamRead_FN>( 0x6454B0 ) };
        rh::rw::engine::g_pRaster_API = {
            reinterpret_cast<RwRasterCreate_FN>( 0x655490 ),
            reinterpret_cast<RwRasterDestroy_FN>( 0x6552E0 ) };
        rh::rw::engine::g_pTexture_API = {
            reinterpret_cast<RwTextureCreate_FN>( 0x64DE60 ),
            reinterpret_cast<RwTextureSetName_FN>( 0x64DF40 ),
            reinterpret_cast<RwTextureSetName_FN>( 0x64DFB0 ) };

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
        // Hide default sky
        RedirectJump( 0x53F650, reinterpret_cast<void *>( true_hook ) );
        RedirectJump( 0x53F380, reinterpret_cast<void *>( true_hook ) );
        // Enable Z-Test for clouds
        uint8_t ztest = 1;
        Patch( 0x53FCD3, &ztest, sizeof( ztest ) );
        // RedirectJump( 0x401000, reinterpret_cast<void *>( logstuff ) );
        BaseModelPipeline::Patch();
        SkinModelPipeline::Patch();
        PatchMaterialSystem();
        RwD3D8Patches::Patch();
        Renderer::Patch();
        Shadows::Patch();
        PointLights::Patch();

        // Im3D
        // SetPointer( 0x6DF754, reinterpret_cast<void *>( rxD3D8SubmitNode ) );

        // check dxt support
        RedirectJump( 0x61E310, reinterpret_cast<void *>( true_hook ) );
        // matfx disable
        // RedirectJump( 0x655EB0, reinterpret_cast<void *>( true_hook ) );

        // Transparent water is buggy with RTX
        // RedirectCall( 0x4A65AE, reinterpret_cast<void *>( water_render ) );

        // RedirectCall( 0x4CA267, reinterpret_cast<void *>( emptystuff ) );

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
