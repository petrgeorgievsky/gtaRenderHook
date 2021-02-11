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
#include "get_adapter_cmd.h"
#include "get_adapter_count_cmd.h"
#include "get_adapter_info_cmd.h"
#include "get_video_mode_cmd.h"
#include "get_video_mode_count.h"
#include "get_video_mode_info_cmd.h"
#include "load_texture_cmd.h"
#include "mesh_load_cmd.h"
#include "mesh_unload_cmd.h"
#include "raster_destroy_cmd.h"
#include "raster_lock_cmd.h"
#include "rw_device_system_globals.h"
#include "set_adapter_cmd.h"
#include "set_video_mode_cmd.h"
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
#include <rw_engine/system_funcs/start_cmd.h>
#include <rw_engine/system_funcs/stop_cmd.h>
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

std::unique_ptr<RenderClient> gRenderClient = nullptr;
std::unique_ptr<RenderDriver> gRenderDriver = nullptr;
DeviceGlobals                 gRwDeviceGlobals{};

bool SystemRegister( RwDevice &device, RwMemoryFunctions *memory_funcs )
{
    static bool is_registered = false;
    assert( gRwDeviceGlobals.DevicePtr );
    assert( gRwDeviceGlobals.DeviceGlobalsPtr );

    device                                      = *gRwDeviceGlobals.DevicePtr;
    gRwDeviceGlobals.DeviceGlobalsPtr->memFuncs = memory_funcs;

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
        assert( gRenderDriver );
        auto &driver_task_queue = gRenderDriver->GetTaskQueue();
        /// Register driver tasks
        /// TODO: Move to RenderDriver
        StartSystemCmdImpl::RegisterCallHandler( driver_task_queue );
        StopSystemCmdImpl::RegisterCallHandler( driver_task_queue );
        GetAdapterCountCmdImpl::RegisterCallHandler( driver_task_queue );
        GetAdapterIdCmdImpl::RegisterCallHandler( driver_task_queue );
        SetAdapterIdCmdImpl::RegisterCallHandler( driver_task_queue );
        GetAdapterInfoCmdImpl::RegisterCallHandler( driver_task_queue );
        GetVideoModeCountCmdImpl::RegisterCallHandler( driver_task_queue );
        GetVideoModeIdCmdImpl::RegisterCallHandler( driver_task_queue );
        SetVideoModeIdCmdImpl::RegisterCallHandler( driver_task_queue );
        GetVideoModeInfoCmdImpl::RegisterCallHandler( driver_task_queue );

        RasterDestroyCmdImpl::RegisterCallHandler( driver_task_queue );
        LoadTextureCmdImpl::RegisterCallHandler( driver_task_queue );
        RasterLockCmdImpl::RegisterCallHandler( driver_task_queue );

        LoadMeshCmdImpl::RegisterCallHandler( driver_task_queue );
        UnloadMeshCmdImpl::RegisterCallHandler( driver_task_queue );

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

        InitRenderEvents();
    }

    is_registered = true;
    return true;
}

int32_t SystemHandler( int32_t nOption, void *pOut, void *pInOut, int32_t nIn )
{
    auto sys_fn = static_cast<RwCoreDeviceSystemFn>( nOption );
    switch ( sys_fn )
    {
    case rwDEVICESYSTEMREGISTER:
    {
        auto deviceOut       = reinterpret_cast<RwDevice *>( pOut );
        auto memoryFunctions = reinterpret_cast<RwMemoryFunctions *>( pInOut );
        if ( !SystemRegister( *deviceOut, memoryFunctions ) )
            return 0;
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
    case rwDEVICESYSTEMSTANDARDS:
    {
        debug::DebugLogger::Log( "Register system standard functions..." );
        auto standardFunctions = reinterpret_cast<RwStandardFunc *>( pOut );
        for ( auto i = 0; i < 27; i++ )
        {
            gStandards[i] = standardFunctions[i] =
                gRenderStandardFuncMap[static_cast<RwDeviceStandardFn>( i )];
        }
        gRwDeviceGlobals.Standards = gStandards.data();
        break;
    }
    case rwDEVICESYSTEMSTART:
    {
        assert( gRenderClient );
        StartSystemCmdImpl cmd( gRenderClient->GetTaskQueue() );
        return cmd.Invoke( gMainWindow );
    }
    case rwDEVICESYSTEMSTOP:
    {
        assert( gRenderClient );
        StopSystemCmdImpl cmd( gRenderClient->GetTaskQueue() );
        return cmd.Invoke();
    }
    case rwDEVICESYSTEMGETNUMSUBSYSTEMS:
    {
        assert( gRenderClient );
        GetAdapterCountCmdImpl cmd( gRenderClient->GetTaskQueue() );
        *static_cast<uint32_t *>( pOut ) = cmd.Invoke();
        break;
    }
    case rwDEVICESYSTEMGETCURRENTSUBSYSTEM:
    {
        assert( gRenderClient );
        GetAdapterIdCmdImpl cmd( gRenderClient->GetTaskQueue() );
        *static_cast<uint32_t *>( pOut ) = cmd.Invoke();
        break;
    }
    case rwDEVICESYSTEMSETSUBSYSTEM:
    {
        assert( gRenderClient );
        SetAdapterIdCmdImpl cmd( gRenderClient->GetTaskQueue() );
        return cmd.Invoke( nIn );
    }
    case rwDEVICESYSTEMGETSUBSYSTEMINFO:
    {
        assert( gRenderClient );
        auto &                subsystem      = *(RwSubSystemInfo *)pOut;
        std::span<char, 80>   subsystem_name = subsystem.name;
        GetAdapterInfoCmdImpl cmd( gRenderClient->GetTaskQueue() );
        if ( !cmd.Invoke( nIn, subsystem_name ) )
            return false;
        break;
    }
    case rwDEVICESYSTEMGETNUMMODES:
    {
        assert( gRenderClient );
        GetVideoModeCountCmdImpl cmd( gRenderClient->GetTaskQueue() );
        *static_cast<uint32_t *>( pOut ) = cmd.Invoke();
        break;
    }
    case rwDEVICESYSTEMGETMODE:
    {
        assert( gRenderClient );
        GetVideoModeIdCmdImpl cmd( gRenderClient->GetTaskQueue() );
        *static_cast<uint32_t *>( pOut ) = cmd.Invoke();
        break;
    }
    case rwDEVICESYSTEMUSEMODE:
    {
        assert( gRenderClient );
        auto &task_queue = gRenderClient->GetTaskQueue();
        using namespace rh::engine;

        GetVideoModeInfoCmdImpl get_mode_info_cmd( task_queue );
        SetVideoModeIdCmdImpl   set_mode_cmd( task_queue );

        DisplayModeInfo display_mode{};
        get_mode_info_cmd.Invoke( nIn, display_mode );
        set_mode_cmd.Invoke( nIn );

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
    case rwDEVICESYSTEMGETMODEINFO:
    {
        assert( gRenderClient );
        auto &video_mode = *static_cast<RwVideoMode *>( pOut );
        rh::engine::DisplayModeInfo display_mode{};
        GetVideoModeInfoCmdImpl     cmd( gRenderClient->GetTaskQueue() );
        if ( cmd.Invoke( nIn, display_mode ) )
        {
            video_mode.width   = display_mode.width;
            video_mode.height  = display_mode.height;
            video_mode.depth   = 32;
            video_mode.flags   = RwVideoModeFlag::rwVIDEOMODEEXCLUSIVE;
            video_mode.refRate = display_mode.refreshRate;
            video_mode.format  = 512;
        }
        break;
    }
    case rwDEVICESYSTEMFINALIZESTART:
    {
        if ( gRwDeviceGlobals.fpOldSystem )
            return gRwDeviceGlobals.fpOldSystem( nOption, pOut, pInOut, nIn );
        break;
    }
    case rwDEVICESYSTEMINITIATESTOP:
    {
        break;
    }
    default:
        throw std::logic_error(
            "Unsupported system command called via rwSystemHandler!" );
    }
    return 1;
}
} // namespace rh::rw::engine