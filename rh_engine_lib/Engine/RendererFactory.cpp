#include "RendererFactory.h"
#include "D3D11Impl/D3D11Im2DPipeline.h"
#include "D3D11Impl/D3D11Renderer.h"
//#include "D3D12Impl/D3D12Renderer.h"
//#include "VulkanImpl/VulkanRenderer.h"

using namespace rh::engine;

std::unique_ptr<IRenderer>
RendererFactory::CreateRenderer( RenderingAPI api, HWND window, HINSTANCE inst )
{
    switch ( api )
    {
    case RenderingAPI::DX11:
        return std::make_unique<D3D11Renderer>( window, inst );
    case RenderingAPI::Vulkan:
        return nullptr; // std::make_unique<VulkanRenderer>( window, inst );
    }
    return nullptr;
}

std::unique_ptr<ISimple2DRenderer>
RendererFactory::CreateSimple2DRenderer( RenderingAPI api )
{
    switch ( api )
    {
    case RenderingAPI::DX11:
        return std::make_unique<D3D11Im2DPipeline>(
            *g_pRHRenderer->GetGPUAllocator() );
    default: return nullptr;
    }
}
