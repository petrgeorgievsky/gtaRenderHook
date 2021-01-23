#include "../rh_backend/camera_backend.h"
#include "../rh_backend/im2d_backend.h"
#include "../rh_backend/mesh_rendering_backend.h"
#include "../rh_backend/raster_backend.h"
#include "../rw_api_injectors.h"
#include "../rw_standard_render_commands/camerabeginupdatecmd.h"
#include "../rw_standard_render_commands/cameraclearcmd.h"
#include "../rw_standard_render_commands/cameraendupdatecmd.h"
#include "../rw_standard_render_commands/nativetexturereadcmd.h"
#include "../rw_standard_render_commands/rastercreatecmd.h"
#include "../rw_standard_render_commands/rastershowrastercmd.h"
#include "raster_lock_cmd.h"
#include "rw_device_system_globals.h"
#include <DebugUtils/DebugLogger.h>
#include <Engine/Common/ISwapchain.h>
#include <Engine/Common/IWindow.h>
#include <Engine/D3D11Impl/D3D11DeviceState.h>
#include <Engine/EngineConfigBlock.h>
#include <Engine/VulkanImpl/VulkanDeviceState.h>
#include <ipc/ipc_utils.h>
#include <map>
#include <render_loop.h>
#include <rendering_loop/ray_tracing/RayTracingRenderer.h>
#include <rw_engine/rh_backend/material_backend.h>
#include <rw_engine/rh_backend/skinned_mesh_backend.h>
#include <rw_engine/rw_standard_render_commands/imagefindrasterformat.h>
#include <rw_engine/rw_standard_render_commands/rasterdestroycmd.h>
#include <rw_engine/rw_standard_render_commands/rasterlockcmd.h>
#include <rw_engine/rw_standard_render_commands/rastersetimagecmd.h>
#include <rw_engine/rw_standard_render_commands/rasterunlockcmd.h>
#include <string>

namespace rh::rw::engine
{
HWND gMainWindow;

struct RwEngineOpenParams
{
    void *displayID; /**< Display Identifier */
};

std::map<RwCoreDeviceSystemFn, std::string> gRenderRequestNameMap{
    { rwDEVICESYSTEMOPEN, "Open" },
    { rwDEVICESYSTEMCLOSE, "Close" },
    { rwDEVICESYSTEMSTART, "Start" },
    { rwDEVICESYSTEMSTOP, "Stop" },
    { rwDEVICESYSTEMREGISTER, "Register" },
    { rwDEVICESYSTEMGETNUMMODES, "GetNumModes" },
    { rwDEVICESYSTEMGETMODEINFO, "GetModeInfo" },
    { rwDEVICESYSTEMUSEMODE, "UseMode" },
    { rwDEVICESYSTEMFOCUS, "Focus" },
    { rwDEVICESYSTEMINITPIPELINE, "InitPipeline" },
    { rwDEVICESYSTEMGETMODE, "GetMode" },
    { rwDEVICESYSTEMSTANDARDS, "Standards" },
    { rwDEVICESYSTEMGETTEXMEMSIZE, "GetTexMemSize" },
    { rwDEVICESYSTEMGETNUMSUBSYSTEMS, "GetNumSubSystems" },
    { rwDEVICESYSTEMGETSUBSYSTEMINFO, "GetSubsystemInfo" },
    { rwDEVICESYSTEMGETCURRENTSUBSYSTEM, "GetCurentSubsystem" },
    { rwDEVICESYSTEMSETSUBSYSTEM, "SetSubsystem" },
    { rwDEVICESYSTEMFINALIZESTART, "FinalizeStart" },
    { rwDEVICESYSTEMINITIATESTOP, "InitiateStop" },
    { rwDEVICESYSTEMGETMAXTEXTURESIZE, "GetMaxTextureSize" },
    { rwDEVICESYSTEMRXPIPELINEREQUESTPIPE, "RxPipelineRequestPipe" },
    { rwDEVICESYSTEMGETMETRICBLOCK, "GetMetricBlock" },
    { rwDEVICESYSTEMGETID, "GetId" },
};

std::map<RwDeviceStandardFn, std::string> gRenderStandardNameMap{
    { rwSTANDARDNASTANDARD, "" },
    { rwSTANDARDCAMERABEGINUPDATE, "" },
    { rwSTANDARDRGBTOPIXEL, "" },
    { rwSTANDARDPIXELTORGB, "" },
    { rwSTANDARDRASTERCREATE, "" },
    { rwSTANDARDRASTERDESTROY, "" },
    { rwSTANDARDIMAGEGETRASTER, "" },
    { rwSTANDARDRASTERSETIMAGE, "" },
    { rwSTANDARDTEXTURESETRASTER, "" },
    { rwSTANDARDIMAGEFINDRASTERFORMAT, "" },
    { rwSTANDARDCAMERAENDUPDATE, "" },
    { rwSTANDARDSETRASTERCONTEXT, "" },
    { rwSTANDARDRASTERSUBRASTER, "" },
    { rwSTANDARDRASTERCLEARRECT, "" },
    { rwSTANDARDRASTERCLEAR, "" },
    { rwSTANDARDRASTERLOCK, "" },
    { rwSTANDARDRASTERUNLOCK, "" },
    { rwSTANDARDRASTERRENDER, "" },
    { rwSTANDARDRASTERRENDERSCALED, "" },
    { rwSTANDARDRASTERRENDERFAST, "" },
    { rwSTANDARDRASTERSHOWRASTER, "" },
    { rwSTANDARDCAMERACLEAR, "" },
    { rwSTANDARDHINTRENDERF2B, "" },
    { rwSTANDARDRASTERLOCKPALETTE, "" },
    { rwSTANDARDRASTERUNLOCKPALETTE, "" },
    { rwSTANDARDNATIVETEXTUREGETSIZE, "" },
    { rwSTANDARDNATIVETEXTUREREAD, "" },
    { rwSTANDARDNATIVETEXTUREWRITE, "" },
    { rwSTANDARDRASTERGETMIPLEVELS, "" },
    { rwSTANDARDNUMOFSTANDARD, "" },
};

std::array<RwStandardFunc, rwSTANDARDNUMOFSTANDARD> gStandards;

std::map<RwDeviceStandardFn, RwStandardFunc> gRenderStandardFuncMap{
    { rwSTANDARDNASTANDARD,
      []( void *pOut, void *pInOut, int32_t nI ) -> int32_t {
          debug::DebugLogger::Log(
              std::string( "RWGAMEHOOKS_LOG: rwSTANDARDNASTANDARD" ) );
          return 1;
      } },
    { rwSTANDARDCAMERABEGINUPDATE,
      []( void *pOut, void *pInOut, int32_t nI ) -> int32_t {
          RwCameraBeginUpdateCmd cmd( static_cast<RwCamera *>( pInOut ) );
          return cmd.Execute();
      } },
    { rwSTANDARDRGBTOPIXEL,
      []( void *pOut, void *pInOut, int32_t nI ) -> int32_t {
          debug::DebugLogger::Log(
              std::string( "RWGAMEHOOKS_LOG: rwSTANDARDRGBTOPIXEL" ) );
          return 1;
      } },
    { rwSTANDARDPIXELTORGB,
      []( void *pOut, void *pInOut, int32_t nI ) -> int32_t {
          debug::DebugLogger::Log(
              std::string( "RWGAMEHOOKS_LOG: rwSTANDARDPIXELTORGB" ) );
          return 1;
      } },
    { rwSTANDARDRASTERCREATE,
      []( void *pOut, void *pInOut, int32_t nI ) -> int32_t {
          RwRasterCreateCmd cmd( static_cast<RwRaster *>( pInOut ), nI );
          return cmd.Execute();
      } },
    { rwSTANDARDRASTERDESTROY,
      []( void *pOut, void *pInOut, int32_t nI ) -> int32_t {
          RwRasterDestroyCmd cmd( static_cast<RwRaster *>( pInOut ) );
          return cmd.Execute();
      } },
    { rwSTANDARDIMAGEGETRASTER,
      []( void *pOut, void *pInOut, int32_t nI ) -> int32_t {
          debug::DebugLogger::Log(
              std::string( "RWGAMEHOOKS_LOG: rwSTANDARDIMAGEGETRASTER" ) );
          return 1;
      } },
    { rwSTANDARDRASTERSETIMAGE,
      []( void *pOut, void *pInOut, int32_t nI ) -> int32_t {
          RwRasterSetImageCmd set_img_cmd( static_cast<RwRaster *>( pOut ),
                                           static_cast<RwImage *>( pInOut ) );
          return set_img_cmd.Execute();
      } },
    { rwSTANDARDTEXTURESETRASTER,
      []( void *pOut, void *pInOut, int32_t nI ) -> int32_t {
          debug::DebugLogger::Log(
              std::string( "RWGAMEHOOKS_LOG: rwSTANDARDTEXTURESETRASTER" ) );
          return 1;
      } },
    { rwSTANDARDIMAGEFINDRASTERFORMAT,
      []( void *pOut, void *pInOut, int32_t nI ) -> int32_t {
          RwImageFindRasterFormatCmd find_raster_cmd(
              static_cast<RwRaster *>( pOut ), static_cast<RwImage *>( pInOut ),
              static_cast<uint32_t>( nI ) );
          return find_raster_cmd.Execute();
      } },
    { rwSTANDARDCAMERAENDUPDATE,
      []( void *pOut, void *pInOut, int32_t nI ) -> int32_t {
          RwCameraEndUpdateCmd cmd( static_cast<RwCamera *>( pInOut ) );
          return cmd.Execute();
      } },
    { rwSTANDARDSETRASTERCONTEXT,
      []( void *pOut, void *pInOut, int32_t nI ) -> int32_t {
          debug::DebugLogger::Log(
              std::string( "RWGAMEHOOKS_LOG: rwSTANDARDSETRASTERCONTEXT" ) );
          return 1;
      } },
    { rwSTANDARDRASTERSUBRASTER,
      []( void *pOut, void *pInOut, int32_t nI ) -> int32_t {
          debug::DebugLogger::Log(
              std::string( "RWGAMEHOOKS_LOG: rwSTANDARDRASTERSUBRASTER" ) );
          return 1;
      } },
    { rwSTANDARDRASTERCLEARRECT,
      []( void *pOut, void *pInOut, int32_t nI ) -> int32_t {
          debug::DebugLogger::Log(
              std::string( "RWGAMEHOOKS_LOG: rwSTANDARDRASTERCLEARRECT" ) );
          return 1;
      } },
    { rwSTANDARDRASTERCLEAR,
      []( void *pOut, void *pInOut, int32_t nI ) -> int32_t {
          debug::DebugLogger::Log(
              std::string( "RWGAMEHOOKS_LOG: rwSTANDARDRASTERCLEAR" ) );
          return 1;
      } },
    { rwSTANDARDRASTERLOCK,
      []( void *pOut, void *pInOut, int32_t nI ) -> int32_t {
          RwRasterLockCmd lock_cmd( static_cast<RwRaster *>( pInOut ), nI );
          void *&         pOutData = *static_cast<void **>( pOut );
          return lock_cmd.Execute( pOutData );
      } },
    { rwSTANDARDRASTERUNLOCK,
      []( void *pOut, void *pInOut, int32_t nI ) -> int32_t {
          RwRasterUnlockCmd unlock_cmd( static_cast<RwRaster *>( pInOut ) );
          return unlock_cmd.Execute();
      } },
    { rwSTANDARDRASTERRENDER,
      []( void *pOut, void *pInOut, int32_t nI ) -> int32_t {
          debug::DebugLogger::Log(
              std::string( "RWGAMEHOOKS_LOG: rwSTANDARDRASTERRENDER" ) );
          return 1;
      } },
    { rwSTANDARDRASTERRENDERSCALED,
      []( void *pOut, void *pInOut, int32_t nI ) -> int32_t {
          debug::DebugLogger::Log(
              std::string( "RWGAMEHOOKS_LOG: rwSTANDARDRASTERRENDERSCALED" ) );
          return 1;
      } },
    { rwSTANDARDRASTERRENDERFAST,
      []( void *pOut, void *pInOut, int32_t nI ) -> int32_t {
          debug::DebugLogger::Log(
              std::string( "RWGAMEHOOKS_LOG: rwSTANDARDRASTERRENDERFAST" ) );
          return 1;
      } },
    { rwSTANDARDRASTERSHOWRASTER,
      []( void *pOut, void *pInOut, int32_t nI ) -> int32_t {
          RwRasterShowRasterCmd cmd( static_cast<RwRaster *>( pOut ), nI );
          return cmd.Execute();
      } },
    { rwSTANDARDCAMERACLEAR,
      []( void *pOut, void *pInOut, int32_t nI ) -> int32_t {
          RwCameraClearCmd cmd( static_cast<RwCamera *>( pOut ),
                                static_cast<RwRGBA *>( pInOut ), nI );
          return cmd.Execute();
      } },
    { rwSTANDARDHINTRENDERF2B,
      []( void *pOut, void *pInOut, int32_t nI ) -> int32_t {
          // debug::DebugLogger::Log(
          //     std::string( "RWGAMEHOOKS_LOG: rwSTANDARDHINTRENDERF2B" ) );
          return 1;
      } },
    { rwSTANDARDRASTERLOCKPALETTE,
      []( void *pOut, void *pInOut, int32_t nI ) -> int32_t {
          debug::DebugLogger::Log(
              std::string( "RWGAMEHOOKS_LOG: rwSTANDARDRASTERLOCKPALETTE" ) );
          return 1;
      } },
    { rwSTANDARDRASTERUNLOCKPALETTE,
      []( void *pOut, void *pInOut, int32_t nI ) -> int32_t {
          debug::DebugLogger::Log(
              std::string( "RWGAMEHOOKS_LOG: rwSTANDARDRASTERUNLOCKPALETTE" ) );
          return 1;
      } },
    { rwSTANDARDNATIVETEXTUREGETSIZE,
      []( void *pOut, void *pInOut, int32_t nI ) -> int32_t {
          debug::DebugLogger::Log( std::string(
              "RWGAMEHOOKS_LOG: rwSTANDARDNATIVETEXTUREGETSIZE" ) );
          return 1;
      } },
    { rwSTANDARDNATIVETEXTUREREAD,
      []( void *pOut, void *pInOut, int32_t nI ) -> int32_t {
          debug::DebugLogger::Log(
              std::string( "RWGAMEHOOKS_LOG: rwSTANDARDNATIVETEXTUREREAD" ) );

          RwNativeTextureReadCmd cmd( static_cast<RwStream *>( pOut ),
                                      static_cast<RwTexture **>( pInOut ) );
          return cmd.Execute();
      } },
    { rwSTANDARDNATIVETEXTUREWRITE,
      []( void *pOut, void *pInOut, int32_t nI ) -> int32_t {
          debug::DebugLogger::Log(
              std::string( "RWGAMEHOOKS_LOG: rwSTANDARDNATIVETEXTUREWRITE" ) );
          return 1;
      } },
    { rwSTANDARDRASTERGETMIPLEVELS,
      []( void *pOut, void *pInOut, int32_t nI ) -> int32_t {
          debug::DebugLogger::Log(
              std::string( "RWGAMEHOOKS_LOG: rwSTANDARDRASTERGETMIPLEVELS" ) );
          return 1;
      } },
    { rwSTANDARDNUMOFSTANDARD,
      []( void *pOut, void *pInOut, int32_t nI ) -> int32_t { return 1; } },
};

RwDevice *         DeviceGlobals::DevicePtr        = nullptr;
RwSystemFunc       DeviceGlobals::fpOldSystem      = nullptr;
RwRwDeviceGlobals *DeviceGlobals::DeviceGlobalsPtr = nullptr;

RwStandardFunc * DeviceGlobals::Standards = nullptr;
PluginPtrTable   DeviceGlobals::PluginFuncs{};
ResourcePtrTable DeviceGlobals::ResourceFuncs{};
SkinPtrTable     DeviceGlobals::SkinFuncs{};

std::unique_ptr<RenderClient> gRenderClient = nullptr;
std::unique_ptr<RenderDriver> gRenderDriver = nullptr;

bool SystemRegister( RwDevice &device, RwMemoryFunctions *memory_funcs )
{
    static bool is_registered                 = false;
    device                                    = *DeviceGlobals::DevicePtr;
    DeviceGlobals::DeviceGlobalsPtr->memFuncs = memory_funcs;

    if ( IPCSettings::mMode != IPCRenderMode::CrossProcessRenderer )
    {
        BackendRasterPluginAttach();
        BackendMaterialPluginAttach();
        // BackendCameraPluginAttach();
    }
    if ( is_registered )
        return true;

    if ( IPCSettings::mMode != IPCRenderMode::CrossProcessClient )
    {

        gRenderDriver->GetTaskQueue().RegisterTask(
            SharedMemoryTaskType::CREATE_WINDOW,
            std::make_unique<SharedMemoryTask>( []( void *memory ) {
                // execute
                HWND wnd;
                CopyMemory( &wnd, memory, sizeof( HWND ) );
                gRenderDriver->OpenMainWindow( wnd );
                EngineState::gCameraState = std::make_unique<CameraState>(
                    gRenderDriver->GetDeviceState() );
                EngineState::gFrameRenderer =
                    std::make_shared<RayTracingRenderer>();
            } ) );

        gRenderDriver->GetTaskQueue().RegisterTask(
            SharedMemoryTaskType::DESTROY_WINDOW,
            std::make_unique<SharedMemoryTask>( []( void *memory ) {
                // execute
                gRenderDriver->GetDeviceState().WaitForGPU();
                EngineState::gFrameRenderer.reset();
                EngineState::gCameraState.reset();
                gRenderDriver->CloseMainWindow();
            } ) );

        ImageLockCmdImpl::RegisterCallHandler();
        gRenderDriver->GetTaskQueue().RegisterTask(
            SharedMemoryTaskType::GET_VIDEO_MODE_COUNT,
            std::make_unique<SharedMemoryTask>( []( void *memory ) {
                // execute
                gRenderDriver->GetDeviceState().SetCurrentDisplayMode(
                    *static_cast<int32_t *>( memory ) );
            } ) );

        gRenderDriver->GetTaskQueue().RegisterTask(
            SharedMemoryTaskType::MESH_LOAD,
            std::make_unique<SharedMemoryTask>( []( void *memory ) {
                // execute
                CreateBackendMeshImpl( memory );
            } ) );

        gRenderDriver->GetTaskQueue().RegisterTask(
            SharedMemoryTaskType::GET_VIDEO_MODE,
            std::make_unique<SharedMemoryTask>( []( void *memory ) {
                // execute
                int32_t mode_id;
                CopyMemory( &mode_id, memory, sizeof( int32_t ) );

                rh::engine::DisplayModeInfo display_mode{};
                gRenderDriver->GetDeviceState().GetDisplayModeInfo(
                    mode_id, display_mode );
                CopyMemory( memory, &display_mode, sizeof( display_mode ) );
            } ) );

        gRenderDriver->GetTaskQueue().RegisterTask(
            SharedMemoryTaskType::GET_CURRENT_VIDEO_MODE,
            std::make_unique<SharedMemoryTask>( []( void *memory ) {
                // execute
                uint32_t &id = *static_cast<uint32_t *>( memory );

                gRenderDriver->GetDeviceState().GetCurrentDisplayMode( id );
            } ) );

        gRenderDriver->GetTaskQueue().RegisterTask(
            SharedMemoryTaskType::GET_VIDEO_MODE_COUNT,
            std::make_unique<SharedMemoryTask>( []( void *memory ) {
                // execute
                uint32_t count = 0;
                gRenderDriver->GetDeviceState().GetDisplayModeCount( 0, count );
                CopyMemory( memory, &count, sizeof( count ) );
            } ) );

        gRenderDriver->GetTaskQueue().RegisterTask(
            SharedMemoryTaskType::GET_ADAPTER_COUNT,
            std::make_unique<SharedMemoryTask>( []( void *memory ) {
                // execute
                uint32_t count = 0;
                gRenderDriver->GetDeviceState().GetAdaptersCount( count );
                CopyMemory( memory, &count, sizeof( count ) );
            } ) );

        gRenderDriver->GetTaskQueue().RegisterTask(
            SharedMemoryTaskType::GET_ADAPTER_INFO,
            std::make_unique<SharedMemoryTask>( []( void *memory ) {
                // execute
                std::array<char, 80> result{};
                MemoryReader         reader( memory );
                MemoryWriter         writer( memory );
                auto                 id = *reader.Read<int32_t>();

                std::string str;
                gRenderDriver->GetDeviceState().GetAdapterInfo(
                    static_cast<unsigned int>( id ), str );

                result[str.copy( result.data(), result.size() - 1 )] = '\0';
                writer.Write( result.data(), result.size() );
            } ) );

        gRenderDriver->GetTaskQueue().RegisterTask(
            SharedMemoryTaskType::GET_CURRENT_ADAPTER,
            std::make_unique<SharedMemoryTask>( []( void *memory ) {
                // execute
                MemoryWriter writer( memory );
                unsigned int adapter = 0;
                gRenderDriver->GetDeviceState().GetCurrentAdapter( adapter );
                auto adapter_id = static_cast<uint32_t>( adapter );
                writer.Write( &adapter_id );
            } ) );

        gRenderDriver->GetTaskQueue().RegisterTask(
            SharedMemoryTaskType::SET_CURRENT_ADAPTER,
            std::make_unique<SharedMemoryTask>( []( void *memory ) {
                // execute
                MemoryReader reader( memory );
                gRenderDriver->GetDeviceState().SetCurrentAdapter(
                    *reader.Read<uint32_t>() );
            } ) );

        gRenderDriver->GetTaskQueue().RegisterTask(
            SharedMemoryTaskType::DESTROY_RASTER,
            std::make_unique<SharedMemoryTask>( []( void *memory ) {
                auto &resources   = gRenderDriver->GetResources();
                auto &raster_pool = resources.GetRasterPool();
                // execute
                raster_pool.FreeResource( *static_cast<uint64_t *>( memory ) );
            } ) );
        gRenderDriver->GetTaskQueue().RegisterTask(
            SharedMemoryTaskType::MESH_DELETE,
            std::make_unique<SharedMemoryTask>( []( void *memory ) {
                auto &resources = gRenderDriver->GetResources();
                auto &mesh_pool = resources.GetMeshPool();
                // execute
                mesh_pool.FreeResource( *static_cast<uint64_t *>( memory ) );
            } ) );

        gRenderDriver->GetTaskQueue().RegisterTask(
            SharedMemoryTaskType::SKINNED_MESH_LOAD,
            std::make_unique<SharedMemoryTask>( []( void *memory ) {
                // execute
                rw::engine::CreateSkinMeshImpl( memory );
            } ) );

        gRenderDriver->GetTaskQueue().RegisterTask(
            SharedMemoryTaskType::SKINNED_MESH_UNLOAD,
            std::make_unique<SharedMemoryTask>( []( void *memory ) {
                // execute
                rw::engine::DestroySkinMeshImpl( memory );
            } ) );

        gRenderDriver->GetTaskQueue().RegisterTask(
            SharedMemoryTaskType::TEXTURE_LOAD,
            std::make_unique<SharedMemoryTask>( []( void *memory ) {
                // execute
                RasterHeader header{};
                CopyMemory( &header, memory, sizeof( RasterHeader ) );
                std::vector<rh::engine::ImageBufferInitData> buffer_init_data;
                buffer_init_data.reserve( header.mMipLevelCount );
                uint32_t memory_offset     = sizeof( RasterHeader );
                uint32_t non_zero_mip_lvls = 0;
                for ( uint32_t i = 0; i < header.mMipLevelCount; i++ )
                {
                    rh::engine::ImageBufferInitData mip_data{};
                    MipLevelHeader                  mipLevelHeader{};
                    CopyMemory( &mipLevelHeader,
                                static_cast<char *>( memory ) + memory_offset,
                                sizeof( MipLevelHeader ) );
                    memory_offset += sizeof( MipLevelHeader );

                    mip_data.mSize   = mipLevelHeader.mSize;
                    mip_data.mStride = mipLevelHeader.mStride;
                    mip_data.mData =
                        static_cast<char *>( memory ) + memory_offset;

                    memory_offset += mipLevelHeader.mSize;
                    buffer_init_data.push_back( mip_data );
                    if ( mip_data.mSize != 0 )
                        non_zero_mip_lvls++;
                }

                auto format = static_cast<rh::engine::ImageBufferFormat>(
                    header.mFormat );
                rh::engine::ImageBufferCreateParams image_buffer_ci{};

                image_buffer_ci.mDimension   = rh::engine::ImageDimensions::d2D;
                image_buffer_ci.mWidth       = header.mWidth;
                image_buffer_ci.mHeight      = header.mHeight;
                image_buffer_ci.mDepth       = header.mDepth;
                image_buffer_ci.mMipLevels   = non_zero_mip_lvls;
                image_buffer_ci.mFormat      = format;
                image_buffer_ci.mPreinitData = buffer_init_data;

                RasterData result_data{};
                result_data.mImageBuffer =
                    gRenderDriver->GetDeviceState().CreateImageBuffer(
                        image_buffer_ci );

                rh::engine::ImageViewCreateInfo shader_view_ci{};
                shader_view_ci.mBuffer = result_data.mImageBuffer;
                shader_view_ci.mFormat = format;
                shader_view_ci.mUsage =
                    rh::engine::ImageViewUsage::ShaderResource;
                shader_view_ci.mLevelCount = non_zero_mip_lvls;

                result_data.mImageView =
                    gRenderDriver->GetDeviceState().CreateImageView(
                        shader_view_ci );
                auto &resources   = gRenderDriver->GetResources();
                auto &raster_pool = resources.GetRasterPool();
                // TODO: Hide impl details
                int64_t result = raster_pool.RequestResource( result_data );
                CopyMemory( memory, &result, sizeof( int64_t ) );
            } ) );

        /*gRenderDriver->GetTaskQueue().RegisterTask(
            SharedMemoryTaskType::MATERIAL_LOAD,
            std::make_unique<SharedMemoryTask>( []( void *memory ) {
                // execute
                auto *init_data = static_cast<MaterialInitData *>( memory );

                *static_cast<uint64_t *>( memory ) =
                    MaterialGlobals::SceneMaterialPool->RequestResource(
                        { static_cast<int32_t>( init_data->mTexture ),
                          init_data->mColor,
                          static_cast<int32_t>( init_data->mSpecTexture ),
                          init_data->specular , init_data->diffuse } );
            } ) );*/

        /* gRenderDriver->GetTaskQueue().RegisterTask(
             SharedMemoryTaskType::MATERIAL_DELETE,
             std::make_unique<SharedMemoryTask>( []( void *memory ) {
                 // execute
                 MaterialGlobals::SceneMaterialPool->FreeResource(
                     *static_cast<uint64_t *>( memory ) );
             } ) );*/

        InitRenderEvents();
    }

    is_registered = true;
    return true;
}

int32_t SystemHandler( int32_t nOption, void *pOut, void *pInOut, int32_t nIn )
{
    assert( gRenderClient );
    auto &task_queue = gRenderClient->GetTaskQueue();
    auto  sys_fn     = static_cast<RwCoreDeviceSystemFn>( nOption );
    switch ( sys_fn )
    {
    case rwDEVICESYSTEMGETSUBSYSTEMINFO:
    {
        auto &subsystem = *(RwSubSystemInfo *)pOut;
        task_queue.ExecuteTask(
            SharedMemoryTaskType::GET_ADAPTER_INFO,
            [&nIn]( MemoryWriter &&memory_writer ) {
                // serialize
                memory_writer.Write( &nIn );
            },
            [&subsystem]( MemoryReader &&memory_reader ) {
                // deserialize
                const auto *     desc    = memory_reader.Read<char>( 80 );
                std::string_view desc_sv = std::string_view( desc, 80 );
                desc_sv.copy( subsystem.name, 80 );
            } );
        break;
    }
    /*case rwDEVICESYSTEMFINALIZESTART:
        if ( DeviceGlobals::fpOldSystem )
            return DeviceGlobals::fpOldSystem( nOption, pOut, pInOut, nIn );
        else
            return 1;*/
    case rwDEVICESYSTEMREGISTER:
    {
        auto deviceOut       = reinterpret_cast<RwDevice *>( pOut );
        auto memoryFunctions = reinterpret_cast<RwMemoryFunctions *>( pInOut );
        if ( !SystemRegister( *deviceOut, memoryFunctions ) )
            return 0;
        break;
    }
    case rwDEVICESYSTEMGETMODEINFO:
    {
        task_queue.ExecuteTask(
            SharedMemoryTaskType::GET_VIDEO_MODE,
            [&nIn]( MemoryWriter &&memory_writer ) {
                // serialize
                memory_writer.Write( &nIn );
            },
            [&pOut]( MemoryReader &&memory_reader ) {
                // deserialize
                const auto &display_mode =
                    *memory_reader.Read<rh::engine::DisplayModeInfo>();
                auto *videoMode    = static_cast<RwVideoMode *>( pOut );
                videoMode->width   = display_mode.width;
                videoMode->height  = display_mode.height;
                videoMode->depth   = 32;
                videoMode->flags   = RwVideoModeFlag::rwVIDEOMODEEXCLUSIVE;
                videoMode->refRate = display_mode.refreshRate;
                videoMode->format  = 512;
            } );
        break;
    }
    case rwDEVICESYSTEMGETMODE:
    {
        task_queue.ExecuteTask( SharedMemoryTaskType::GET_CURRENT_VIDEO_MODE,
                                EmptySerializer,
                                [&pOut]( MemoryReader &&memory_reader ) {
                                    // deserialize
                                    *static_cast<uint32_t *>( pOut ) =
                                        *memory_reader.Read<uint32_t>();
                                } );
        break;
    }
    case rwDEVICESYSTEMGETNUMMODES:
    {
        task_queue.ExecuteTask( SharedMemoryTaskType::GET_VIDEO_MODE_COUNT,
                                EmptySerializer,
                                [&pOut]( MemoryReader &&memory_reader ) {
                                    // deserialize
                                    *static_cast<uint32_t *>( pOut ) =
                                        *memory_reader.Read<uint32_t>();
                                } );
        break;
    }
    case rwDEVICESYSTEMUSEMODE:
    {
        using namespace rh::engine;
        DisplayModeInfo display_mode{};
        task_queue.ExecuteTask(
            SharedMemoryTaskType::GET_VIDEO_MODE,
            [&nIn]( MemoryWriter &&memory_writer ) {
                // serialize
                memory_writer.Write( &nIn );
            },
            [&display_mode]( MemoryReader &&memory_reader ) {
                // deserialize
                display_mode = *memory_reader.Read<decltype( display_mode )>();
            } );

        if ( IPCSettings::mMode == IPCRenderMode::CrossProcessRenderer )
            break;

        auto w = EngineConfigBlock::It.IsWindowed
                     ? display_mode.width
                     : GetSystemMetrics( SM_CXSCREEN );
        auto h = EngineConfigBlock::It.IsWindowed
                     ? display_mode.height
                     : GetSystemMetrics( SM_CYSCREEN );

        RECT rect{ .left   = 0,
                   .top    = 0,
                   .right  = static_cast<LONG>( w ),
                   .bottom = static_cast<LONG>( h ) };

        auto wnd_ex_style = GetWindowLongA( gMainWindow, GWL_EXSTYLE );
        auto wnd_style    = GetWindowLongA( gMainWindow, GWL_STYLE );
        auto wnd_has_menu = GetMenu( gMainWindow ) != nullptr;
        SetWindowLongPtr( gMainWindow, GWL_STYLE, WS_VISIBLE | WS_POPUP );

        AdjustWindowRectEx( &rect, static_cast<DWORD>( wnd_style ),
                            wnd_has_menu, static_cast<DWORD>( wnd_ex_style ) );

        SetWindowPos( gMainWindow, HWND_TOP, rect.left, rect.top,
                      rect.right - rect.left, rect.bottom - rect.top,
                      SWP_NOZORDER );
        break;
    }
    case rwDEVICESYSTEMGETNUMSUBSYSTEMS:
    {
        task_queue.ExecuteTask( SharedMemoryTaskType::GET_ADAPTER_COUNT,
                                EmptySerializer,
                                [&pOut]( MemoryReader &&memory_reader ) {
                                    // deserialize
                                    *static_cast<uint32_t *>( pOut ) =
                                        *memory_reader.Read<uint32_t>();
                                } );
        break;
    }
    case rwDEVICESYSTEMGETCURRENTSUBSYSTEM:
    {
        task_queue.ExecuteTask( SharedMemoryTaskType::GET_CURRENT_ADAPTER,
                                EmptySerializer,
                                [&pOut]( MemoryReader &&memory_reader ) {
                                    // deserialize
                                    *static_cast<uint32_t *>( pOut ) =
                                        *memory_reader.Read<uint32_t>();
                                } );
        break;
    }
    case rwDEVICESYSTEMSTANDARDS:
    {
        debug::DebugLogger::Log(
            std::string( "RWGAMEHOOKS_LOG: Standards FUNC: " ) );
        auto standardFunctions = reinterpret_cast<RwStandardFunc *>( pOut );
        for ( auto i = 0; i < 27; i++ )
        {
            gStandards[i] = standardFunctions[i] =
                gRenderStandardFuncMap[static_cast<RwDeviceStandardFn>( i )];
        }
        DeviceGlobals::Standards = gStandards.data();
        break;
    }
    case rwDEVICESYSTEMSTART:
    {
        // WINDOW INIT TASK
        task_queue.ExecuteTask( SharedMemoryTaskType::CREATE_WINDOW,
                                []( MemoryWriter &&memory_writer ) {
                                    // serialize
                                    memory_writer.Write( &gMainWindow );
                                } );
        break;
    }
    case rwDEVICESYSTEMSTOP:
    {
        task_queue.ExecuteTask( SharedMemoryTaskType::DESTROY_WINDOW );
        break;
    }
    case rwDEVICESYSTEMOPEN:
    {
        gMainWindow = static_cast<HWND>(
            static_cast<RwEngineOpenParams *>( pInOut )->displayID );
        break;
    }
    case rwDEVICESYSTEMCLOSE:
    {
        gMainWindow = nullptr;
        break;
    }
    case rwDEVICESYSTEMSETSUBSYSTEM:
    {
        task_queue.ExecuteTask(
            SharedMemoryTaskType::SET_CURRENT_ADAPTER,
            [&nIn]( MemoryWriter &&memory_writer ) {
                // serialize
                memory_writer.Write( &nIn );
            },
            EmptyDeserializer );
        break;
    }
    case rwDEVICESYSTEMFINALIZESTART:
    {
        if ( DeviceGlobals::fpOldSystem )
            return DeviceGlobals::fpOldSystem( nOption, pOut, pInOut, nIn );
        break;
    }
    case rwDEVICESYSTEMINITIATESTOP:
    {
        break;
    }
    default: throw std::logic_error( "bad" );
    }
    return 1;
}
} // namespace rh::rw::engine