//
// Created by peter on 20.04.2020.
//
#include <ConfigUtils/ConfigurationManager.h>
#include <DebugUtils/DebugLogger.h>
#include <DebugUtils/Win32UncaughtExceptionHandler.h>
#include <ipc/ipc_utils.h>
#include <ipc/shared_memory_queue_client.h>
#include <rw_game_hooks.h>

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
using namespace rh::rw::engine;

struct Globals
{
    RwDevice          rwDevice{};
    RwRwDeviceGlobals rwDeviceGlobals{};
    RwMemoryFunctions rwMemoryFunctions{};
};

Globals gInstance{};

int APIENTRY WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance,
                      LPSTR lpCmdLine, int nCmdShow )
{
    using namespace rh::debug;

    /// Init logs
    DebugLogger::Init( "render_driver.log", LogLevel::Info );
    InitExceptionHandler();

    IPCSettings::mMode = IPCRenderMode::CrossProcessRenderer;

    /// Init config
    auto cfg_mgr  = rh::engine::ConfigurationManager::Instance();
    auto cfg_path = "renderer_config.cfg";
    if ( !cfg_mgr.LoadFromFile( cfg_path ) )
        cfg_mgr.SaveToFile( cfg_path );

    /// Init renderer
    RwGameHooks::Patch(
        { .mRwDevicePtr = reinterpret_cast<INT_PTR>( &gInstance.rwDevice ) } );

    DeviceGlobals::DeviceGlobalsPtr = &gInstance.rwDeviceGlobals;

    InitRenderer();

    SystemHandler( rwDEVICESYSTEMREGISTER, DeviceGlobals::DevicePtr,
                   &gInstance.rwMemoryFunctions, 0 );

    gRenderDriver->GetTaskQueue().WaitForExit();

    ShutdownRenderer();
    return 1;
}