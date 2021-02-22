#pragma once
#include "ArrayProxy.h"
#include "Engine/Common/types/string_typedefs.h"
#include "IBuffer.h"
#include "ICommandBuffer.h"
#include "IDescriptorSetAllocator.h"
#include "IDescriptorSetLayout.h"
#include "IDeviceOutputView.h"
#include "IFrameBuffer.h"
#include "IImageBuffer.h"
#include "IImageView.h"
#include "INativeWindow.h"
#include "IPipeline.h"
#include "IPipelineLayout.h"
#include "IRenderPass.h"
#include "ISampler.h"
#include "IShader.h"
#include "ISyncPrimitive.h"
#include "IWindow.h"

#ifndef HWND
using HWND = struct HWND__ *;
#endif
#ifndef HINSTANCE
using HINSTANCE = struct HINSTANCE__ *;
#endif

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

struct BufferUpdateInfo
{
    uint64_t mOffset;
    uint64_t mRange;
    IBuffer *mBuffer;
};

struct ImageUpdateInfo
{
    ImageLayout mLayout;
    IImageView *mView    = nullptr;
    ISampler *  mSampler = nullptr;
};

/**
 * Contains device limits
 */
struct DeviceLimitsInfo
{
    // Alignment of an offset to buffer object
    uint32_t BufferOffsetMinAlign;

    uint32_t GetMinAlignedBufferEntrySize( uint32_t s ) const
    {
        return ( s + ( BufferOffsetMinAlign - 1 ) ) &
               ~( BufferOffsetMinAlign - 1 );
    }
};

struct AccelStructUpdateInfo
{
    void *mTLAS;
};

struct DescriptorSetUpdateInfo
{
    IDescriptorSet *                  mSet;
    uint32_t                          mBinding;
    DescriptorType                    mDescriptorType;
    uint32_t                          mArrayStartIdx = 0;
    ArrayProxy<BufferUpdateInfo>      mBufferUpdateInfo;
    ArrayProxy<ImageUpdateInfo>       mImageUpdateInfo;
    ArrayProxy<AccelStructUpdateInfo> mASUpdateInfo;
};

struct CommandBufferSubmitInfo
{
    ICommandBuffer *              mCmdBuffer;
    std::vector<ISyncPrimitive *> mWaitForDep;
    ISyncPrimitive *              mToSignalDep;
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

    /**
     * @brief Creates a window wrapper, that will handle resize etc.
     *
     * @param window Win32 window handle.
     * @param info window creation params
     * @return IWindow* window interface
     */
    virtual IWindow *CreateDeviceWindow( HWND              window,
                                         const OutputInfo &info ) = 0;

    virtual const DeviceLimitsInfo &GetLimits() = 0;

    virtual ICommandBuffer *GetMainCommandBuffer() = 0;

    virtual ICommandBuffer *CreateCommandBuffer() = 0;

    virtual ISyncPrimitive *CreateSyncPrimitive( SyncPrimitiveType type ) = 0;

    virtual IDescriptorSetLayout *CreateDescriptorSetLayout(
        const DescriptorSetLayoutCreateParams &params ) = 0;
    virtual IDescriptorSetAllocator *CreateDescriptorSetAllocator(
        const DescriptorSetAllocatorCreateParams &params ) = 0;
    virtual IPipelineLayout *
    CreatePipelineLayout( const PipelineLayoutCreateParams &params ) = 0;
    virtual IFrameBuffer *
    CreateFrameBuffer( const FrameBufferCreateParams &params ) = 0;
    virtual IRenderPass *
    CreateRenderPass( const RenderPassCreateParams &params ) = 0;
    virtual IPipeline *
    CreateRasterPipeline( const RasterPipelineCreateParams &params ) = 0;
    virtual IShader * CreateShader( const ShaderDesc &params )       = 0;
    virtual ISampler *CreateSampler( const SamplerDesc &params )     = 0;
    virtual IBuffer * CreateBuffer( const BufferCreateInfo &params ) = 0;
    virtual IImageBuffer *
    CreateImageBuffer( const ImageBufferCreateParams &params ) = 0;
    virtual IImageView *
    CreateImageView( const ImageViewCreateInfo &params ) = 0;
    virtual void
    UpdateDescriptorSets( const DescriptorSetUpdateInfo &params ) = 0;

    // Executes the command buffer on GPU, waits for waitFor sync primitive and
    // signals to signal sync primitive after execution
    virtual void ExecuteCommandBuffer( ICommandBuffer *buffer,
                                       ISyncPrimitive *waitFor,
                                       ISyncPrimitive *signal ) = 0;

    virtual void
    DispatchToGPU( const ArrayProxy<CommandBufferSubmitInfo> &buffers ) = 0;

    virtual void Wait( const ArrayProxy<ISyncPrimitive *> &primitiveList ) = 0;

    /**
     * @brief Waits for all remaining GPU work.
     */
    virtual void WaitForGPU() = 0;
};
} // namespace rh::engine
