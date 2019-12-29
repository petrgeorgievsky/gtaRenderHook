#pragma once
#include "bvh_builder.h"
#include <unordered_map>

class RayTracingScene
{
public:
    RayTracingScene( BVHBuilder *bvh_builder );

    void PushModel( uint32_t id, RpGeometry *geometry );

    std::pair<const PackedBLAS_BVH &, bool> PackBLAS();
    TLAS_BVH GenerateTLAS( std::vector<BLAS_Instance> instances );

private:
    std::unordered_map<uint32_t, BLAS_BVH> m_bottomLevelAS;
    std::vector<BLAS_BVH> m_BLASList;
    std::unordered_map<uint32_t, uint32_t> m_BLASRemappedIds;
    bool m_bBLASReqUpdate = false;
    PackedBLAS_BVH m_packedBLAS;
    BVHBuilder *m_pBVHBuilder;
};
