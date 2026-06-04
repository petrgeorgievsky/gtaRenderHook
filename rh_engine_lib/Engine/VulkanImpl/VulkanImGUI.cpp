//
// Created by peter on 12.07.2020.
//

#include "VulkanImGUI.h"
#include "DebugUtils/DebugLogger.h"
#include "VulkanCommandBuffer.h"
#include "VulkanRenderPass.h"
#include <imgui.h>
#include <imgui_impl_vulkan.h>
// #include <imgui_impl_win32.h>

extern LRESULT ImGuiImplWin32WndProcHandler( HWND hwnd, UINT msg,
                                             WPARAM                  w_param,
                                             [[maybe_unused]] LPARAM l_param );

namespace rh::engine
{
// static HHOOK g_hImGuiHook = nullptr;

// Returns the last Win32 error, in string format. Returns an empty string if
// there is no error.
std::string GetLastErrorAsString()
{
    // Get the error message, if any.
    DWORD errorMessageID = ::GetLastError();
    if ( errorMessageID == 0 )
        return std::string(); // No error message has been recorded

    LPSTR  messageBuffer = nullptr;
    size_t size          = FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, errorMessageID, MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),
        (LPSTR)&messageBuffer, 0, NULL );

    std::string message( messageBuffer, size );

    // Free the buffer.
    LocalFree( messageBuffer );

    return message;
}

VulkanImGUI::VulkanImGUI( const VulkanImGUIStartParams &params )
{
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    /*auto hinst = GetWindowLong( params.mWindow, -6 );
    assert( hinst != 0 );
    if ( hinst == 0 )
    {
        std::stringstream ss;
        ss << "Failed to get hwnd hinst! Error code:" << GetLastError()
           << "\n Msg:" << GetLastErrorAsString();

        debug::DebugLogger::Log( ss.str(), debug::LogLevel::Error );
    }*/
    /*g_hImGuiHook = SetWindowsHookEx(
        WH_GETMESSAGE, ImGuiMsgProc, GetModuleHandle( nullptr ),
        GetWindowThreadProcessId( params.mWindow, nullptr ) );
    if ( g_hImGuiHook == nullptr )
    {

        std::stringstream ss;
        ss << "Failed to create imgui hook! Error code:" << GetLastError()
           << "\n Msg:" << GetLastErrorAsString();

        debug::DebugLogger::Log( ss.str(), debug::LogLevel::Error );
    }
    assert( g_hImGuiHook );*/
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable
    // Keyboard Controls io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad; //
    // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // ImGui_ImplWin32_Init( params.mWindow );
    mInstance       = params.Instance;
    mPhysicalDevice = params.PhysicalDevice;
    mDevice         = params.Device;
    mQueueFamily    = params.QueueFamily;
    mQueue          = params.Queue;

    std::array pools = {
        VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
        VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
        VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
        VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
        VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
        VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
        VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
        VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
        VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
        VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
        VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 } };

    VkDescriptorPoolCreateInfo poolCreateInfo{
        VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        nullptr,
        VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
        2000,
        static_cast<uint32_t>( pools.size() ),
        pools.data() };

    vkCreateDescriptorPool( mDevice, &poolCreateInfo, nullptr,
                            &mDescriptorPool );
}

void VulkanImGUI::Init( const VulkanImGUIInitParams &params )
{

    ImGui_ImplVulkan_InitInfo init_info = {};
#if IMGUI_VERSION_NUM >= 19200
#ifdef VK_API_VERSION_1_3
    init_info.ApiVersion = VK_API_VERSION_1_3;
#else
    init_info.ApiVersion = VK_API_VERSION_1_0;
#endif
#endif
    init_info.Instance        = mInstance;
    init_info.PhysicalDevice  = mPhysicalDevice;
    init_info.Device          = mDevice;
    init_info.QueueFamily     = mQueueFamily;
    init_info.Queue           = mQueue;
    init_info.PipelineCache   = VK_NULL_HANDLE;
    init_info.DescriptorPool  = mDescriptorPool;
    init_info.Allocator       = VK_NULL_HANDLE;
    init_info.CheckVkResultFn = []( VkResult ) {};
    init_info.MinImageCount   = 2;
    init_info.ImageCount      = 2;

    auto render_pass_impl =
        dynamic_cast<VulkanRenderPass *>( params.mRenderPass );

    auto render_pass = static_cast<VkRenderPass>(
        static_cast<vk::RenderPass>( *render_pass_impl ) );

#if IMGUI_VERSION_NUM >= 19270
    init_info.PipelineInfoMain.RenderPass  = render_pass;
    init_info.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    ImGui_ImplVulkan_Init( &init_info );
#elif IMGUI_VERSION_NUM >= 19010
    init_info.RenderPass  = render_pass;
    init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    ImGui_ImplVulkan_Init( &init_info );
#else
    ImGui_ImplVulkan_Init( &init_info, render_pass );
#endif
}
bool VulkanImGUI::UploadFonts( ICommandBuffer *cmd_buff )
{
    if ( mFontsUploaded )
        return true;

#if IMGUI_VERSION_NUM < 19010
    auto command_buffer = reinterpret_cast<VulkanCommandBuffer *>( cmd_buff );
    mFontsUploaded =
        ImGui_ImplVulkan_CreateFontsTexture( command_buffer->GetBuffer() );
#elif IMGUI_VERSION_NUM < 19200
    (void)cmd_buff;
    mFontsUploaded = ImGui_ImplVulkan_CreateFontsTexture();
#else
    (void)cmd_buff;
    // Since Dear ImGui 1.92, the Vulkan backend owns font texture updates and
    // uploads the atlas from NewFrame()/RenderDrawData() as needed.
    mFontsUploaded = true;
#endif
    return mFontsUploaded;
}
void VulkanImGUI::DrawGui( ICommandBuffer *cmd_buff )
{
    ImGui::Render();

    auto command_buffer = reinterpret_cast<VulkanCommandBuffer *>( cmd_buff );
    ImGui_ImplVulkan_RenderDrawData( ImGui::GetDrawData(),
                                     command_buffer->GetBuffer() );
}
void VulkanImGUI::BeginFrame()
{
    ImGui_ImplVulkan_NewFrame();
    // ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
}
VulkanImGUI::~VulkanImGUI()
{
    ImGui_ImplVulkan_Shutdown();
    vkDestroyDescriptorPool( mDevice, mDescriptorPool, nullptr );
    // ImGui_ImplWin32_Shutdown();
}
} // namespace rh::engine