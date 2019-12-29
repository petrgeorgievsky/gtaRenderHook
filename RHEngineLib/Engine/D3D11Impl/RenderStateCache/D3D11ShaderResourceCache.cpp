#include "D3D11ShaderResourceCache.h"
#include "Engine/Common/types/index_ptr_pair.h"
#include "Engine/Common/types/shader_stage.h"
#include "Engine/D3D11Impl/ImageBuffers/ImageBuffer.h"
#include <common.h>

rh::engine::D3D11ShaderResourceCache::D3D11ShaderResourceCache() noexcept
{
    for ( size_t i = 0; i < D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT; i++ ) {
        m_aPSResourceViews[i] = nullptr;
        m_aDSResourceViews[i] = nullptr;
        m_aVSResourceViews[i] = nullptr;
        m_aHSResourceViews[i] = nullptr;
        m_aGSResourceViews[i] = nullptr;
        m_aCSResourceViews[i] = nullptr;
    }
    for ( auto &uav : m_aCSUAResourceViews ) {
        uav = nullptr;
    }
}

void rh::engine::D3D11ShaderResourceCache::SetShaderResources(
    const std::vector<IndexPtrPair> &resource_views, ShaderStage stage )
{
    for ( auto el : resource_views ) {
        if ( stage & ShaderStage::Compute ) {
            if ( m_aCSResourceViews[el.id] != el.ptr ) {
                m_aCSResourceViews[el.id] = reinterpret_cast<D3D11BindableResource *>( el.ptr );
                m_nDirtyStage |= ShaderStage::Compute;
                m_nMaxDirtyIdCS = max( m_nMaxDirtyIdCS, el.id );
                m_nMinDirtyIdCS = min( m_nMinDirtyIdCS, el.id );
                IStateCacheObject::MakeDirty();
            }
        }
        if ( stage & ShaderStage::Domain ) {
            if ( m_aDSResourceViews[el.id] != el.ptr ) {
                m_aDSResourceViews[el.id] = reinterpret_cast<D3D11BindableResource *>( el.ptr );
                m_nDirtyStage |= ShaderStage::Domain;
                m_nMaxDirtyIdDS = max( m_nMaxDirtyIdDS, el.id );
                m_nMinDirtyIdDS = min( m_nMinDirtyIdDS, el.id );
                IStateCacheObject::MakeDirty();
            }
        }
        if ( stage & ShaderStage::Hull ) {
            if ( m_aHSResourceViews[el.id] != el.ptr ) {
                m_aHSResourceViews[el.id] = reinterpret_cast<D3D11BindableResource *>( el.ptr );
                m_nDirtyStage |= ShaderStage::Hull;
                m_nMaxDirtyIdHS = max( m_nMaxDirtyIdHS, el.id );
                m_nMinDirtyIdHS = min( m_nMinDirtyIdHS, el.id );
                IStateCacheObject::MakeDirty();
            }
        }
        if ( stage & ShaderStage::Geometry ) {
            if ( m_aGSResourceViews[el.id] != el.ptr ) {
                m_aGSResourceViews[el.id] = reinterpret_cast<D3D11BindableResource *>( el.ptr );
                m_nDirtyStage |= ShaderStage::Geometry;
                m_nMaxDirtyIdGS = max( m_nMaxDirtyIdGS, el.id );
                m_nMinDirtyIdGS = min( m_nMinDirtyIdGS, el.id );
                IStateCacheObject::MakeDirty();
            }
        }
        if ( stage & ShaderStage::Pixel ) {
            if ( m_aPSResourceViews[el.id] != el.ptr ) {
                m_aPSResourceViews[el.id] = reinterpret_cast<D3D11BindableResource *>( el.ptr );
                m_nDirtyStage |= ShaderStage::Pixel;
                m_nMaxDirtyIdPS = max( m_nMaxDirtyIdPS, el.id );
                m_nMinDirtyIdPS = min( m_nMinDirtyIdPS, el.id );
                IStateCacheObject::MakeDirty();
            }
        }
        if ( stage & ShaderStage::Vertex ) {
            if ( m_aVSResourceViews[el.id] != el.ptr ) {
                m_aVSResourceViews[el.id] = reinterpret_cast<D3D11BindableResource *>( el.ptr );
                m_nDirtyStage |= ShaderStage::Vertex;
                m_nMaxDirtyIdVS = max( m_nMaxDirtyIdVS, el.id );
                m_nMinDirtyIdVS = min( m_nMinDirtyIdVS, el.id );
                IStateCacheObject::MakeDirty();
            }
        }
    }
}

void rh::engine::D3D11ShaderResourceCache::SetUnorderedAccessResources(
    const std::vector<rh::engine::IndexPtrPair> &resource_views, rh::engine::ShaderStage stage )
{
    for ( auto [id, ptr] : resource_views ) {
        if ( stage & ShaderStage::Compute ) {
            if ( m_aCSUAResourceViews[id] != ptr ) {
                m_aCSUAResourceViews[id] = reinterpret_cast<D3D11BindableResource *>( ptr );
                m_nDirtyStage |= ShaderStage::Compute;
                m_nMaxDirtyIdUACS = max( m_nMaxDirtyIdUACS, id );
                m_nMinDirtyIdUACS = min( m_nMinDirtyIdUACS, id );
                IStateCacheObject::MakeDirty();
            }
        }
    }
}

void rh::engine::D3D11ShaderResourceCache::OnFlush( void *deviceObject )
{
    std::vector<ID3D11ShaderResourceView *> srvArray;
    srvArray.reserve( static_cast<uint32_t>( D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT * 0.25 ) );
    std::vector<ID3D11UnorderedAccessView *> uavArray;
    uavArray.reserve( static_cast<uint32_t>( D3D11_1_UAV_SLOT_COUNT * 0.25 ) );
    auto *context = reinterpret_cast<ID3D11DeviceContext *>( deviceObject );

    if ( context ) {
        if ( m_nDirtyStage & ShaderStage::Compute ) {
            for ( unsigned int i = m_nMinDirtyIdCS; i <= m_nMaxDirtyIdCS; i++ )
                srvArray.push_back( m_aCSResourceViews[i]
                                        ? m_aCSResourceViews[i]->GetShaderResourceView()
                                        : nullptr );

            context->CSSetShaderResources( m_nMinDirtyIdCS,
                                           static_cast<UINT>( srvArray.size() ),
                                           srvArray.data() );

            m_nMaxDirtyIdCS = 0;
            m_nMinDirtyIdCS = D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT - 1;
            srvArray.clear();

            for ( unsigned int i = m_nMinDirtyIdUACS; i <= m_nMaxDirtyIdUACS; i++ )
                uavArray.push_back( m_aCSUAResourceViews[i] != nullptr
                                        ? m_aCSUAResourceViews[i]->GetUnorderedAccessView()
                                        : nullptr );

            context->CSSetUnorderedAccessViews( m_nMinDirtyIdUACS,
                                                static_cast<UINT>( uavArray.size() ),
                                                uavArray.data(),
                                                nullptr );

            m_nMaxDirtyIdUACS = 0;
            m_nMinDirtyIdUACS = D3D11_1_UAV_SLOT_COUNT - 1;
            uavArray.clear();
        }

        if ( m_nDirtyStage & ShaderStage::Domain ) {
            for ( unsigned int i = m_nMinDirtyIdDS; i <= m_nMaxDirtyIdDS; i++ )
                srvArray.push_back( m_aDSResourceViews[i]
                                        ? m_aDSResourceViews[i]->GetShaderResourceView()
                                        : nullptr );

            context->DSSetShaderResources( m_nMinDirtyIdDS,
                                           static_cast<UINT>( srvArray.size() ),
                                           srvArray.data() );

            m_nMaxDirtyIdDS = 0;
            m_nMinDirtyIdDS = D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT - 1;
            srvArray.clear();
        }

        if ( m_nDirtyStage & ShaderStage::Hull ) {
            for ( unsigned int i = m_nMinDirtyIdHS; i <= m_nMaxDirtyIdHS; i++ )
                srvArray.push_back( m_aHSResourceViews[i]
                                        ? m_aHSResourceViews[i]->GetShaderResourceView()
                                        : nullptr );

            context->HSSetShaderResources( m_nMinDirtyIdHS,
                                           static_cast<UINT>( srvArray.size() ),
                                           srvArray.data() );

            m_nMaxDirtyIdHS = 0;
            m_nMinDirtyIdHS = D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT - 1;
            srvArray.clear();
        }

        if ( m_nDirtyStage & ShaderStage::Geometry ) {
            for ( unsigned int i = m_nMinDirtyIdGS; i <= m_nMaxDirtyIdGS; i++ )
                srvArray.push_back( m_aGSResourceViews[i]
                                        ? m_aGSResourceViews[i]->GetShaderResourceView()
                                        : nullptr );

            context->GSSetShaderResources( m_nMinDirtyIdGS,
                                           static_cast<UINT>( srvArray.size() ),
                                           srvArray.data() );

            m_nMaxDirtyIdGS = 0;
            m_nMinDirtyIdGS = D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT - 1;
            srvArray.clear();
        }

        if ( m_nDirtyStage & ShaderStage::Pixel ) {
            for ( unsigned int i = m_nMinDirtyIdPS; i <= m_nMaxDirtyIdPS; i++ )
                srvArray.push_back( m_aPSResourceViews[i]
                                        ? m_aPSResourceViews[i]->GetShaderResourceView()
                                        : nullptr );

            context->PSSetShaderResources( m_nMinDirtyIdPS,
                                           static_cast<UINT>( srvArray.size() ),
                                           srvArray.data() );

            m_nMaxDirtyIdPS = 0;
            m_nMinDirtyIdPS = D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT - 1;
            srvArray.clear();
        }

        if ( m_nDirtyStage & ShaderStage::Vertex ) {
            for ( unsigned int i = m_nMinDirtyIdVS; i <= m_nMaxDirtyIdVS; i++ )
                srvArray.push_back( m_aVSResourceViews[i]
                                        ? m_aVSResourceViews[i]->GetShaderResourceView()
                                        : nullptr );

            context->VSSetShaderResources( m_nMinDirtyIdVS,
                                           static_cast<UINT>( srvArray.size() ),
                                           srvArray.data() );

            m_nMaxDirtyIdVS = 0;
            m_nMinDirtyIdVS = D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT - 1;
            srvArray.clear();
        }
    }
    m_nDirtyStage = 0;
}

void rh::engine::D3D11ShaderResourceCache::Invalidate()
{
    IStateCacheObject::MakeDirty();
    for ( size_t i = 0; i < D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT; i++ ) {
        m_aPSResourceViews[i] = nullptr;
        m_aDSResourceViews[i] = nullptr;
        m_aVSResourceViews[i] = nullptr;
        m_aHSResourceViews[i] = nullptr;
        m_aGSResourceViews[i] = nullptr;
        m_aCSResourceViews[i] = nullptr;
    }
    for ( auto &uav : m_aCSUAResourceViews ) {
        uav = nullptr;
    }
}
