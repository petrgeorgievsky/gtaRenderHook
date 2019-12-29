#pragma once
#include "Engine/Common/types/string_typedefs.h"
#include "IBuffer.h"
#include "ICommandBuffer.h"
#include "IDeviceOutputView.h"
#include "IFrameBuffer.h"
#include "IPipeline.h"
#include "IRenderPass.h"
#include "IShader.h"
#include "ISyncPrimitive.h"
#include "IWindow.h"
#include <Windows.h>

namespace rh::engine
{

struct DisplayModeInfo
{
    uint32_t width;
    uint32_t height;
    uint32_t refreshRate;
    uint32_t padding;
};

struct OutputInfo
{
    uint32_t displayModeId;
    bool     windowed;
};

/**
 * @brief Rendering device state, holds info about physical devices,
 * avaliable display modes and logical device state.
 *
 */
class IDeviceState
{
  public:
    virtual ~IDeviceState() = default;

    /**
     * @brief Initializes rendering device and all it's states.
     */
    virtual bool Init() = 0;

    /**
     * @brief Releases resources held by rendering device.
     */
    virtual bool Shutdown() = 0;

    /**
     * @brief Get Adapters count
     *
     * @param n - adapters (GPU) count
     */
    virtual bool GetAdaptersCount( unsigned int &count ) = 0;

    /**
     * @brief Get the Adapter Info
     *
     * @param n - adapter id
     * @param info - adapter name
     */
    virtual bool GetAdapterInfo( unsigned int id, String &info ) = 0;

    /**
     * @brief Get Current Adapter
     *
     * @param n - current adapter
     */
    virtual bool GetCurrentAdapter( unsigned int &id ) = 0;

    /**
     * @brief Set Current Adapter
     *
     * @param n - adapter id
     */
    virtual bool SetCurrentAdapter( unsigned int id ) = 0;

    /**
     * @brief Get adapter output count
     *
     * @param adapterId - adapter id
     * @param n - output(monitor) count
     */
    virtual bool GetOutputCount( unsigned int  adapterId,
                                 unsigned int &count ) = 0;

    /**
     * @brief Get output device info
     *
     * @param n - output device id
     * @param info - output device name
     */
    virtual bool GetOutputInfo( unsigned int id, String &info ) = 0;

    /**
     * @brief Get current output device
     *
     * @param id - current output device id
     */
    virtual bool GetCurrentOutput( unsigned int &id ) = 0;

    /**
     * @brief Sets output(display) device for current adapter
     *
     * @param id - output device id
     */
    virtual bool SetCurrentOutput( unsigned int id ) = 0;

    /**
     * @brief Get Display Mode count
     *
     * @param outputId - current output(display) device id
     * @param count - display mode count
     */
    virtual bool GetDisplayModeCount( unsigned int  outputId,
                                      unsigned int &count ) = 0;

    /**
     * @brief Get display mode info
     *
     * @param n - display mode id
     * @param info - display mode info
     */
    virtual bool GetDisplayModeInfo( unsigned int     id,
                                     DisplayModeInfo &info ) = 0;

    /**
     * @brief Get current display mode
     *
     * @param id - current display mode id
     */
    virtual bool GetCurrentDisplayMode( unsigned int &id ) = 0;

    /**
     * @brief Sets display mode for current output device
     *
     * @param id - display mode id
     */
    virtual bool SetCurrentDisplayMode( unsigned int id ) = 0;

    virtual IDeviceOutputView *
    CreateDeviceOutputView( HWND window, const OutputInfo &info ) = 0;

    /**
     * @brief Creates a window wrapper, that will handle resize etc.
     *
     * @param window Win32 window handle.
     * @param info window creation params
     * @return IWindow* window interface
     */
    virtual IWindow *CreateDeviceWindow( HWND              window,
                                         const OutputInfo &info ) = 0;

    virtual ICommandBuffer *GetMainCommandBuffer() = 0;

    virtual ISyncPrimitive *CreateSyncPrimitive( SyncPrimitiveType type ) = 0;
    virtual IFrameBuffer *
    CreateFrameBuffer( const FrameBufferCreateParams &params ) = 0;
    virtual IRenderPass *
                       CreateRenderPass( const RenderPassCreateParams &params ) = 0;
    virtual IPipeline *CreatePipeline( const PipelineCreateParams &params ) = 0;
    virtual IShader *  CreateShader( const ShaderDesc &params )             = 0;
    virtual IBuffer *  CreateBuffer( const BufferCreateInfo &params )       = 0;

    // Executes the command buffer on GPU, waits for waitFor sync primitive and
    // signals to signal sync primitive after execution
    virtual void ExecuteCommandBuffer( ICommandBuffer *buffer,
                                       ISyncPrimitive *waitFor,
                                       ISyncPrimitive *signal ) = 0;

    virtual void Wait( const std::vector<ISyncPrimitive *> &primitiveList ) = 0;
};
}; // namespace rh::engine
