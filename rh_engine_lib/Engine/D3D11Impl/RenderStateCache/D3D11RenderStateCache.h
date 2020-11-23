#pragma once
#include "D3D11BlendStateCache.h"
#include "D3D11DepthStencilStateCache.h"
#include "D3D11RasterizerStateCache.h"
#include "D3D11RenderTargetCache.h"
#include "D3D11SampleStateCache.h"
#include "D3D11ShaderResourceCache.h"

// d3d11 struct forwards:
struct ID3D11DeviceContext;

namespace rh::engine {

/**
 * @brief D3D11 render state cache class,
 * used to interact with graphics pipeline states
 * caching them to reduce CPU overhead
 *
 */
class D3D11RenderStateCache
{
public:
    /**
   * @brief Returns render target cache object
   *
   * @return D3D11RenderTargetCache
   */
    D3D11RenderTargetCache *GetRTCache() { return &m_rtCache; }

    /**
   * @brief Returns shader resource cache object
   *
   * @return D3D11ShaderResourceCache
   */
    D3D11ShaderResourceCache *GetSRCache() { return &m_shaderResourceCache; }

    /**
   * @brief Returns sampler state cache object
   *
   * @return D3D11SamplerStateCache
   */
    D3D11SamplerStateCache *GetSamplerStateCache() { return &m_samplerStateCache; }

    /**
   * @brief Returns depth stencil state cache object
   *
   * @return D3D11DepthStencilStateCache
   */
    D3D11DepthStencilStateCache *GetDepthStencilStateCache() { return &m_depthStencilStateCache; }

    /**
   * @brief Returns blend state cache object
   *
   * @return D3D11BlendStateCache
   */
    D3D11BlendStateCache *GetBlendStateCache() { return &m_blendStateCache; }

    /**
   * @brief Returns rasterizer state cache object
   *
   * @return D3D11BlendStateCache
   */
    D3D11RasterizerStateCache *GetRasterizerStateCache() { return &m_rasterizerStateCache; }

    /**
   * @brief Immediatly sends cached render targets to GPU
   *
   * @param context - device context
   *
   * *Use this method if you want to use resources of binded render targets in
   * next calls
   */
    void FlushRenderTargets( ID3D11DeviceContext *context );

    /**
   * @brief Sends all cached render states to GPU
   *
   * @param context - device context
   */
    void Flush( ID3D11DeviceContext *context );

    void Init( IGPUAllocator *allocator );

    void Invalidate();

private:
    D3D11RenderTargetCache m_rtCache;
    D3D11ShaderResourceCache m_shaderResourceCache;
    D3D11SamplerStateCache m_samplerStateCache;
    D3D11DepthStencilStateCache m_depthStencilStateCache;
    D3D11BlendStateCache m_blendStateCache;
    D3D11RasterizerStateCache m_rasterizerStateCache;
};
} // namespace rh::engine
