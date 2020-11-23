#pragma once
#include "Engine/Common/IStateCacheObject.h"
#include "Engine/Common/types/sampler.h"
#include <array>
#include <unordered_map>
#include <vector>

// d3d11 struct forwards:
struct ID3D11SamplerState;

namespace rh::engine
{
class D3D11BindableResource;
struct IndexPtrPair;
enum ShaderStage : uint32_t;
uint64_t GenerateSamplerHash( void *sampler_ptr );
Sampler  GetSamplerFromHash( uint64_t hash );

constexpr auto D3D11_SAMPLER_SLOT_COUNT = 16;
/**
 * @brief Sampler state cache object.
 *
 * This object is used to hold pointers to cached shader resources.
 */
class D3D11SamplerStateCache : public IStateCacheObject
{
  public:
    /**
     * @brief Construct a new D3D11RenderTargetCache object
     *
     */
    D3D11SamplerStateCache() noexcept;

    ~D3D11SamplerStateCache() noexcept override;

    /**
     * @brief Set Resources to Shader stages of graphics pipeline
     *
     * @param resource_views - list of shader resource viewables mapped to slot
     * number in shader stage
     * @param stage - shader stage
     */
    void SetSamplerState( const std::vector<IndexPtrPair> &sampler_states,
                          ShaderStage                      stages );

    /**
     * @brief Binds resources to shader stages of graphics pipeline
     *
     */
    void OnFlush( void * ) override;

    void Invalidate() override;

  private:
    ID3D11SamplerState *GetSamplerStateObj( uint64_t hash );

  private:
    std::unordered_map<uint64_t, ID3D11SamplerState *> m_aInternalStateData;

    std::array<uint64_t, D3D11_SAMPLER_SLOT_COUNT> m_aPSSamplerStates{};
    std::array<uint64_t, D3D11_SAMPLER_SLOT_COUNT> m_aCSSamplerStates{};
    std::array<uint64_t, D3D11_SAMPLER_SLOT_COUNT> m_aGSSamplerStates{};
    std::array<uint64_t, D3D11_SAMPLER_SLOT_COUNT> m_aVSSamplerStates{};
    std::array<uint64_t, D3D11_SAMPLER_SLOT_COUNT> m_aDSSamplerStates{};
    std::array<uint64_t, D3D11_SAMPLER_SLOT_COUNT> m_aHSSamplerStates{};

    uint32_t m_nDirtyStage   = 0;
    uint32_t m_nMaxDirtyIdPS = 0;
    uint32_t m_nMaxDirtyIdVS = 0;
    uint32_t m_nMaxDirtyIdGS = 0;
    uint32_t m_nMaxDirtyIdHS = 0;
    uint32_t m_nMaxDirtyIdDS = 0;
    uint32_t m_nMaxDirtyIdCS = 0;

    uint32_t m_nMinDirtyIdPS = D3D11_SAMPLER_SLOT_COUNT;
    uint32_t m_nMinDirtyIdVS = D3D11_SAMPLER_SLOT_COUNT;
    uint32_t m_nMinDirtyIdGS = D3D11_SAMPLER_SLOT_COUNT;
    uint32_t m_nMinDirtyIdHS = D3D11_SAMPLER_SLOT_COUNT;
    uint32_t m_nMinDirtyIdDS = D3D11_SAMPLER_SLOT_COUNT;
    uint32_t m_nMinDirtyIdCS = D3D11_SAMPLER_SLOT_COUNT;
};
} // namespace rh::engine
