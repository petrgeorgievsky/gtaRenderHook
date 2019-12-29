#pragma once
#include "Engine/Common/IStateCacheObject.h"
#include "Engine/Common/types/rasterizer_state.h"
#include <unordered_map>

namespace rh::engine {
class IGPUAllocator;
enum class CullMode : uint8_t;
/**
 * @brief Depth stencil state cache object.
 *
 * This object is used to hold pointers to cached depth stencil states.
 */
class D3D11RasterizerStateCache : public IStateCacheObject
{
public:
    /**
   * @brief Construct a new D3D11DepthStencilStateCache object
   *
   */
    D3D11RasterizerStateCache() noexcept;

    ~D3D11RasterizerStateCache() noexcept override;

    void Init( IGPUAllocator *allocator );

    void SetCullMode( CullMode mode );

    /**
   * @brief Binds stencil state to DS stage of graphics pipeline
   *
   */
    void OnFlush( void * ) override;

private:
    RasterizerState m_rasterizerDescription{};
    IGPUAllocator *m_pAllocator = nullptr;
    std::unordered_map<RasterizerState, void *> m_aInternalStateData;
};
}; // namespace rh::engine
