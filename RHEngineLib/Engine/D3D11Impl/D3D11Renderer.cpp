#include "D3D11Renderer.h"
#include "D3D11Common.h"
#include "D3D11Convert.h"
#include "Engine/Common/IGPUAllocator.h"
#include "ImGUI/imgui_impl_dx11.h"
#include "ImGUI/imgui_impl_win32.h"
#include "ImageBuffers\D3D11BackBuffer.h"
#include "ImageBuffers\D3D11DepthStencilBuffer.h"
#include "ImageBuffers\D3D11Texture2D.h"
#include <common.h>

rh::engine::D3D11Renderer::D3D11Renderer( HWND window, HINSTANCE inst )
    : IRenderer( window, inst )
{
    rh::debug::DebugLogger::Log( "D3D11Renderer initialization..." );
    RH_ASSERT( window != nullptr )
    RH_ASSERT( inst != nullptr )
}

rh::engine::D3D11Renderer::~D3D11Renderer()
{
    rh::debug::DebugLogger::Log( "D3D11Renderer destructor..." );
}

bool rh::engine::D3D11Renderer::InitDevice()
{
    rh::debug::DebugLogger::Log( "D3D11Renderer device initialization..." );

    if ( !m_pDeviceState.Init() ) {
        rh::debug::DebugLogger::Error( "DeviceState initialization failed!" );
        return false;
    }

    uint32_t currentDisplayMode = 0;
    if ( !m_pDeviceState.GetCurrentDisplayMode( currentDisplayMode ) ) {
        rh::debug::DebugLogger::Error( "Failed to retrieve current display mode!" );
        return false;
    }
    DisplayModeInfo displ_info;
    if ( !m_pDeviceState.GetDisplayModeInfo( currentDisplayMode, displ_info ) ) {
        rh::debug::DebugLogger::Error( "Failed to retrieve current display mode info!" );
        return false;
    }

    m_pOutputView = reinterpret_cast<D3D11DeviceOutputView *>(
        m_pDeviceState.CreateDeviceOutputView( m_hWnd, {currentDisplayMode, true} ) );
    m_gpuMemoryAllocator = new D3D11GPUAllocator();
    m_mainRenderingContext = new D3D11RenderingContext();
    if ( m_pOutputView == nullptr ) {
        rh::debug::DebugLogger::Error( "Failed to allocate output view!" );
        return false;
    }
    m_pDeviceState.InitAllocator( *m_gpuMemoryAllocator, m_pOutputView );
    m_pDeviceState.InitRenderingContext( *m_mainRenderingContext, m_gpuMemoryAllocator, false );

    m_pOutputView->Resize( m_gpuMemoryAllocator, displ_info.height, displ_info.width );

    return true;
}

bool rh::engine::D3D11Renderer::ShutdownDevice()
{
    delete m_pOutputView;
    delete m_mainRenderingContext;
    delete m_gpuMemoryAllocator;
    return m_pDeviceState.Shutdown();
}

bool rh::engine::D3D11Renderer::Present( void * /*image*/ )
{
    return m_pOutputView->Present();
}

void *rh::engine::D3D11Renderer::AllocateImageBuffer( const ImageBufferInfo &info )
{
    IGPUResource *buffer_ptr = nullptr;
    if ( !m_gpuMemoryAllocator->AllocateImageBuffer( info, buffer_ptr ) )
        return nullptr;
    return buffer_ptr;
}

bool rh::engine::D3D11Renderer::FreeImageBuffer( void *buffer, ImageBufferType type )
{
    return m_gpuMemoryAllocator->FreeImageBuffer( buffer, type );
}

bool rh::engine::D3D11Renderer::BindImageBuffers( ImageBindType bindType,
                                                const std::vector<IndexPtrPair> &buffers )
{
    return m_mainRenderingContext->BindImageBuffers( bindType, buffers );
}

bool rh::engine::D3D11Renderer::ClearImageBuffer( ImageClearType clearType,
                                                  void *buffer,
                                                  const std::array<float, 4> &clearColor )
{
    return m_mainRenderingContext->ClearImageBuffer( clearType, buffer, clearColor );
}

void *rh::engine::D3D11Renderer::GetCurrentDevice()
{
    return m_gpuMemoryAllocator->GetDevice();
}

void *rh::engine::D3D11Renderer::GetCurrentContext()
{
    return m_mainRenderingContext;
}

void rh::engine::D3D11Renderer::BindViewPorts( const std::vector<ViewPort> &viewports )
{
    m_mainRenderingContext->BindViewPorts( viewports );
}

bool rh::engine::D3D11Renderer::BeginCommandList( void * /*cmdList*/ )
{
    return true;
}

bool rh::engine::D3D11Renderer::EndCommandList( void * /*cmdList*/ )
{
    return false;
}

bool rh::engine::D3D11Renderer::RequestSwapChainImage( void * /*frameBuffer*/ )
{
    return true;
}

bool rh::engine::D3D11Renderer::PresentSwapChainImage( void * /*frameBuffer*/ )
{
    return false;
}

void rh::engine::D3D11Renderer::InitImGUI()
{
    ImGui_ImplWin32_Init( m_hWnd );
    ImGui_ImplDX11_Init( m_gpuMemoryAllocator->GetDevice(),
                         m_mainRenderingContext->GetContextImpl() );
}

void rh::engine::D3D11Renderer::ImGUIStartFrame()
{
    ImGui_ImplDX11_NewFrame();
}

void rh::engine::D3D11Renderer::ImGUIRender()
{
    ImGui_ImplDX11_RenderDrawData( ImGui::GetDrawData() );
}

rh::engine::IGPUAllocator *rh::engine::D3D11Renderer::GetGPUAllocator()
{
    return m_gpuMemoryAllocator;
}

rh::engine::IDeviceState &rh::engine::D3D11Renderer::GetDeviceState()
{
    return m_pDeviceState;
}

D3D_FEATURE_LEVEL rh::engine::D3D11Renderer::GetFeatureLevel() const
{
    return m_pDeviceState.GetFeatureLevel();
}

void rh::engine::D3D11Renderer::FlushCache()
{
    m_mainRenderingContext->FlushCache();
}

void rh::engine::D3D11Renderer::ShutdownImGUI()
{
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
}
