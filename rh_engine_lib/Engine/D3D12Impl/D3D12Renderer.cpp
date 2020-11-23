#include "D3D12Renderer.h"

rh::engine::D3D12Renderer::D3D12Renderer( HWND window, HINSTANCE inst )
    : IRenderer( window, inst )
{}

rh::engine::D3D12Renderer::~D3D12Renderer() = default;

bool rh::engine::D3D12Renderer::InitDevice()
{
    return false;
}

bool rh::engine::D3D12Renderer::ShutdownDevice()
{
    return false;
}

bool rh::engine::D3D12Renderer::GetAdaptersCount( unsigned int & )
{
    return false;
}

bool rh::engine::D3D12Renderer::GetAdapterInfo( unsigned int /*n*/, rh::engine::String & )
{
    return false;
}

bool rh::engine::D3D12Renderer::GetOutputCount( unsigned int /*adapterId*/, int & )
{
    return false;
}

bool rh::engine::D3D12Renderer::SetCurrentOutput( unsigned int /*id*/ )
{
    return false;
}

bool rh::engine::D3D12Renderer::GetOutputInfo( unsigned int /*n*/, std::wstring & )
{
    return false;
}

bool rh::engine::D3D12Renderer::SetCurrentAdapter( unsigned int /*n*/ )
{
    return false;
}

bool rh::engine::D3D12Renderer::GetDisplayModeCount( unsigned int /*outputId*/, int & )
{
    return false;
}

bool rh::engine::D3D12Renderer::SetCurrentDisplayMode( unsigned int /*id*/ )
{
    return false;
}

bool rh::engine::D3D12Renderer::GetDisplayModeInfo( unsigned int /*id*/, DisplayModeInfo & )
{
    return false;
}

bool rh::engine::D3D12Renderer::Present( void * )
{
    return false;
}

void *rh::engine::D3D12Renderer::GetCurrentDevice()
{
    return nullptr;
}

void *rh::engine::D3D12Renderer::GetCurrentContext()
{
    return nullptr;
}

void rh::engine::D3D12Renderer::BindViewPorts( const std::vector<ViewPort> & /*viewports*/ ) {}

bool rh::engine::D3D12Renderer::GetCurrentAdapter( int & )
{
    return false;
}

bool rh::engine::D3D12Renderer::GetCurrentOutput( int & )
{
    return false;
}

bool rh::engine::D3D12Renderer::GetCurrentDisplayMode( int & )
{
    return false;
}

void *rh::engine::D3D12Renderer::AllocateImageBuffer( const ImageBufferInfo & /*info*/ )
{
    return nullptr;
}

bool rh::engine::D3D12Renderer::FreeImageBuffer( void * /*buffer*/, ImageBufferType /*type*/ )
{
    return false;
}

bool rh::engine::D3D12Renderer::BindImageBuffers( ImageBindType /*bindType*/,
                                                const std::vector<IndexPtrPair> & /*buffers*/ )
{
    return false;
}

bool rh::engine::D3D12Renderer::ClearImageBuffer( ImageClearType /*clearType*/,
                                                  void * /*buffer*/,
                                                  const std::array<float, 4> & )
{
    return false;
}

bool rh::engine::D3D12Renderer::BeginCommandList( void * /*cmdList*/ )
{
    return false;
}

bool rh::engine::D3D12Renderer::EndCommandList( void * /*cmdList*/ )
{
    return false;
}

bool rh::engine::D3D12Renderer::RequestSwapChainImage( void * /*frameBuffer*/ )
{
    return true;
}

bool rh::engine::D3D12Renderer::PresentSwapChainImage( void * /*frameBuffer*/ )
{
    return true;
}

void rh::engine::D3D12Renderer::FlushCache() {}

void rh::engine::D3D12Renderer::InitImGUI() {}

void rh::engine::D3D12Renderer::ImGUIStartFrame() {}

rh::engine::IGPUAllocator *rh::engine::D3D12Renderer::GetGPUAllocator()
{
    return nullptr;
}

void rh::engine::D3D12Renderer::ImGUIRender() {}

void rh::engine::D3D12Renderer::ShutdownImGUI() {}
