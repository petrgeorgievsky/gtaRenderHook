#pragma once
#include "../ray_tracer.h"
#include <stdint.h>
namespace rw_raytracing_lib {
class RTIrradianceGIPass
{
public:
    RTIrradianceGIPass( uint32_t sx, uint32_t sy, uint32_t sz );
    ~RTIrradianceGIPass();

    void ComputeIrradianceField( RayTracer *ray_tracer );

    void Execute( void *gbPos, void *gbNormals, void *result, size_t w, size_t h );

private:
    TraceRaysDispatchParams mDispatchParams;
    rh::engine::IGPUResource *mIrradianceFieldCS;
    uint32_t mThreadGroupSizeIrradianceFields = 4;
    uint32_t mThreadGroupSizeComposite = 4;
};
} // namespace rw_raytracing_lib
