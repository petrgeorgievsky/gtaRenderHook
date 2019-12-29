#pragma once
#include "../ray_tracer.h"
#include <stdint.h>
namespace rw_raytracing_lib {
class RTShadowsPass
{
public:
    RTShadowsPass( uint32_t w, uint32_t h );
    ~RTShadowsPass();

    void *Execute( RayTracer *ray_tracer, void *gbPos, void *gbNormals );

private:
    TraceRaysDispatchParams mDispatchParams;
    uint32_t mThreadGroupSize = 8;
};
} // namespace rw_raytracing_lib
