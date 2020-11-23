#pragma once
#include "Common/IDeviceState.h"
#include "Common/IGPUAllocator.h"
#include "Common/types/index_ptr_pair.h"
#include "Common/types/string_typedefs.h"
#include "Common/types/viewport.h"
#include <vector>
#include <memory>

namespace rh::engine {
enum class ImageBindType : unsigned char;
enum class ImageClearType : unsigned char;
/**
        \brief Renderer interface for API-dependant renderers

        This abstraction clusterfuck is created to be able to implement other
   wrappers for APIs
@todo REMOVE IT
    */
class IRenderer
{
public:
    /**
   * @brief Construct a new IRenderer object
   *
   * @param window - window handle
   * @param inst - instance handle
   */
    IRenderer( HWND window, HINSTANCE inst ) noexcept
    {
        m_hWnd = window;
        m_hInst = inst;
    }

    /**
   * @brief Destroy the IRenderer object
   *
   */
    virtual ~IRenderer() noexcept = default;

    /**
   * @brief Initializes rendering device and main swap-chain.
   *
   * @return true if initialization was succesful
   */
    virtual bool InitDevice() = 0;

    /**
   * @brief Releases resources held by rendering device
   *
   * @return true if everything went well
   */
    virtual bool ShutdownDevice() = 0;

    /*
Allocates image buffer and returns pointer to allocated buffer.
*/
    virtual void *AllocateImageBuffer( const ImageBufferInfo &info ) = 0;

    /*
Frees image buffer allocated by this renderer.
*/
    virtual bool FreeImageBuffer( void *buffer, ImageBufferType type ) = 0;

    /*
Binds image buffers to specific pipeline stage.
*/
    virtual bool BindImageBuffers( ImageBindType bindType, const std::vector<IndexPtrPair> &buffers )
        = 0;
    virtual bool ClearImageBuffer( ImageClearType clearType,
                                   void *buffer,
                                   const std::array<float, 4> &clearColor )
        = 0;
    virtual bool Present( void *image ) = 0;
    virtual bool BeginCommandList( void *cmdList ) = 0;
    virtual bool EndCommandList( void *cmdList ) = 0;
    virtual bool RequestSwapChainImage( void *frameBuffer ) = 0;
    virtual bool PresentSwapChainImage( void *frameBuffer ) = 0;
    virtual IGPUAllocator *GetGPUAllocator() = 0;
    virtual IDeviceState &GetDeviceState() = 0;
    virtual void *GetCurrentDevice() = 0;
    virtual void *GetCurrentContext() = 0;
    virtual void BindViewPorts( const std::vector<ViewPort> &viewports ) = 0;
    virtual void FlushCache() = 0;

    virtual void InitImGUI() = 0;
    virtual void ShutdownImGUI() = 0;

    virtual void ImGUIStartFrame() = 0;
    virtual void ImGUIRender() = 0;

protected:
    HWND m_hWnd = nullptr;
    HINSTANCE m_hInst = nullptr;
};

extern std::unique_ptr<IRenderer> g_pRHRenderer;
} // namespace rh::engine
