#pragma once
#include "bvh_builder.h"
#include "ray_tracing_texture_cache.h"
#include <unordered_map>
namespace rh::rw::engine {
class RpGeometryInterface;
}
namespace rw_raytracing_lib {

class RayTracingScene
{
public:
    RayTracingScene( BVHBuilder *bvh_builder );

    void PushModel( uint32_t id, rh::rw::engine::RpGeometryInterface *geometry );

    std::pair<const PackedBLAS_BVH &, bool> PackBLAS();
    TLAS_BVH GenerateTLAS( std::vector<BLAS_Instance> instances );
    void TidyUp();
    RayTracingTextureCache *GetTextureCache();

private:
    std::unordered_map<uint32_t, BLAS_BVH *> m_bottomLevelAS;
    std::vector<BLAS_BVH> m_BLASList;
    std::unordered_map<uint32_t, uint32_t> m_BLASRemappedIds;
    uint32_t mCurrentBLASId = 0;
    uint32_t mLastBLASIdBeforePacking = 0;
    bool m_bBLASReqUpdate = false;
    PackedBLAS_BVH m_packedBLAS;
    BVHBuilder *m_pBVHBuilder;
    RayTracingTextureCache *mTextureCache;
};
} // namespace rw_raytracing_lib
