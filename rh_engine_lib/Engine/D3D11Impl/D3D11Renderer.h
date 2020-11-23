#pragma once
#include "D3D11DeviceOutputView.h"
#include "D3D11DeviceState.h"
#include "D3D11GPUAllocator.h"
#include "D3D11RenderingContext.h"
#include "Engine/Definitions.h"
#include "Engine/IRenderer.h"
#include "RenderStateCache/D3D11RenderStateCache.h"

namespace rh::engine {
class D3D11Renderer final : public IRenderer
{
public:
    D3D11Renderer( HWND window, HINSTANCE inst );
    ~D3D11Renderer() override;
    bool InitDevice() override;
    bool ShutdownDevice() override;
    bool Present( void *image ) override;

    void *AllocateImageBuffer( const ImageBufferInfo &info ) override;

    bool FreeImageBuffer( void *buffer, ImageBufferType type ) override;

    bool BindImageBuffers( ImageBindType bindType,
                           const std::vector<IndexPtrPair> &buffers ) override;

    bool ClearImageBuffer( ImageClearType clearType,
                           void *buffer,
                           const std::array<float, 4> &clearColor ) override;

    void *GetCurrentDevice() override;
    void *GetCurrentContext() override;
    void BindViewPorts( const std::vector<ViewPort> &viewports ) override;
    void FlushCache() override;

    void InitImGUI() override;
    void ImGUIStartFrame() override;
    void ImGUIRender() override;
    void ShutdownImGUI() override;
    IGPUAllocator *GetGPUAllocator() override;
    IDeviceState &GetDeviceState() override;

    [[nodiscard]] D3D_FEATURE_LEVEL GetFeatureLevel() const;

private:
    D3D11DeviceState m_pDeviceState;
    D3D11DeviceOutputView *m_pOutputView = nullptr;
    D3D11GPUAllocator *m_gpuMemoryAllocator = nullptr;
    D3D11RenderingContext *m_mainRenderingContext = nullptr;

    bool BeginCommandList( void *cmdList ) override;
    bool EndCommandList( void *cmdList ) override;
    bool RequestSwapChainImage( void *frameBuffer ) override;
    bool PresentSwapChainImage( void *frameBuffer ) override;
};
}; // namespace rh::engine
