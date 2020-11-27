#include "call_redirection_util.h"
#include "game/PointLights.h"
#include "game/Renderer.h"
#include "game_patches/base_model_pipeline.h"
#include "game_patches/car_path_bug_fix.h"
#include "gta3_geometry_proxy.h"
#include <ConfigUtils/ConfigurationManager.h>
#include <DebugUtils/DebugLogger.h>
#include <MemoryInjectionUtils/InjectorHelpers.h>
#include <filesystem>
#include <ipc/ipc_utils.h>
#include <nlohmann/json.hpp>
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

void debug_log( char *format... )
{
    std::string str( 250, '\0' );
    va_list     args;
    va_start( args, format );

    vsprintf_s( str.data(), 250, format, args );

    va_end( args );

    rh::debug::DebugLogger::Log( "GTA3LOGS:" + str );
}

static int32_t rxD3D8SubmitNode( void *, const void * ) { return 1; }

bool    false_ret_hook() { return false; }
bool    true_ret_hook() { return true; }
void    empty_hook() {}
int32_t RwIm3DRenderLine( int32_t vert1, int32_t vert2 ) { return 1; }
int32_t RwIm3DRenderTriangle( int32_t vert1, int32_t vert2, int32_t vert3 )
{
    return 1;
}
int32_t RwIm3DRenderIndexedPrimitive( RwPrimitiveType primType,
                                      uint16_t *indices, int32_t numIndices )
{
    rh::rw::engine::EngineClient::gIm3DGlobals.RenderIndexedPrimitive(
        primType, indices, numIndices );
    return 1;
}
int32_t RwIm3DRenderPrimitive( RwPrimitiveType primType ) { return 1; }
void *  RwIm3DTransform( void *pVerts, uint32_t numVerts, RwMatrix *ltm,
                         uint32_t flags )
{
    rh::rw::engine::EngineClient::gIm3DGlobals.Transform(
        static_cast<RwIm3DVertex *>( pVerts ), numVerts, ltm, flags );
    return pVerts;
}

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

struct MaterialDescription
{
    std::string          mTextureDictName;
    std::array<char, 32> mSpecularTextureName{};
};

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

class MaterialExtensionSystem
{
  public:
    static MaterialExtensionSystem &GetInstance()
    {
        static MaterialExtensionSystem m;
        return m;
    }

    std::optional<MaterialDescription> GetMatDesc( const std::string &name )
    {
        auto x = mMaterials.find( name );
        return x != mMaterials.end() ? x->second
                                     : std::optional<MaterialDescription>{};
    }

  private:
    MaterialExtensionSystem()
    {
        namespace fs  = std::filesystem;
        auto dir_path = fs::current_path() / "materials";
        if ( !fs::exists( dir_path ) )
            return;
        for ( auto &p : fs::directory_iterator( dir_path ) )
        {
            const fs::path &file_path = p.path();
            if ( file_path.extension() != ".mat" )
                continue;
            ParseMaterialDesc( file_path );
        }
    }
    void ParseMaterialDesc( const std::filesystem::path &mat_desc )
    {
        std::ifstream filestream( mat_desc );
        if ( !filestream.is_open() )
        {
            rh::debug::DebugLogger::Error(
                "Failed to open material description" );
            return;
        }
        auto &desc = mMaterials[mat_desc.stem().generic_string()];

        nlohmann::json desc_json{};
        filestream >> desc_json;
        auto tex_name = desc_json["spec_tex"].get<std::string>();
        desc.mSpecularTextureName.fill( 0 );
        std::copy( tex_name.begin(), tex_name.end(),
                   desc.mSpecularTextureName.begin() );
        desc.mTextureDictName =
            desc_json["tex_dict_slot_name"].get<std::string>();
    }
    std::unordered_map<std::string, MaterialDescription> mMaterials;
};

RpMaterial *RpMaterialStreamReadImpl( void *stream )
{
    return ( reinterpret_cast<RpMaterial *(__cdecl *)( void * )>( 0x5AE060 ) )(
        stream );
}

RwTexture *RwTextureRead( const char *name, const char *maskName )
{
    return (
        reinterpret_cast<RwTexture *(__cdecl *)( const char *, const char * )>(
            0x5A7840 ) )( name, maskName );
}
RpMaterial *RpMaterialStreamRead( void *stream )
{
    auto material = RpMaterialStreamReadImpl( stream );
    if ( material == nullptr )
        return nullptr;
    auto mat_ext = GetBackendMaterialExt( material );
    if ( material->texture )
    {
        auto &m_ext_sys     = MaterialExtensionSystem::GetInstance();
        auto  string_s_name = std::string( material->texture->name );
        auto  desc          = m_ext_sys.GetMatDesc( string_s_name );
        if ( desc )
        {
            auto txd_slot =
                TxdStore::FindTxdSlot( desc->mTextureDictName.c_str() );
            if ( txd_slot != -1 )
            {
                TxdStore::PushCurrentTxd();
                TxdStore::SetCurrentTxd( txd_slot );
                mat_ext->mSpecTex =
                    RwTextureRead( desc->mSpecularTextureName.data(), nullptr );
                TxdStore::PopCurrentTxd();
            }
        }
    }
    return material;
}

class CShadows
{
  public:
    static void StoreShadowForPedObject( CEntity *, float, float, float, float,
                                         float, float )
    {
    }
    static void StoreShadowForPole( CEntity *, float, float, float, float,
                                    float, uint32_t )
    {
    }
    static void StoreCarLightShadow( CEntity *, int32_t, RwTexture *, CVector *,
                                     float, float, float, float, uint8_t,
                                     uint8_t, uint8_t, float )
    {
    }

    static void StoreStaticShadow( uint32_t, uint8_t, RwTexture *, CVector *,
                                   float, float, float, float, int16_t, uint8_t,
                                   uint8_t, uint8_t, float, float, float, bool,
                                   float )
    {
    }
    static void StoreShadowForCar( CEntity * ) {}
};

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

        rh::rw::engine::IPCSettings::mMode =
            rh::rw::engine::IPCRenderMode::CrossProcessClient;

        rh::rw::engine::IPCSettings::mProcessName = "gta_3_render_driver.exe";

        /// Init config
        auto cfg_mgr  = rh::engine::ConfigurationManager::Instance();
        auto cfg_path = "gta3_rh_config.cfg";
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

        // 5C3430
        //  pipeline ptr
        RwGameHooks::Patch( gta3_ptr_table );
        BaseModelPipelines::Patch();
        /// TODO: Move to another cpp/hpp file
        RedirectJump( 0x581830,
                      reinterpret_cast<void *>( _psGetVideoModeList ) );

        // RedirectJump( 0x405DB0, reinterpret_cast<void *>( debug_log ) );
        // RedirectJump( 0x59E720, reinterpret_cast<void *>( debug_log ) );
        RedirectCall( 0x5C9126,
                      reinterpret_cast<void *>( RpMaterialStreamRead ) );

        RedirectJump( 0x510980,
                      reinterpret_cast<void *>( PointLights::AddLight ) );

        // RedirectCall( 0x478902, reinterpret_cast<void *>( load_threadable
        // )
        // );

        RedirectJump( 0x5B69E0, reinterpret_cast<void *>( RwIm3DTransform ) );
        RedirectJump( 0x5B6AE0, reinterpret_cast<void *>(
                                    RwIm3DRenderIndexedPrimitive ) );
        RedirectJump( 0x5B6C40, reinterpret_cast<void *>( RwIm3DRenderLine ) );
        // Im3DEnd
        RedirectJump( 0x5B6AB0, reinterpret_cast<void *>( empty_hook ) );

        // RedirectJump( 0x429C00, reinterpret_cast<void *>( true_ret_hook )
        // );
        RedirectJump( 0x59A610, reinterpret_cast<void *>(
                                    rwD3D8FindCorrectRasterFormat ) );
        SetPointer( 0x61AADC, reinterpret_cast<void *>( rxD3D8SubmitNode ) );

        // "Fix" car paths, ugly bug appears for no reason that trashes all
        // memory for some reason
        RedirectCall( 0x4298F7, reinterpret_cast<void *>(
                                    CPathFind::PreparePathDataForType_Jmp ) );
        // Hide default sky
        RedirectJump( 0x4F7FE0, reinterpret_cast<void *>( true_ret_hook ) );

        RedirectJump( 0x4A8A60,
                      reinterpret_cast<void *>( Renderer::ScanWorld ) );
        RedirectJump( 0x4A7930,
                      reinterpret_cast<void *>( Renderer::PreRender ) );
        RedirectJump( 0x48E0F0, reinterpret_cast<void *>( Renderer::Render ) );

        /// Remove ingame static shadows

        RedirectJump( 0x513EC0, reinterpret_cast<void *>(
                                    CShadows::StoreShadowForPedObject ) );
        RedirectJump( 0x514020, reinterpret_cast<void *>(
                                    CShadows::StoreShadowForPole ) );
        RedirectJump( 0x513C80, reinterpret_cast<void *>(
                                    CShadows::StoreCarLightShadow ) );
        RedirectJump( 0x5132B0,
                      reinterpret_cast<void *>( CShadows::StoreStaticShadow ) );
        RedirectJump( 0x513A40,
                      reinterpret_cast<void *>( CShadows::StoreShadowForCar ) );

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
