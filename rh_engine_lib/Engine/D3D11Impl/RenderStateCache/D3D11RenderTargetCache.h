#pragma once
#include "Engine/Common/IStateCacheObject.h"
#include <vector>

namespace rh::engine {
class D3D11BindableResource;
struct IndexPtrPair;

constexpr auto D3D11_MAX_RENDER_TARGET_COUNT = 8;

/**
 * @brief Render targets state cache object.
 *
 * This object is used to hold pointers to cached render targets and depth
 * stencil buffer.
 */
class D3D11RenderTargetCache : public IStateCacheObject
{
public:
    /**
   * @brief Construct a new D3D11RenderTargetCache object
   *
   */
    D3D11RenderTargetCache() noexcept;
    ~D3D11RenderTargetCache() noexcept override = default;

    /**
   * @brief Set the Render Targets and DepthStencil buffer to Output-Merger
   * stage of graphics pipeline
   *
   * @param renderTargets - list of render target viewables mapped to slot
   * number in OM stage
   * @param depthStencilView - depth stencil viewable object
   */
    void SetRenderTargets( const std::vector<IndexPtrPair> &renderTargets, void *depthStencilView );

    /**
   * @brief Set the Render Targets to Output-Merger stage of graphics pipeline
   *
   * @param renderTargets - list of render target viewables mapped to slot
   * number in OM stage
   */
    void SetRenderTargets( const std::vector<IndexPtrPair> &renderTargets );

    /**
   * @brief Set the Depth Stencil buffer to Output-Merger stage of graphics
   * pipeline
   *
   * @param depthStencilView - depth stencil viewable object
   */
    void SetDepthStencilTarget( void *depthStencilView );

    /**
   * @brief Binds render targets to OM stage of graphics pipeline
   *
   */
    void OnFlush( void * ) override;

private:
    D3D11BindableResource
        *m_aRenderTargetViews[D3D11_MAX_RENDER_TARGET_COUNT]{};
    D3D11BindableResource *m_pDepthStencilView = nullptr;
};
} // namespace rh::engine
