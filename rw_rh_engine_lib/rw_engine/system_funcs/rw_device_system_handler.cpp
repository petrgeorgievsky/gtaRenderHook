#include <render_client/render_client.h>
#include <render_driver/render_driver.h>

#include <rw_engine/rw_api_injectors.h>
#include <rw_engine/system_funcs/rw_device_system_globals.h>

#include <rw_engine/system_funcs/rw_device_standards.h>

#include <rw_engine/system_funcs/get_adapter_cmd.h>
#include <rw_engine/system_funcs/get_adapter_count_cmd.h>
#include <rw_engine/system_funcs/get_adapter_info_cmd.h>
#include <rw_engine/system_funcs/set_adapter_cmd.h>

#include <rw_engine/system_funcs/get_video_mode_cmd.h>
#include <rw_engine/system_funcs/get_video_mode_count.h>
#include <rw_engine/system_funcs/get_video_mode_info_cmd.h>
#include <rw_engine/system_funcs/set_video_mode_cmd.h>

#include <rw_engine/system_funcs/start_cmd.h>
#include <rw_engine/system_funcs/stop_cmd.h>

#include <DebugUtils/DebugLogger.h>
#include <Engine/Common/IDeviceState.h>
#include <Engine/EngineConfigBlock.h>

namespace rh::rw::engine
{
HWND                                                gMainWindow;
DeviceGlobals                                       gRwDeviceGlobals{};
std::array<RwStandardFunc, rwSTANDARDNUMOFSTANDARD> gStandards{};

struct RwEngineOpenParams
{
    void *displayID; /**< Display Identifier */
};

bool SystemRegister( RwDevice &device, RwMemoryFunctions *memory_funcs );
bool SystemOpen( const RwEngineOpenParams &params );
bool SystemClose();
bool SystemStandards( RwStandardFunc *standards );
bool SystemStart();
bool SystemStop();
bool SystemGetNumSubSystems( uint32_t &count );
bool SystemGetCurrentSubSystem( uint32_t &id );
bool SystemSetSubSystem( uint32_t id );
bool SystemGetSubSystemInfo( uint32_t id, RwSubSystemInfo &info );
bool SystemGetNumModes( uint32_t &count );
bool SystemGetMode( uint32_t &id );
bool SystemUseMode( uint32_t id );
bool SystemGetModeInfo( uint32_t id, RwVideoMode &info );

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
        assert( pInOut );
        if ( !SystemOpen( *static_cast<RwEngineOpenParams *>( pInOut ) ) )
            return 0;
        break;
    }
    case rwDEVICESYSTEMCLOSE:
    {
        if ( !SystemClose() )
            return 0;
        break;
    }
    case rwDEVICESYSTEMSTANDARDS:
    {
        if ( !SystemStandards( reinterpret_cast<RwStandardFunc *>( pOut ) ) )
            return 0;
        break;
    }
    case rwDEVICESYSTEMSTART:
    {
        if ( !SystemStart() )
            return 0;
        break;
    }
    case rwDEVICESYSTEMSTOP:
    {
        if ( !SystemStop() )
            return 0;
        break;
    }
    case rwDEVICESYSTEMGETNUMSUBSYSTEMS:
    {
        if ( !SystemGetNumSubSystems( *static_cast<uint32_t *>( pOut ) ) )
            return 0;
        break;
    }
    case rwDEVICESYSTEMGETCURRENTSUBSYSTEM:
    {
        if ( !SystemGetCurrentSubSystem( *static_cast<uint32_t *>( pOut ) ) )
            return 0;
        break;
    }
    case rwDEVICESYSTEMSETSUBSYSTEM:
    {
        if ( !SystemSetSubSystem( nIn ) )
            return 0;
        break;
    }
    case rwDEVICESYSTEMGETSUBSYSTEMINFO:
    {
        if ( !SystemGetSubSystemInfo( nIn, *(RwSubSystemInfo *)pOut ) )
            return 0;
        break;
    }
    case rwDEVICESYSTEMGETNUMMODES:
    {
        if ( !SystemGetNumModes( *static_cast<uint32_t *>( pOut ) ) )
            return 0;
        break;
    }
    case rwDEVICESYSTEMGETMODE:
    {
        if ( !SystemGetMode( *static_cast<uint32_t *>( pOut ) ) )
            return 0;
        break;
    }
    case rwDEVICESYSTEMUSEMODE:
    {
        if ( !SystemUseMode( nIn ) )
            return 0;

        break;
    }
    case rwDEVICESYSTEMGETMODEINFO:
    {
        if ( !SystemGetModeInfo( nIn, *static_cast<RwVideoMode *>( pOut ) ) )
            return 0;
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

bool SystemRegister( RwDevice &device, RwMemoryFunctions *memory_funcs )
{
    assert( gRwDeviceGlobals.DevicePtr );
    assert( gRwDeviceGlobals.DeviceGlobalsPtr );

    device                                      = *gRwDeviceGlobals.DevicePtr;
    gRwDeviceGlobals.DeviceGlobalsPtr->memFuncs = memory_funcs;

    if ( IPCSettings::mMode != IPCRenderMode::CrossProcessRenderer )
    {
        assert( gRenderClient );
        gRenderClient->RegisterPlugins( gRwDeviceGlobals.PluginFuncs );
    }
    if ( IPCSettings::mMode != IPCRenderMode::CrossProcessClient )
    {
        assert( gRenderDriver );
        gRenderDriver->RegisterTasks();
    }

    return true;
}

bool SystemOpen( const RwEngineOpenParams &params )
{
    gMainWindow = static_cast<HWND>( params.displayID );
    return gMainWindow != nullptr;
}

bool SystemClose()
{
    gMainWindow = nullptr;
    return true;
}

bool SystemStandards( RwStandardFunc *standards )
{
    debug::DebugLogger::Log( "Register system standard functions..." );
    for ( auto i = 0; i < 27; i++ )
    {
        gStandards[i] = standards[i] =
            GetStandardMap()[static_cast<RwDeviceStandardFn>( i )];
    }
    gRwDeviceGlobals.Standards = gStandards.data();
    return true;
}

bool SystemStart()
{
    assert( gRenderClient );
    StartSystemCmdImpl cmd( gRenderClient->GetTaskQueue() );
    return cmd.Invoke( gMainWindow );
}

bool SystemStop()
{
    assert( gRenderClient );
    StopSystemCmdImpl cmd( gRenderClient->GetTaskQueue() );
    return cmd.Invoke();
}

bool SystemGetNumSubSystems( uint32_t &count )
{
    assert( gRenderClient );
    GetAdapterCountCmdImpl cmd( gRenderClient->GetTaskQueue() );
    count = cmd.Invoke();
    return true;
}

bool SystemGetCurrentSubSystem( uint32_t &id )
{
    assert( gRenderClient );
    GetAdapterIdCmdImpl cmd( gRenderClient->GetTaskQueue() );
    id = cmd.Invoke();
    return true;
}

bool SystemSetSubSystem( uint32_t id )
{
    assert( gRenderClient );
    SetAdapterIdCmdImpl cmd( gRenderClient->GetTaskQueue() );
    return cmd.Invoke( id );
}

bool SystemGetSubSystemInfo( uint32_t id, RwSubSystemInfo &info )
{
    assert( gRenderClient );
    std::span<char, 80>   subsystem_name{ info.name };
    GetAdapterInfoCmdImpl cmd( gRenderClient->GetTaskQueue() );
    if ( !cmd.Invoke( id, subsystem_name ) )
        return false;
    return true;
}

bool SystemGetNumModes( uint32_t &count )
{
    assert( gRenderClient );
    GetVideoModeCountCmdImpl cmd( gRenderClient->GetTaskQueue() );
    count = cmd.Invoke();
    return true;
}

bool SystemGetMode( uint32_t &id )
{
    assert( gRenderClient );
    GetVideoModeIdCmdImpl cmd( gRenderClient->GetTaskQueue() );
    id = cmd.Invoke();
    return true;
}

bool SystemUseMode( uint32_t id )
{
    assert( gRenderClient );
    auto &task_queue = gRenderClient->GetTaskQueue();
    using namespace rh::engine;

    GetVideoModeInfoCmdImpl get_mode_info_cmd( task_queue );
    SetVideoModeIdCmdImpl   set_mode_cmd( task_queue );

    DisplayModeInfo display_mode{};
    get_mode_info_cmd.Invoke( id, display_mode );
    set_mode_cmd.Invoke( id );

    auto w = EngineConfigBlock::It.IsWindowed ? display_mode.width
                                              : GetSystemMetrics( SM_CXSCREEN );
    auto h = EngineConfigBlock::It.IsWindowed ? display_mode.height
                                              : GetSystemMetrics( SM_CYSCREEN );

    RECT rect{ .left   = 0,
               .top    = 0,
               .right  = static_cast<LONG>( w ),
               .bottom = static_cast<LONG>( h ) };

    auto wnd_ex_style = GetWindowLongA( gMainWindow, GWL_EXSTYLE );
    auto wnd_style    = GetWindowLongA( gMainWindow, GWL_STYLE );
    auto wnd_has_menu = GetMenu( gMainWindow ) != nullptr;
    SetWindowLongPtr( gMainWindow, GWL_STYLE, WS_VISIBLE | WS_POPUP );

    AdjustWindowRectEx( &rect, static_cast<DWORD>( wnd_style ), wnd_has_menu,
                        static_cast<DWORD>( wnd_ex_style ) );

    SetWindowPos( gMainWindow, HWND_TOP, rect.left, rect.top,
                  rect.right - rect.left, rect.bottom - rect.top,
                  SWP_NOZORDER );
    return true;
}

bool SystemGetModeInfo( uint32_t id, RwVideoMode &info )
{
    assert( gRenderClient );
    rh::engine::DisplayModeInfo display_mode{};
    GetVideoModeInfoCmdImpl     cmd( gRenderClient->GetTaskQueue() );
    if ( cmd.Invoke( id, display_mode ) )
    {
        info.width   = display_mode.width;
        info.height  = display_mode.height;
        info.depth   = 32;
        info.flags   = RwVideoModeFlag::rwVIDEOMODEEXCLUSIVE;
        info.refRate = display_mode.refreshRate;
        info.format  = 512;
        return true;
    }
    return false;
}
} // namespace rh::rw::engine