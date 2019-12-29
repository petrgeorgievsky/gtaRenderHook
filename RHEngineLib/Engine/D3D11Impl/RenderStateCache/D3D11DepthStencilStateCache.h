#pragma once
#include "Engine/Common/IStateCacheObject.h"
#include "Engine/Common/types/depth_stencil_state.h"
#include <unordered_map>

namespace rh::engine {
class IGPUAllocator;

/**
 * @brief Depth stencil state cache object.
 *
 * This object is used to hold pointers to cached depth stencil states.
 */
class D3D11DepthStencilStateCache : public IStateCacheObject
{
public:
    /**
   * @brief Construct a new D3D11DepthStencilStateCache object
   *
   */
    D3D11DepthStencilStateCache() noexcept;

    ~D3D11DepthStencilStateCache() noexcept override;

    void Init( IGPUAllocator *allocator );

    void SetDepthEnable( bool enable );

    void SetDepthWriteEnable( bool enable );

    /**
   * @brief Binds stencil state to DS stage of graphics pipeline
   *
   */
    void OnFlush( void * ) override;

private:
    DepthStencilState m_depthStencilDescription{};
    IGPUAllocator *m_pAllocator = nullptr;
    std::unordered_map<DepthStencilState, void *> m_aInternalStateData;
};
}; // namespace rh::engine
