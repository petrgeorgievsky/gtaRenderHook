#include "ray_tracing_scene_cache.h"

RayTracingScene::RayTracingScene( BVHBuilder *bvh_builder )
{
    m_pBVHBuilder = bvh_builder;
}

void RayTracingScene::PushModel( uint32_t id, RpGeometry *geometry )
{
    if ( m_bottomLevelAS.find( id ) != m_bottomLevelAS.end() )
        return;
    m_bottomLevelAS[id] = m_pBVHBuilder->BuildBVH( geometry );
    m_bBLASReqUpdate = true;
}

std::pair<const PackedBLAS_BVH &, bool> RayTracingScene::PackBLAS()
{
    if ( !m_bBLASReqUpdate )
        return {m_packedBLAS, false};

    m_BLASList.clear();
    m_BLASRemappedIds.clear();
    m_BLASList.reserve( m_bottomLevelAS.size() );
    m_BLASRemappedIds.reserve( m_bottomLevelAS.size() );

    uint32_t it = 0;
    for ( auto [id, blas] : m_bottomLevelAS ) {
        m_BLASList.push_back( blas );
        m_BLASRemappedIds[id] = it;
        it++;
    }
    m_packedBLAS = m_pBVHBuilder->PackBLASBVH( m_BLASList );
    m_bBLASReqUpdate = false;
    return {m_packedBLAS, true};
}

TLAS_BVH
RayTracingScene::GenerateTLAS( std::vector<BLAS_Instance> instances )
{
    for ( auto &inst : instances )
        inst.blas_id = m_BLASRemappedIds[inst.blas_id];

    return m_pBVHBuilder->BuildTLASBVH( m_packedBLAS, m_BLASList, instances );
}
