#include "rwtestsample.h"
#include "rw_engine/rw_camera/rw_camera.h"
#include "rw_engine/rw_frame/rw_frame.h"
#include "rw_engine/rw_raster/rw_raster.h"
#include "rw_engine/rw_standard_render_commands/camerabeginupdatecmd.h"
#include "rw_engine/rw_standard_render_commands/cameraendupdatecmd.h"
#include "rw_engine/rw_standard_render_commands/rastershowrastercmd.h"
#include "rw_engine/system_funcs/rw_device_system_globals.h"
#include <DebugUtils/DebugLogger.h>
#include <array>
#include <common_headers.h>
#include <ipc/ipc_utils.h>
#include <rw_game_hooks.h>
#include <scene_graph.h>

using namespace rh::rw::engine;

// RwGlobals *rh::rw::engine::g_pRwEngineInstance;
RwTestSample::RwTestSample( rh::engine::RenderingAPI api, void *inst )
    : TestSample( api, inst )
{
    // TODO: REMOVE
    if ( api == rh::engine::RenderingAPI::Vulkan )
        m_bRenderGUI = false;
}

RwTestSample::~RwTestSample() = default;

struct RwMemoryFunctions
{
    /* c.f.
     * Program Files/Microsoft Visual Studio/VC98/Include/MALLOC.H
     */
    void *( *rwmalloc )( size_t size, uint32_t hint );
    /**< Allocates memory blocks.
     *  \param size Number of bytes to allocate. Should be greater
     *         then zero.
     *  \param hint A RwUInt32 value representing a memory hint.
     *  \return A void pointer to the allocated space, or NULL if
     *          there is insufficient memory available.
     */
    void ( *rwfree )( void *mem );
    /**< Deallocates or frees a memory block.
     *  \param mem Previously allocated memory block to be freed.
     *         Shouldn't be NULL pointer.
     */
    void *( *rwrealloc )( void *mem, size_t newSize, uint32_t hint );
    /**< Reallocate memory blocks.
     *  \param mem Pointer to previously allocated memory block.
     *  \param size New size in bytes. Should be greater then zero.
     *  \param hint A RwUInt32 value representing a memory hint.
     *  \return A void pointer to the allocated space, or NULL if
     *          there is insufficient memory available.
     */
    void *( *rwcalloc )( size_t numObj, size_t sizeObj, uint32_t hint );
    /**< Allocates an array in memory with elements initialized to 0.
     *  \param numObj Non-zero number of elements.
     *  \param sizeObj Non-zero length in bytes of each element.
     *  \param hint A RwUInt32 value representing a memory hint.
     *  \return A void pointer to the allocated space, or NULL if
     *          there is insufficient memory available.
     */
};

bool RwTestSample::Initialize( void *wnd )
{
    unsigned int gpuCount;

    DeviceGlobals::PluginFuncs.CameraRegisterPlugin =
        []( int32_t size, uint32_t pluginID,
            RwPluginObjectConstructor constructCB,
            RwPluginObjectDestructor  destructCB,
            RwPluginObjectCopy        copyCB ) -> int32_t {
        return sizeof( RwCamera );
    };
    DeviceGlobals::PluginFuncs.RasterRegisterPlugin =
        []( int32_t size, uint32_t pluginID,
            RwPluginObjectConstructor constructCB,
            RwPluginObjectDestructor  destructCB,
            RwPluginObjectCopy        copyCB ) -> int32_t {
        return sizeof( RwRaster );
    };
    DeviceGlobals::PluginFuncs.MaterialRegisterPlugin =
        []( int32_t size, uint32_t pluginID,
            RwPluginObjectConstructor constructCB,
            RwPluginObjectDestructor  destructCB,
            RwPluginObjectCopy        copyCB ) -> int32_t {
        return sizeof( RpMaterial );
    };
    DeviceGlobals::PluginFuncs.MaterialSetStreamAlwaysCallBack =
        []( uint32_t                        pluginID,
            RwPluginDataChunkAlwaysCallBack alwaysCB ) -> int32_t { return 1; };
    auto devptr = static_cast<RwDevice *>( malloc( sizeof( RwDevice ) ) );
    new ( devptr ) RwDevice{};

    RwPointerTable ptr_table{ .mRwDevicePtr =
                                  reinterpret_cast<INT_PTR>( devptr ) };
    RwGameHooks::Patch( ptr_table );

    RwMemoryFunctions memoryFuncs{};
    DeviceGlobals::DeviceGlobalsPtr = new RwRwDeviceGlobals();
    InitClient();
    InitRenderer();
    SystemHandler( rwDEVICESYSTEMREGISTER, DeviceGlobals::DevicePtr,
                   &memoryFuncs, 0 );
    // Preparation of rendering engine, initializes info about hardware that'll
    // use this window
    if ( !SystemHandler( rwDEVICESYSTEMOPEN, nullptr, &wnd, 0 ) )
    {
        rh::debug::DebugLogger::Error( TEXT( "Failed to open RHEngine!" ) );
        return false;
    }

    std::array<void *, rwSTANDARDNUMOFSTANDARD> standards{};

    // Preparation of standard rendering callbacks
    if ( !SystemHandler( rwDEVICESYSTEMSTANDARDS, standards.data(), nullptr,
                         rwSTANDARDNUMOFSTANDARD ) )
    {
        rh::debug::DebugLogger::Error(
            TEXT( "Failed to initialize RH engine standards!" ) );
        return false;
    }

    // GPU count retrieval
    if ( !SystemHandler( rwDEVICESYSTEMGETNUMSUBSYSTEMS, &gpuCount, nullptr,
                         0 ) )
    {
        SystemHandler( rwDEVICESYSTEMCLOSE, nullptr, nullptr, 0 );
        rh::debug::DebugLogger::Error( TEXT( "Failed to get gpu count!" ) );
        return false;
    }

    rh::debug::DebugLogger::Log( TEXT( "GPU List:" ) );

    // Enumerate over GPUs and log all the info
    for ( unsigned int i = 0; i < gpuCount; i++ )
    {
        RwSubSystemInfo info{};

        if ( !SystemHandler( rwDEVICESYSTEMGETSUBSYSTEMINFO, &info, nullptr,
                             i ) )
        {
            SystemHandler( rwDEVICESYSTEMCLOSE, nullptr, nullptr, 0 );
            rh::debug::DebugLogger::Error( TEXT( "Failed to get gpu info!" ) );
            return false;
        }

        // m_aSubSysInfo.push_back(info);
        rh::debug::DebugLogger::Log( ToRHString( info.name ) );
    }

    // Show device settings dialog
    /*DeviceSettingsDialog::SetSubSystemInfo(m_aSubSysInfo);

if (DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG1), wnd,
DeviceSettingsDialog::DialogProc) <= 0) { TCHAR errormsg[4096];
    FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,
                  nullptr,
                  GetLastError(),
                  0,
                  errormsg,
                  4096,
                  nullptr);
    RHDebug::DebugLogger::Error(ToRHString(errormsg));
}*/
    RwVideoMode videoMode{};
    int         modeId    = 0;
    int         maxModeId = 0;
    SystemHandler( rwDEVICESYSTEMGETNUMMODES, &maxModeId, nullptr, 0 );
    while ( videoMode.width != 1280 && videoMode.height != 720 &&
            modeId < maxModeId )
    {
        if ( !SystemHandler( rwDEVICESYSTEMGETMODEINFO, &videoMode, nullptr,
                             modeId ) )
            break;
        modeId++;
    }
    SystemHandler( rwDEVICESYSTEMUSEMODE, nullptr, nullptr, modeId - 1 );
    if ( !SystemHandler( rwDEVICESYSTEMSTART, nullptr, nullptr, 0 ) )
    {
        rh::debug::DebugLogger::Error( TEXT( "Failed to start RWEngine!" ) );
        SystemHandler( rwDEVICESYSTEMCLOSE, nullptr, nullptr, 0 );
        return false;
    }
    // rh::engine::g_pRHRenderer->InitImGUI();

    return CustomInitialize();
}

bool RwTestSample::CustomInitialize()
{
    m_pMainCameraFrame = RwFrameCreate();
    m_pMainCamera      = RwCameraCreate();
    m_pMainCamera->frameBuffer =
        rh::rw::engine::RwRasterCreate( 1280, 720, 32, rwRASTERTYPECAMERA );
    m_pMainCamera->zBuffer =
        rh::rw::engine::RwRasterCreate( 1280, 720, 32, rwRASTERTYPEZBUFFER );
    m_pMainCamera->object.object.parent =
        static_cast<void *>( m_pMainCameraFrame );
    return true;
}

void RwTestSample::CustomShutdown()
{
    if ( m_pMainCamera )
    {
        if ( m_pMainCamera->frameBuffer )
        {
            rh::rw::engine::RwRasterDestroy( m_pMainCamera->frameBuffer );
            m_pMainCamera->frameBuffer = nullptr;
        }

        if ( m_pMainCamera->zBuffer )
        {
            rh::rw::engine::RwRasterDestroy( m_pMainCamera->zBuffer );
            m_pMainCamera->zBuffer = nullptr;
        }

        rh::rw::engine::RwCameraDestroy( m_pMainCamera );
        m_pMainCamera = nullptr;
        m_pMainCamera = nullptr;
    }

    SystemHandler( rwDEVICESYSTEMSTOP, nullptr, nullptr, 0 );
    SystemHandler( rwDEVICESYSTEMCLOSE, nullptr, nullptr, 0 );

    ShutdownRenderer();
    ShutdownClient();
}

void RwTestSample::Render()
{
    RwCameraBeginUpdateCmd( m_pMainCamera ).Execute();
    auto &frame                           = GetCurrentSceneGraph()->mFrameInfo;
    frame.mSkyTopColor[0]                 = 0.5f;
    frame.mSkyTopColor[1]                 = 0.5f;
    frame.mSkyTopColor[2]                 = 0.5f;
    frame.mSkyTopColor[3]                 = 1.0f;
    frame.mSunDir[0]                      = -0.5f;
    frame.mSunDir[1]                      = 0.5f;
    frame.mSunDir[2]                      = -0.5f;
    frame.mSunDir[3]                      = 1.0f;
    frame.mFirst4PointLights[0].mRadius   = 10.0f;
    frame.mFirst4PointLights[0].mPos[0]   = -10.0f;
    frame.mFirst4PointLights[0].mColor[0] = 1.0f;

    CustomRender();
    RwCameraEndUpdateCmd( m_pMainCamera ).Execute();
    RwRasterShowRasterCmd( m_pMainCamera->frameBuffer, 0 ).Execute();
}
