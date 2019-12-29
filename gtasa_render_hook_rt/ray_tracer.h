#pragma once
#include <bvh_builder.h>
struct RpGeometry;

struct TimeCycle
{
    DirectX::XMFLOAT4 cTopSkyColor;
    DirectX::XMFLOAT4 cBottomSkyColor;
    DirectX::XMFLOAT4 vSunDir;
};

class RayTracer
{
public:
    void Initialize();
    void AllocateBVHOnGPU( rw_raytracing_lib::TLAS_BVH &&bvh );
    void AllocatePackedBLASGPU( const rw_raytracing_lib::PackedBLAS_BVH &bvh );
    void TraceRays(
        void *gbPos, void *gbNormals, void *gbColor, void *gbVertexRadiance, void *textureCache );
    void UpdateTimecyc( const DirectX::XMFLOAT4 &topSkyColor,
                        const DirectX::XMFLOAT4 &bottomSkyColor,
                        const DirectX::XMFLOAT4 &sunDir );
    void *Accumulate( void *curr_tex );
    void *RaytracedTex();
    void *Composite( void *blurred_tex, void *gbColor );

private:
    void *mRaytraceCS;
    void *mCompositeCS;
    void *mAccumulateCS;

    void *mBlueNoiseTex;

    TimeCycle mTCBuf;
    void *mTimeCycleCB;
    void *mTriangleBuffer;
    void *mVertexBuffer;
    void *mBVHLeafBuffer;
    void *mMaterialBuffer;
    void *mTextureBuffer;
    void *mTLAS_BVHBuffer;
    void *mCompositeTarget;
    void *mRayTracingTarget[3];
    void *mGICacheTarget[2];
    uint32_t current_gi_cache = 0;
    uint32_t current_rt_tex = 0;
};
