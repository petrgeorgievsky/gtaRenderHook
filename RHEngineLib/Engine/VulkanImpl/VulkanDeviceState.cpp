#include "VulkanDeviceState.h"
#include "SyncPrimitives/VulkanCPUSyncPrimitive.h"
#include "SyncPrimitives/VulkanGPUSyncPrimitive.h"
#include "VulkanBuffer.h"
#include "VulkanCommandBuffer.h"
#include "VulkanCommon.h"
#include "VulkanDeviceOutputView.h"
#include "VulkanFrameBuffer.h"
#include "VulkanImageView.h"
#include "VulkanPipeline.h"
#include "VulkanRenderPass.h"
#include "VulkanShader.h"
#include "VulkanWin32Window.h"

#include <common.h>

using namespace rh;
using namespace rh::engine;

static VKAPI_ATTR VkBool32 VKAPI_CALL VkDebugCallback(
    VkDebugReportFlagsEXT /*flags*/, VkDebugReportObjectTypeEXT /*objType*/,
    uint64_t /*obj*/, size_t /*location*/, int32_t /*code*/,
    const char * /*layerPrefix*/, const char *msg, void * /*userData*/ )
{
    debug::DebugLogger::Log( ToRHString( msg ) );
    return VK_FALSE;
}

VkResult CreateDebugReportCallbackEXT(
    vk::Instance                              instance,
    const VkDebugReportCallbackCreateInfoEXT *pCreateInfo,
    VkDebugReportCallbackEXT *                pCallback )
{
    auto func = reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>(
        instance.getProcAddr( "vkCreateDebugReportCallbackEXT" ) );

    if ( func != nullptr )
        return func( static_cast<VkInstance>( instance ), pCreateInfo, nullptr,
                     pCallback );
    return VK_ERROR_EXTENSION_NOT_PRESENT;
}

void DestroyDebugReportCallbackEXT( vk::Instance                 instance,
                                    VkDebugReportCallbackEXT     callback,
                                    const VkAllocationCallbacks *pAllocator )
{
    auto func = reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>(
        instance.getProcAddr( "vkDestroyDebugReportCallbackEXT" ) );

    if ( func != nullptr )
        func( static_cast<VkInstance>( instance ), callback, pAllocator );
}

VulkanDeviceState::VulkanDeviceState()
{
    m_aExtensions.emplace_back( VK_KHR_SURFACE_EXTENSION_NAME );
    m_aExtensions.emplace_back( VK_KHR_WIN32_SURFACE_EXTENSION_NAME );
#ifndef NDEBUG
    m_aExtensions.emplace_back( VK_EXT_DEBUG_REPORT_EXTENSION_NAME );

    m_aLayers.emplace_back( "VK_LAYER_LUNARG_standard_validation" );
#endif
    m_aLayers.emplace_back( "VK_LAYER_LUNARG_monitor" );

    // App info
    vk::ApplicationInfo app_info{};
    app_info.pApplicationName = "Render Hook App";
    app_info.pEngineName      = "Render Hook Engine";

    // Instance info
    vk::InstanceCreateInfo inst_info{};
    inst_info.pApplicationInfo = &app_info;
    inst_info.enabledExtensionCount =
        static_cast<uint32_t>( m_aExtensions.size() );
    inst_info.ppEnabledExtensionNames = m_aExtensions.data();
    inst_info.enabledLayerCount   = static_cast<uint32_t>( m_aLayers.size() );
    inst_info.ppEnabledLayerNames = m_aLayers.data();

    // Create vulkan instance
    if ( !CALL_VK_API( vk::createInstance( &inst_info, nullptr, &m_vkInstance ),
                       TEXT( "VulkanRenderer failed to initialize: Failed to "
                             "initialize Vulkan Instance!" ) ) )
        return;

    // Enumerate GPUS
    m_aAdapters = m_vkInstance.enumeratePhysicalDevices();
    VkDebugReportCallbackCreateInfoEXT createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
    createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT
#ifndef NDEBUG
                       | VK_DEBUG_REPORT_WARNING_BIT_EXT |
                       VK_DEBUG_REPORT_INFORMATION_BIT_EXT |
                       VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT

#endif
        ;
    createInfo.pfnCallback = VkDebugCallback;

#ifndef NDEBUG
    if ( !CALL_VK_API(
             CreateDebugReportCallbackEXT( m_vkInstance, &createInfo,
                                           &m_debugCallback ),
             TEXT( "VulkanRenderer failed to initialize: Failed to create "
                   "debug callback!" ) ) )
        return;

#endif
    DISPLAY_DEVICE display_device{};
    display_device.cb = sizeof( DISPLAY_DEVICE );

    unsigned int display_id = 0;

    rh::debug::DebugLogger::Log( "DisplayDeviceInfo Enumeration:\t" );

    while ( EnumDisplayDevices( nullptr, display_id, &display_device, 0 ) !=
            false )
    {
        display_id++;

        StringStream ss;
        ss << "DisplayDeviceInfo:\t"
           << "\nDeviceName:\t" << display_device.DeviceName
           << "\nDeviceString:\t" << display_device.DeviceString;
        debug::DebugLogger::Log( ss.str() );

        if ( display_device.StateFlags & DISPLAY_DEVICE_ACTIVE )
        {
            DisplayInfo display_info = {display_device.DeviceName, {}};

            unsigned int display_mode_id = 0;

            DEVMODE device_mode{};
            device_mode.dmSize = sizeof( DEVMODE );

            debug::DebugLogger::Log( "DisplaySettings Enumeration:\t" );

            while ( EnumDisplaySettings( display_info.m_sDisplayName.c_str(),
                                         display_mode_id, &device_mode ) )
            {
                StringStream ss;
                ss << "DisplayModeInfo:\t"
                   << "\nBitsPerPixel\t" << device_mode.dmBitsPerPel
                   << "\nWidth\t" << device_mode.dmPelsWidth << "\nHeight:\t"
                   << device_mode.dmPelsHeight << "\nFrequency:\t"
                   << device_mode.dmDisplayFrequency;
                debug::DebugLogger::Log( ss.str() );

                display_info.m_aDisplayModes.push_back(
                    {static_cast<uint32_t>( device_mode.dmPelsWidth ),
                     static_cast<uint32_t>( device_mode.dmPelsHeight ),
                     static_cast<uint32_t>( device_mode.dmDisplayFrequency )} );

                display_mode_id++;
            }

            m_aDisplayInfos.push_back( display_info );
        }
    }
}

VulkanDeviceState::~VulkanDeviceState()
{
#ifndef NDEBUG
    DestroyDebugReportCallbackEXT( m_vkInstance, m_debugCallback, nullptr );
#endif
    m_vkInstance.destroy();
}

bool VulkanDeviceState::Init()
{
    auto queue_family_properties =
        m_aAdapters[m_uiCurrentAdapter].getQueueFamilyProperties();
    uint32_t i = 0;
    for ( auto queue_family : queue_family_properties )
    {
        if ( queue_family.queueFlags & vk::QueueFlagBits::eGraphics )
            m_iGraphicsQueueFamilyIdx = i;
        i++;
    }

    float       queuePriority[] = {1.0f};
    const char *devExtensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    vk::DeviceQueueCreateInfo queueCreateInfo = {};
    queueCreateInfo.queueFamilyIndex          = m_iGraphicsQueueFamilyIdx;
    queueCreateInfo.queueCount                = 1;
    queueCreateInfo.pQueuePriorities          = queuePriority;

    vk::DeviceCreateInfo info{};
    info.pQueueCreateInfos       = &queueCreateInfo;
    info.queueCreateInfoCount    = 1;
    info.enabledExtensionCount   = 1;
    info.ppEnabledExtensionNames = devExtensions;

    m_vkDevice = m_aAdapters[m_uiCurrentAdapter].createDevice( info );

    m_vkMainQueue = m_vkDevice.getQueue( m_iGraphicsQueueFamilyIdx, 0 );

    vk::CommandPoolCreateInfo cmd_pool_create_info{};
    cmd_pool_create_info.queueFamilyIndex = m_iGraphicsQueueFamilyIdx;
    cmd_pool_create_info.flags =
        vk::CommandPoolCreateFlagBits::eResetCommandBuffer;

    m_vkCommandPool = m_vkDevice.createCommandPool( cmd_pool_create_info );

    return true;
}

bool VulkanDeviceState::Shutdown()
{
    delete mMainCmdBuffer;
    mMainCmdBuffer = nullptr;
    m_vkDevice.destroyCommandPool( m_vkCommandPool );
    m_vkDevice.destroy();
    return true;
}

bool VulkanDeviceState::GetAdaptersCount( unsigned int &count )
{
    count = m_aAdapters.size();
    return true;
}

bool VulkanDeviceState::GetAdapterInfo( unsigned int id, String &info )
{
    if ( id >= m_aAdapters.size() )
        return false;

    info = ToRHString( m_aAdapters[id].getProperties().deviceName );
    return true;
}

bool VulkanDeviceState::GetCurrentAdapter( unsigned int &id )
{
    id = m_uiCurrentAdapter;
    return true;
}

bool VulkanDeviceState::SetCurrentAdapter( unsigned int id )
{
    if ( id > m_aAdapters.size() )
        return false;
    m_uiCurrentAdapter = id;
    return true;
}

bool VulkanDeviceState::GetOutputCount( unsigned int  adapterId,
                                        unsigned int &count )
{
    if ( adapterId > m_aAdapters.size() )
        return false;

    count = m_aDisplayInfos.size();
    return true;
}

bool VulkanDeviceState::GetOutputInfo( unsigned int id, String &info )
{
    if ( id >= m_aDisplayInfos.size() )
        return false;
    info = m_aDisplayInfos[id].m_sDisplayName;
    return true;
}

bool VulkanDeviceState::GetCurrentOutput( unsigned int &id )
{
    id = m_uiCurrentOutput;

    return true;
}

bool VulkanDeviceState::SetCurrentOutput( unsigned int id )
{
    if ( id >= m_aDisplayInfos.size() )
        return false;
    m_uiCurrentOutput = id;
    return true;
}

bool VulkanDeviceState::GetDisplayModeCount( unsigned int  outputId,
                                             unsigned int &count )
{
    count = m_aDisplayInfos[outputId].m_aDisplayModes.size();
    return true;
}

bool VulkanDeviceState::GetDisplayModeInfo( unsigned int     id,
                                            DisplayModeInfo &info )
{
    if ( id >= m_aDisplayInfos[m_uiCurrentOutput].m_aDisplayModes.size() )
        return false;
    info = m_aDisplayInfos[m_uiCurrentOutput].m_aDisplayModes[id];
    return true;
}

bool VulkanDeviceState::GetCurrentDisplayMode( unsigned int &id )
{
    id = m_uiCurrentAdapterMode;
    return true;
}

bool VulkanDeviceState::SetCurrentDisplayMode( unsigned int id )
{
    m_uiCurrentAdapterMode = id;
    return true;
}

IDeviceOutputView *
VulkanDeviceState::CreateDeviceOutputView( HWND window, const OutputInfo &info )
{
    auto display_mode =
        m_aDisplayInfos[m_uiCurrentOutput].m_aDisplayModes[info.displayModeId];
    RECT window_rect;
    window_rect.top    = 0;
    window_rect.left   = 0;
    window_rect.bottom = static_cast<LONG>( display_mode.height );
    window_rect.right  = static_cast<LONG>( display_mode.width );

    LONG style    = GetWindowLong( window, GWL_STYLE );
    LONG ex_style = GetWindowLong( window, GWL_EXSTYLE );
    auto res = AdjustWindowRectEx( &window_rect, static_cast<DWORD>( style ),
                                   GetMenu( window ) != nullptr,
                                   static_cast<DWORD>( ex_style ) );
    assert( res );
    SetWindowPos( window, nullptr, window_rect.left, window_rect.top,
                  window_rect.right - window_rect.left,
                  window_rect.bottom - window_rect.top, 6u );

    return new VulkanDeviceOutputView(
        window, m_vkInstance, m_aAdapters[m_uiCurrentAdapter], m_vkDevice,
        m_vkMainQueue, m_iGraphicsQueueFamilyIdx );
}

IWindow *VulkanDeviceState::CreateDeviceWindow( HWND              window,
                                                const OutputInfo &info )
{
    auto display_mode =
        m_aDisplayInfos[m_uiCurrentOutput].m_aDisplayModes[info.displayModeId];
    VulkanWin32WindowCreateParams create_params{};
    create_params.mGPU                  = m_aAdapters[m_uiCurrentAdapter];
    create_params.mInstance             = m_vkInstance;
    create_params.mDevice               = m_vkDevice;
    create_params.mPresentQueue         = m_vkMainQueue;
    create_params.mPresentQueueIdx      = m_iGraphicsQueueFamilyIdx;
    create_params.mWndHandle            = window;
    create_params.mWindowParams.mWidth  = display_mode.width;
    create_params.mWindowParams.mHeight = display_mode.height;

    return new VulkanWin32Window( create_params );
}

ISyncPrimitive *VulkanDeviceState::CreateSyncPrimitive( SyncPrimitiveType type )
{
    switch ( type )
    {
    case SyncPrimitiveType::GPU:
        return new VulkanGPUSyncPrimitive( m_vkDevice );
    case SyncPrimitiveType::CPU:
        return new VulkanCPUSyncPrimitive( m_vkDevice );
    }
    return nullptr;
}

IRenderPass *
VulkanDeviceState::CreateRenderPass( const RenderPassCreateParams &params )
{
    VulkanRenderPassCreateInfo create_info{};
    create_info.mDevice = m_vkDevice;
    create_info.mDesc   = params;
    return new VulkanRenderPass( create_info );
}

IShader *VulkanDeviceState::CreateShader( const ShaderDesc &params )
{
    VulkanShaderDesc desc{};
    desc.mDevice = m_vkDevice;
    desc.mDesc   = params;
    return new VulkanShader( desc );
}

ICommandBuffer *VulkanDeviceState::GetMainCommandBuffer()
{
    if ( mMainCmdBuffer != nullptr )
        return mMainCmdBuffer;

    vk::CommandBufferAllocateInfo cmd_buffer_alloc_info{};
    cmd_buffer_alloc_info.level              = vk::CommandBufferLevel::ePrimary;
    cmd_buffer_alloc_info.commandPool        = m_vkCommandPool;
    cmd_buffer_alloc_info.commandBufferCount = 1;
    auto cmd_buffer =
        m_vkDevice.allocateCommandBuffers( cmd_buffer_alloc_info );
    return ( mMainCmdBuffer =
                 new VulkanCommandBuffer( m_vkDevice, cmd_buffer[0] ) );
}

void VulkanDeviceState::ExecuteCommandBuffer( ICommandBuffer *buffer,
                                              ISyncPrimitive *waitFor,
                                              ISyncPrimitive *signal )
{
    auto vk_cmd_buffer_ptr = static_cast<VulkanCommandBuffer *>( buffer );
    auto cmd_buffer        = vk_cmd_buffer_ptr->GetBuffer();
    std::vector<vk::Semaphore> signal_semaphores;
    std::vector<vk::Semaphore> wait_semaphores;
    signal_semaphores.reserve( 1 );
    wait_semaphores.reserve( 1 );

    vk::SubmitInfo queue_submit_info{};
    queue_submit_info.commandBufferCount = 1;
    queue_submit_info.pCommandBuffers    = &cmd_buffer;

    if ( waitFor != nullptr )
    {
        std::array<vk::PipelineStageFlags, 1> stage_flags{
            vk::PipelineStageFlagBits::eTopOfPipe};
        auto vk_wait_for_sp = dynamic_cast<VulkanGPUSyncPrimitive *>( waitFor );
        assert( vk_wait_for_sp );
        wait_semaphores.emplace_back(
            static_cast<vk::Semaphore>( *vk_wait_for_sp ) );
        queue_submit_info.waitSemaphoreCount = wait_semaphores.size();
        queue_submit_info.pWaitSemaphores    = wait_semaphores.data();
        queue_submit_info.pWaitDstStageMask  = stage_flags.data();
    }

    if ( signal != nullptr )
    {
        auto vk_signal_sp = dynamic_cast<VulkanGPUSyncPrimitive *>( signal );
        assert( vk_signal_sp );
        signal_semaphores.emplace_back(
            static_cast<vk::Semaphore>( *vk_signal_sp ) );
        queue_submit_info.signalSemaphoreCount = signal_semaphores.size();
        queue_submit_info.pSignalSemaphores    = signal_semaphores.data();
    }
    auto vk_cmd_buf_exec_sp = dynamic_cast<VulkanCPUSyncPrimitive *>(
        vk_cmd_buffer_ptr->ExecutionFinishedPrimitive() );

    m_vkMainQueue.submit( {queue_submit_info},
                          vk_cmd_buf_exec_sp
                              ? static_cast<vk::Fence>( *vk_cmd_buf_exec_sp )
                              : nullptr );
}

IFrameBuffer *
VulkanDeviceState::CreateFrameBuffer( const FrameBufferCreateParams &params )
{
    std::vector<vk::ImageView> image_views;
    image_views.reserve( params.imageViews.size() );

    std::transform( params.imageViews.begin(), params.imageViews.end(),
                    std::back_inserter( image_views ),
                    []( IImageView *img_view_ptr ) {
                        auto impl_img_view =
                            dynamic_cast<VulkanImageView *>( img_view_ptr );
                        return static_cast<vk::ImageView>( *impl_img_view );
                    } );

    auto render_pass_impl =
        dynamic_cast<VulkanRenderPass *>( params.renderPass );
    return new VulkanFrameBuffer( m_vkDevice, image_views, params.width,
                                  params.height, *render_pass_impl );
}

IPipeline *
VulkanDeviceState::CreatePipeline( const PipelineCreateParams &params )
{
    VulkanPipelineCreateInfo vk_pipe_ci{};
    vk_pipe_ci.mDevice = m_vkDevice;
    vk_pipe_ci.mRenderPass =
        *dynamic_cast<VulkanRenderPass *>( params.mRenderPass );
    vk_pipe_ci.mShaderStages = params.mShaderStages;
    return new VulkanPipeline( vk_pipe_ci );
}

IBuffer *VulkanDeviceState::CreateBuffer( const BufferCreateInfo &params )
{
    VulkanBufferCreateInfo create_info = {params};
    create_info.mDevice                = m_vkDevice;
    return new VulkanBuffer( create_info );
}

void VulkanDeviceState::Wait(
    const std::vector<ISyncPrimitive *> &primitiveList )
{
    std::vector<vk::Fence> fenceList;
    std::transform( primitiveList.begin(), primitiveList.end(),
                    std::back_inserter( fenceList ),
                    []( ISyncPrimitive *fence ) -> vk::Fence {
                        auto fence_impl =
                            dynamic_cast<VulkanCPUSyncPrimitive *>( fence );
                        return *fence_impl;
                    } );
    m_vkDevice.waitForFences( fenceList, true, ~0 );
    m_vkDevice.resetFences( fenceList );
}