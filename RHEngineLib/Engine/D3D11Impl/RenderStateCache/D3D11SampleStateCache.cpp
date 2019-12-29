#include "D3D11SampleStateCache.h"
#include "../D3D11Renderer.h"
#include "Engine/Common/types/shader_stage.h"
#include <common.h>
namespace rh::engine {
uint64_t GenerateSamplerHash( void *sampler_ptr )
{
    auto *sampler_ptr_ = reinterpret_cast<Sampler *>( sampler_ptr );
    if ( sampler_ptr_ == nullptr )
        return 0;
    Sampler sampler = *sampler_ptr_;
    auto hash = static_cast<uint64_t>( sampler.filtering );
    uint64_t offset = 2;
    hash += static_cast<uint64_t>( sampler.adressU ) << offset;
    offset += 3;
    hash += static_cast<uint64_t>( sampler.adressV ) << offset;
    offset += 3;
    hash += static_cast<uint64_t>( sampler.adressW ) << offset;
    offset += 3;
    hash += static_cast<uint64_t>( sampler.comparison ) << offset;
    offset += 4;
    /*hash += static_cast<uint64_t>( RWRGBALONG( sampler.borderColor.red,
                                              sampler.borderColor.green,
                                              sampler.borderColor.blue,
                                              sampler.borderColor.alpha ) )
           << offset;*/
    return hash;
}

Sampler GetSamplerFromHash( uint64_t hash )
{
    Sampler sampler{};
    sampler.filtering = static_cast<SamplerFilter>( hash & 3 );
    sampler.adressU = static_cast<SamplerAddressing>( ( hash >> 2 ) & 4 );
    sampler.adressV = static_cast<SamplerAddressing>( ( hash >> 5 ) & 4 );
    sampler.adressW = static_cast<SamplerAddressing>( ( hash >> 8 ) & 4 );
    sampler.comparison = static_cast<ComparisonFunc>( ( hash >> 11 ) & 8 );
    uint64_t color = ( hash >> 15 );
    sampler.borderColor = {static_cast<unsigned char>( ( color >> 16 ) & 0xFF ),
                           static_cast<unsigned char>( ( color >> 8 ) & 0xFF ),
                           static_cast<unsigned char>( (color) &0xFF ),
                           static_cast<unsigned char>( ( color >> 24 ) & 0xFF )};
    return sampler;
}

D3D11SamplerStateCache::D3D11SamplerStateCache() noexcept
{
    for ( size_t i = 0; i < D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT; i++ ) {
        m_aPSSamplerStates[i] = 0;
        m_aDSSamplerStates[i] = 0;
        m_aVSSamplerStates[i] = 0;
        m_aHSSamplerStates[i] = 0;
        m_aGSSamplerStates[i] = 0;
        m_aCSSamplerStates[i] = 0;
    }
    m_aInternalStateData[0] = nullptr;
}

D3D11SamplerStateCache::~D3D11SamplerStateCache() noexcept
{
    for ( auto &&[id, sampler] : m_aInternalStateData ) {
        if ( sampler != nullptr ) {
            sampler->Release();
            sampler = nullptr;
        }
    }
}

void D3D11SamplerStateCache::SetSamplerState( const std::vector<IndexPtrPair> &sampler_states,
                                              ShaderStage stage )
{
    for ( auto &&el : sampler_states ) {
        uint64_t value_hash = GenerateSamplerHash( el.ptr );
        if ( stage & ShaderStage::Compute ) {
            if ( m_aCSSamplerStates[el.id] != value_hash ) {
                m_aCSSamplerStates[el.id] = value_hash;
                m_nDirtyStage |= ShaderStage::Compute;
                m_nMaxDirtyIdCS = max( m_nMaxDirtyIdCS, el.id );
                m_nMinDirtyIdCS = min( m_nMinDirtyIdCS, el.id );
                MakeDirty();
            }
        }
        if ( stage & ShaderStage::Domain ) {
            if ( m_aDSSamplerStates[el.id] != value_hash ) {
                m_aDSSamplerStates[el.id] = value_hash;
                m_nDirtyStage |= ShaderStage::Domain;
                m_nMaxDirtyIdDS = max( m_nMaxDirtyIdDS, el.id );
                m_nMinDirtyIdDS = min( m_nMinDirtyIdDS, el.id );
                MakeDirty();
            }
        }
        if ( stage & ShaderStage::Hull ) {
            if ( m_aHSSamplerStates[el.id] != value_hash ) {
                m_aHSSamplerStates[el.id] = value_hash;
                m_nDirtyStage |= ShaderStage::Hull;
                m_nMaxDirtyIdHS = max( m_nMaxDirtyIdHS, el.id );
                m_nMinDirtyIdHS = min( m_nMinDirtyIdHS, el.id );
                MakeDirty();
            }
        }
        if ( stage & ShaderStage::Geometry ) {
            if ( m_aGSSamplerStates[el.id] != value_hash ) {
                m_aGSSamplerStates[el.id] = value_hash;
                m_nDirtyStage |= ShaderStage::Geometry;
                m_nMaxDirtyIdGS = max( m_nMaxDirtyIdGS, el.id );
                m_nMinDirtyIdGS = min( m_nMinDirtyIdGS, el.id );
                MakeDirty();
            }
        }
        if ( stage & ShaderStage::Pixel ) {
            if ( m_aPSSamplerStates[el.id] != value_hash ) {
                m_aPSSamplerStates[el.id] = value_hash;
                m_nDirtyStage |= ShaderStage::Pixel;
                m_nMaxDirtyIdPS = max( m_nMaxDirtyIdPS, el.id );
                m_nMinDirtyIdPS = min( m_nMinDirtyIdPS, el.id );
                MakeDirty();
            }
        }
        if ( stage & ShaderStage::Vertex ) {
            if ( m_aVSSamplerStates[el.id] != value_hash ) {
                m_aVSSamplerStates[el.id] = value_hash;
                m_nDirtyStage |= ShaderStage::Vertex;
                m_nMaxDirtyIdVS = max( m_nMaxDirtyIdVS, el.id );
                m_nMinDirtyIdVS = min( m_nMinDirtyIdVS, el.id );
                MakeDirty();
            }
        }
    }
}

void D3D11SamplerStateCache::OnFlush( void *deviceObject )
{
    std::vector<ID3D11SamplerState *> sampler_states;
    auto *context = reinterpret_cast<ID3D11DeviceContext *>( deviceObject );

    if ( context ) {
        if ( m_nDirtyStage & ShaderStage::Compute ) {
            for ( unsigned int i = m_nMinDirtyIdCS; i <= m_nMaxDirtyIdCS; i++ )
                sampler_states.push_back( GetSamplerStateObj( m_aCSSamplerStates[i] ) );

            context->CSSetSamplers( m_nMinDirtyIdCS,
                                    static_cast<UINT>( sampler_states.size() ),
                                    sampler_states.data() );

            m_nMaxDirtyIdCS = 0;
            m_nMinDirtyIdCS = D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT;
            sampler_states.clear();
        }

        if ( m_nDirtyStage & ShaderStage::Domain ) {
            for ( unsigned int i = m_nMinDirtyIdDS; i <= m_nMaxDirtyIdDS; i++ )
                sampler_states.push_back( GetSamplerStateObj( m_aDSSamplerStates[i] ) );

            context->DSSetSamplers( m_nMinDirtyIdDS,
                                    static_cast<UINT>( sampler_states.size() ),
                                    sampler_states.data() );

            m_nMaxDirtyIdDS = 0;
            m_nMinDirtyIdDS = D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT;
            sampler_states.clear();
        }

        if ( m_nDirtyStage & ShaderStage::Hull ) {
            for ( unsigned int i = m_nMinDirtyIdHS; i <= m_nMaxDirtyIdHS; i++ )
                sampler_states.push_back( GetSamplerStateObj( m_aHSSamplerStates[i] ) );

            context->HSSetSamplers( m_nMinDirtyIdHS,
                                    static_cast<UINT>( sampler_states.size() ),
                                    sampler_states.data() );

            m_nMaxDirtyIdHS = 0;
            m_nMinDirtyIdHS = D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT;
            sampler_states.clear();
        }

        if ( m_nDirtyStage & ShaderStage::Geometry ) {
            for ( unsigned int i = m_nMinDirtyIdGS; i <= m_nMaxDirtyIdGS; i++ )
                sampler_states.push_back( GetSamplerStateObj( m_aGSSamplerStates[i] ) );

            context->GSSetSamplers( m_nMinDirtyIdGS,
                                    static_cast<UINT>( sampler_states.size() ),
                                    sampler_states.data() );

            m_nMaxDirtyIdGS = 0;
            m_nMinDirtyIdGS = D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT;
            sampler_states.clear();
        }

        if ( m_nDirtyStage & ShaderStage::Pixel ) {
            for ( unsigned int i = m_nMinDirtyIdPS; i <= m_nMaxDirtyIdPS; i++ )
                sampler_states.push_back( GetSamplerStateObj( m_aPSSamplerStates[i] ) );

            context->PSSetSamplers( m_nMinDirtyIdPS,
                                    static_cast<UINT>( sampler_states.size() ),
                                    sampler_states.data() );

            m_nMaxDirtyIdPS = 0;
            m_nMinDirtyIdPS = D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT;
            sampler_states.clear();
        }

        if ( m_nDirtyStage & ShaderStage::Vertex ) {
            for ( unsigned int i = m_nMinDirtyIdVS; i <= m_nMaxDirtyIdVS; i++ )
                sampler_states.push_back( GetSamplerStateObj( m_aVSSamplerStates[i] ) );

            context->VSSetSamplers( m_nMinDirtyIdVS,
                                    static_cast<UINT>( sampler_states.size() ),
                                    sampler_states.data() );

            m_nMaxDirtyIdVS = 0;
            m_nMinDirtyIdVS = D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT;
            sampler_states.clear();
        }
    }
    m_nDirtyStage = 0;
}

void D3D11SamplerStateCache::Invalidate()
{
    IStateCacheObject::Invalidate();
    for ( size_t i = 0; i < D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT; i++ ) {
        m_aPSSamplerStates[i] = 0;
        m_aDSSamplerStates[i] = 0;
        m_aVSSamplerStates[i] = 0;
        m_aHSSamplerStates[i] = 0;
        m_aGSSamplerStates[i] = 0;
        m_aCSSamplerStates[i] = 0;
    }
}

ID3D11SamplerState *D3D11SamplerStateCache::GetSamplerStateObj( uint64_t hash )
{
    if ( hash == 0 )
        return nullptr;
    if ( m_aInternalStateData[hash] != nullptr )
        return m_aInternalStateData[hash];

    auto allocator = rh::engine::g_pRHRenderer->GetGPUAllocator();

    allocator->AllocateSampler( GetSamplerFromHash( hash ),
                                reinterpret_cast<void *&>( m_aInternalStateData[hash] ) );

    return m_aInternalStateData[hash];
}
} // namespace rh::engine
