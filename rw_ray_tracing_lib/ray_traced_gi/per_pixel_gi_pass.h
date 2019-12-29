#pragma once
#include "../ray_tracer.h"
#include <stdint.h>
namespace rh::engine {
class IGPUResource;
}
namespace rw_raytracing_lib {
struct PerPixelGIParams
{
    float fRenderingScale;
    float fRaytracingDistance;
    float fNoiseScale;
    float gi_padd_;
};

class RTPerPixelGIPass
{
public:
    RTPerPixelGIPass( size_t w, size_t h, float scale );
    ~RTPerPixelGIPass();

    void *Execute( RayTracer *ray_tracer, void *gbPos, void *gbNormals );
    void *GetSHCoCg();

private:
    TraceRaysDispatchParams mDispatchParams;
    rh::engine::IGPUResource *mBlueNoise = nullptr;
    rh::engine::IGPUResource *mGIParamsBuffer = nullptr;
    rh::engine::IGPUResource *mSHCoCgBuff = nullptr;
    PerPixelGIParams mGIParamsData;
    uint32_t mThreadGroupSize = 8;
};
} // namespace rw_raytracing_lib
