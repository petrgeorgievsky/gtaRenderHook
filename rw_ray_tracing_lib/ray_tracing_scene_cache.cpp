#include "ray_tracing_scene_cache.h"
using namespace rw_raytracing_lib;

RayTracingScene::RayTracingScene( BVHBuilder *bvh_builder )
{
    mTextureCache = new RayTracingTextureCache();
    m_pBVHBuilder = bvh_builder;
    //m_packedBLAS.blas.nodes.reserve( ( 1024 * 1024 * 20 ) / sizeof( LinearBVHNode ) );
    //m_packedBLAS.blas.vertices.reserve( ( 1024 * 1024 * 20 ) / sizeof( RTVertex ) );
    //m_packedBLAS.blas.ordered_triangles.reserve( 1024 * 1024 * 20 / sizeof( RpTriangle ) );
}

void RayTracingScene::PushModel( uint32_t id, rh::rw::engine::RpGeometryInterface *geometry )
{
    if ( m_bottomLevelAS.find( id ) != m_bottomLevelAS.end() )
        return;

    m_BLASList.push_back( m_pBVHBuilder->BuildBVH( geometry, mTextureCache ) );
    m_bottomLevelAS[id] = &m_BLASList[mCurrentBLASId];
    m_BLASRemappedIds[id] = mCurrentBLASId++;

    m_bBLASReqUpdate = true;
}

std::pair<const PackedBLAS_BVH &, bool> RayTracingScene::PackBLAS()
{
    if ( !m_bBLASReqUpdate )
        return {m_packedBLAS, false};

    std::vector<BLAS_BVH *> blas_list;
    blas_list.reserve( mCurrentBLASId - mLastBLASIdBeforePacking );

    for ( uint32_t it = mLastBLASIdBeforePacking; it < mCurrentBLASId; it++ ) {
        blas_list.push_back( &m_BLASList[it] );
    }
    m_pBVHBuilder->PackBLASBVH( m_packedBLAS, blas_list, mLastBLASIdBeforePacking );
    m_bBLASReqUpdate = false;
    mLastBLASIdBeforePacking = mCurrentBLASId;
    return {m_packedBLAS, true};
}

TLAS_BVH
RayTracingScene::GenerateTLAS( std::vector<BLAS_Instance> instances )
{
    for ( auto &inst : instances )
        inst.blas_id = m_BLASRemappedIds[inst.blas_id];

    return m_pBVHBuilder->BuildTLASBVH( m_packedBLAS, m_BLASList, instances );
}

void RayTracingScene::TidyUp()
{
    if ( m_BLASList.size() > 4000 ) {
        mCurrentBLASId = 0;
        mLastBLASIdBeforePacking = 0;
        m_bottomLevelAS.clear();
        m_BLASRemappedIds.clear();
        m_BLASList.clear();
        m_packedBLAS.blas.nodes.clear();
        m_packedBLAS.blas.vertices.clear();
        m_packedBLAS.blas.ordered_triangles.clear();
        m_packedBLAS.blas_offsets_map.clear();
    }
}

RayTracingTextureCache *RayTracingScene::GetTextureCache()
{
    return mTextureCache;
}
