#pragma once
#include "Engine/Common/IStateCacheObject.h"
#include "Engine/Common/types/blend_state.h"
#include <unordered_map>

namespace rh::engine {
class IGPUAllocator;
/**
 * @brief Blend state cache object.
 *
 * This object is used to hold pointers to cached blend states.
 TODO: TODODO
 */
class D3D11BlendStateCache : public IStateCacheObject
{
public:
    /**
   * @brief Construct a new D3D11DepthStencilStateCache object
   *
   */
    D3D11BlendStateCache() noexcept;

    ~D3D11BlendStateCache() noexcept override;

    void Init( IGPUAllocator *allocator );

    void SetAlphaTestEnable( bool enable );

    void SetBlendEnable( bool enable, uint32_t renderTargetId );

    void SetSrcBlendOp( BlendOp blendOp, uint32_t renderTargetId );

    void SetDestBlendOp( BlendOp blendOp, uint32_t renderTargetId );

    /**
   * @brief Binds stencil state to DS stage of graphics pipeline
   *
   */
    void OnFlush( void * ) override;

private:
    bool alphaTestEnabled = false;
    BlendState m_blendStateDescription{};
    IGPUAllocator *m_pAllocator = nullptr;
    std::unordered_map<BlendState, void *> m_aInternalStateData;
};
}; // namespace rh::engine
