#pragma once
#include "Engine/Common/IStateCacheObject.h"
#include <unordered_map>
#include <vector>

namespace rh::engine
{
class D3D11BindableResource;
struct IndexPtrPair;
enum ShaderStage : uint32_t;

constexpr auto D3D11_INPUT_RESOURCE_SLOT_COUNT = 128;
constexpr auto D3D11_UAV_SLOT_COUNT            = 64;

/**
 * @brief Shader resource cache object.
 *
 * This object is used to hold pointers to cached shader resources.
 */
class D3D11ShaderResourceCache : public IStateCacheObject
{
  public:
    /**
     * @brief Construct a new D3D11RenderTargetCache object
     *
     */
    D3D11ShaderResourceCache() noexcept;

    /**
     * @brief Set Resources to Shader stages of graphics pipeline
     *
     * @param resource_views - list of shader resource viewables mapped to slot
     * number in shader stage
     * @param stage - shader stage
     */
    void SetShaderResources( const std::vector<IndexPtrPair> &resource_views,
                             ShaderStage                      stage );

    /**
     * @brief Set UAV Resources to Shader stages of graphics pipeline
     *
     * @param resource_views - list of shader resource viewables mapped to slot
     * number in shader stage
     * @param stage - shader stage
     */
    void SetUnorderedAccessResources(
        const std::vector<IndexPtrPair> &resource_views, ShaderStage stage );

    /**
     * @brief Binds resources to shader stages of graphics pipeline
     *
     */
    void OnFlush( void * ) override;

    void Invalidate() override;

  private:
    D3D11BindableResource
        *m_aPSResourceViews[D3D11_INPUT_RESOURCE_SLOT_COUNT]{};
    D3D11BindableResource
        *m_aCSResourceViews[D3D11_INPUT_RESOURCE_SLOT_COUNT]{};
    D3D11BindableResource
        *m_aGSResourceViews[D3D11_INPUT_RESOURCE_SLOT_COUNT]{};
    D3D11BindableResource
        *m_aVSResourceViews[D3D11_INPUT_RESOURCE_SLOT_COUNT]{};
    D3D11BindableResource
        *m_aDSResourceViews[D3D11_INPUT_RESOURCE_SLOT_COUNT]{};
    D3D11BindableResource
        *m_aHSResourceViews[D3D11_INPUT_RESOURCE_SLOT_COUNT]{};
    D3D11BindableResource *m_aCSUAResourceViews[D3D11_UAV_SLOT_COUNT]{};

    uint32_t m_nDirtyStage     = 0;
    uint32_t m_nMaxDirtyIdPS   = 0;
    uint32_t m_nMaxDirtyIdVS   = 0;
    uint32_t m_nMaxDirtyIdGS   = 0;
    uint32_t m_nMaxDirtyIdHS   = 0;
    uint32_t m_nMaxDirtyIdDS   = 0;
    uint32_t m_nMaxDirtyIdCS   = 0;
    uint32_t m_nMaxDirtyIdUACS = D3D11_UAV_SLOT_COUNT - 1;

    uint32_t m_nMinDirtyIdPS   = D3D11_INPUT_RESOURCE_SLOT_COUNT - 1;
    uint32_t m_nMinDirtyIdVS   = D3D11_INPUT_RESOURCE_SLOT_COUNT - 1;
    uint32_t m_nMinDirtyIdGS   = D3D11_INPUT_RESOURCE_SLOT_COUNT - 1;
    uint32_t m_nMinDirtyIdHS   = D3D11_INPUT_RESOURCE_SLOT_COUNT - 1;
    uint32_t m_nMinDirtyIdDS   = D3D11_INPUT_RESOURCE_SLOT_COUNT - 1;
    uint32_t m_nMinDirtyIdCS   = D3D11_INPUT_RESOURCE_SLOT_COUNT - 1;
    uint32_t m_nMinDirtyIdUACS = D3D11_UAV_SLOT_COUNT - 1;
};
} // namespace rh::engine
