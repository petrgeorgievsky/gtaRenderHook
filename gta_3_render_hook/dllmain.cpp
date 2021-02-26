#include "call_redirection_util.h"
#include "game/Clouds.h"
#include "game/PointLights.h"
#include "game/Renderer.h"
#include "game/Shadows.h"
#include "game_patches/base_model_pipeline.h"
#include "game_patches/material_system_patches.h"
#include "game_patches/rwd3d8_patches.h"
#include "game_patches/skin_model_pipeline.h"
#include <ConfigUtils/ConfigurationManager.h>
#include <DebugUtils/DebugLogger.h>
#include <DebugUtils/Win32UncaughtExceptionHandler.h>
#include <ipc/ipc_utils.h>
#include <ipc/shared_memory_queue_client.h>
#include <rw_engine/rw_api_injectors.h>
#include <rw_game_hooks.h>

using namespace rh;
using namespace rh::rw::engine;

BOOL WINAPI DllMain( HINSTANCE, DWORD ul_reason_for_call, LPVOID )
{
    switch ( ul_reason_for_call )
    {
    case DLL_PROCESS_ATTACH:
    {
        /// Init logging
        debug::DebugLogger::Init( "gta3_logs.log", debug::LogLevel::Error );
        debug::InitExceptionHandler();

        IPCSettings::mMode        = IPCRenderMode::CrossProcessClient;
        IPCSettings::mProcessName = "gta_3_render_driver.exe";

        /// Init config
        auto       cfg_mgr  = engine::ConfigurationManager::Instance();
        const auto cfg_path = "gta3_rh_config.cfg";
        if ( !cfg_mgr.LoadFromFile( cfg_path ) )
            cfg_mgr.SaveToFile( cfg_path );

        /// Populate RW pointer table with rw-specific addresses

        g_pIO_API     = { reinterpret_cast<RwStreamFindChunk_FN>(
                          GetAddressByGame( 0x5AA540, 0x5AA800, 0x5ACC00 ) ),
                      reinterpret_cast<RwStreamRead_FN>(
                          GetAddressByGame( 0x5A3AD0, 0x5A3D90, 0x5A4900 ) ) };
        g_pRaster_API = {
            reinterpret_cast<RwRasterCreate_FN>(
                GetAddressByGame( 0x5AD930, 0x5ADBF0, 0x5B0580 ) ),
            reinterpret_cast<RwRasterDestroy_FN>(
                GetAddressByGame( 0x5AD780, 0x5ADA40, 0x5B0360 ) ) };
        g_pTexture_API = {
            reinterpret_cast<RwTextureCreate_FN>(
                GetAddressByGame( 0x5A72D0, 0x5A7590, 0x5A8AC0 ) ),
            reinterpret_cast<RwTextureSetName_FN>(
                GetAddressByGame( 0x5A73B0, 0x5A7670, 0x5A8B50 ) ),
            reinterpret_cast<RwTextureSetName_FN>(
                GetAddressByGame( 0x5A7420, 0x5A76E0, 0x5A8BE0 ) ) };

        RwPointerTable gta3_ptr_table{};

        gta3_ptr_table.mRwDevicePtr =
            GetAddressByGame( 0x619488, 0x618B50, 0x625B00 );
        gta3_ptr_table.mRwRwDeviceGlobalsPtr =
            GetAddressByGame( 0x6F1D08, 0x6F1D08, 0x701E48 );
        gta3_ptr_table.mIm3DOpen =
            GetAddressByGame( 0x5A151F, 0x5A17DF, 0x5A1362 );
        gta3_ptr_table.mIm3DClose =
            GetAddressByGame( 0x5A1518, 0x5A17D8, 0x5A136A );
        gta3_ptr_table.m_fpCheckNativeTextureSupport =
            GetAddressByGame( 0x584C0B, 0x584F4B, 0x584E3B );
        gta3_ptr_table.m_fpSetRefreshRate =
            GetAddressByGame( 0x5B95D0, 0x5B9890, 0x5BE5D0 );
        gta3_ptr_table.mRasterRegisterPluginPtr =
            GetAddressByGame( 0x5AD810, 0x5ADAD0, 0x5B07B0 );
        gta3_ptr_table.mCameraRegisterPluginPtr =
            GetAddressByGame( 0x5A52F8, 0x5A55B0, 0x5A75A0 );
        gta3_ptr_table.mMaterialRegisterPluginPtr =
            GetAddressByGame( 0x5ADD40, 0x5AE000, 0x5B0A10 );
        gta3_ptr_table.mAllocateResourceEntry =
            GetAddressByGame( 0x5C3170, 0x5C3430, 0x5C87E0 );
        gta3_ptr_table.mFreeResourceEntry =
            GetAddressByGame( 0x5C3080, 0x5C3340, 0x5C86D0 );
        gta3_ptr_table.mAtomicGetHAnimHierarchy =
            GetAddressByGame( 0x5B1070, 0x5B1330, 0x5B4B80 );
        gta3_ptr_table.mGeometryGetSkin =
            GetAddressByGame( 0x5B1080, 0x5B1340, 0x5B4B94 );
        gta3_ptr_table.mGetSkinToBoneMatrices =
            GetAddressByGame( 0x5B10D0, 0x5B1390, 0x5B4C30 );
        gta3_ptr_table.mIm3DTransform =
            GetAddressByGame( 0x5B6720, 0x5B69E0, 0x5BB0D0 );
        gta3_ptr_table.mIm3DRenderIndexedPrimitive =
            GetAddressByGame( 0x5B6820, 0x5B6AE0, 0x5BB200 );
        gta3_ptr_table.mIm3DRenderLine =
            GetAddressByGame( 0x5B6980, 0x5B6C40, 0x5BB3D0 );
        gta3_ptr_table.mIm3DEnd =
            GetAddressByGame( 0x5B67F0, 0x5B6AB0, 0x5BB1C0 );
        RwGameHooks::Patch( gta3_ptr_table );
        RwD3D8Patches::Patch();
        PatchMaterialSystem();
        BaseModelPipeline::Patch();
        SkinModelPipeline::Patch();
        PointLights::Patch();
        Clouds::Patch();
        Renderer::Patch();
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
