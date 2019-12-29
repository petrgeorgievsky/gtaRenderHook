#pragma once
#include <stdint.h>
namespace rh::engine {
class IGPUResource;
}
namespace rw_raytracing_lib {
struct LFFilterParams
{
    float fRenderingScale;
    int nPreFilterIteration;
    float fNormalsScale;
    float fDepthScale;
};

class LowFreqGIFilterPass
{
public:
    LowFreqGIFilterPass( size_t w, size_t h, float lf_scale );
    ~LowFreqGIFilterPass();

    void *Execute(
        void *gbPos, void *gbNormals, void *shY, void *shCoCg, void *gbColor, void *gbRadiance );

private:
    void PreFilter( void *shY_in,
                    void *shCoCg_in,
                    void *shY_out,
                    void *shCoCg_out,
                    void *gbPos,
                    void *gbNormals );

private:
    uint32_t dispatchThreadsX, dispatchThreadsY, dispatchThreadsZ;
    rh::engine::IGPUResource *mPreFilterParamsCB = nullptr;
    LFFilterParams mPreFilterParams{};
    rh::engine::IGPUResource *mFilterResult = nullptr;
    rh::engine::IGPUResource *mPreFilteredSHY_Ping = nullptr;
    rh::engine::IGPUResource *mPreFilteredSHCoCg_Ping = nullptr;
    rh::engine::IGPUResource *mPreFilteredSHY_Pong = nullptr;
    rh::engine::IGPUResource *mPreFilteredSHCoCg_Pong = nullptr;
    rh::engine::IGPUResource *mLowFreqGIFilterCS = nullptr;
    rh::engine::IGPUResource *mLowFreqGIPreFilterCS = nullptr;
    uint32_t mThreadGroupSize = 8;
};

} // namespace rw_raytracing_lib
