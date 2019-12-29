#pragma once
#include "bvh_builder.h"

struct RpGeometry;
namespace rh::engine {
class IGPUResource;
}
namespace rw_raytracing_lib {

struct TraceRaysDispatchParams
{
    uint32_t dispatchThreadsX, dispatchThreadsY, dispatchThreadsZ;
    rh::engine::IGPUResource *shader;
    rh::engine::IGPUResource *target;
};

class RayTracer
{
public:
    void AllocateTLAS( const TLAS_BVH &bvh );
    void AllocatePackedBLAS( const PackedBLAS_BVH &bvh );
    void TraceRays( const TraceRaysDispatchParams &params );

private:
    rh::engine::IGPUResource *mTriangleBuffer;
    rh::engine::IGPUResource *mVertexBuffer;
    rh::engine::IGPUResource *mBVHLeafBuffer;
    rh::engine::IGPUResource *mTLAS_BVHBuffer;
};

} // namespace rw_raytracing_lib
