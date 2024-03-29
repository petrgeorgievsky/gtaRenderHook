//
// Created by peter on 31.07.2021.
//

#pragma once
#include <Engine/Common/ScopedPtr.h>

#include <array>
#include <cstdint>
#include <data_desc/light_system/packed_light.h>

namespace rh::engine
{
class IImageView;
class IShader;
class IPipelineLayout;
class VulkanRayTracingPipeline;
class VulkanComputePipeline;
class IBuffer;
class IImageBuffer;
class ISampler;
class ICommandBuffer;
class IDescriptorSetAllocator;
class IDescriptorSet;
class IDescriptorSetLayout;
class IDeviceState;
} // namespace rh::engine

namespace rh::rw::engine
{
using rh::engine::ScopedPointer;
class RTSceneDescription;
class CameraDescription;
class VarAwareTempAccumColorFilterPipe;
class VATAColorFilterPass;
class BilateralFilterPipeline;
class BilateralFilterPass;

namespace restir
{
class LightSamplingPass;
class SpatialReusePass;
class VisibilityReusePass;

struct ShadowsInitParams
{
    rh::engine::IDeviceState         &Device;
    uint32_t                          mWidth;
    uint32_t                          mHeight;
    RTSceneDescription               *mScene;
    CameraDescription                *mCamera;
    rh::engine::IBuffer              *mSkyCfg;
    rh::engine::IImageView           *mNormalsView;
    rh::engine::IBuffer              *mLightBuffer;
    rh::engine::IImageView           *mPrevNormalsView;
    rh::engine::IImageView           *mMotionVectorsView;
    VarAwareTempAccumColorFilterPipe *mTAFilterPipeline;
    BilateralFilterPipeline          *mBilFilterPipe;
};

struct ShadowsProperties
{
    float    LightRadius         = 0.3f;
    float    LightPenumbra       = 0.03f;
    uint32_t Timestamp           = 0;
    float    LightIntensityBoost = 1.0f;
    uint32_t LightsCount         = 0;
    uint32_t padd[3];
};

class ShadowsPass
{

  public:
    rh::engine::IImageView *GetShadowsView(); // { return mShadowsBufferView; }

    void Execute( void *tlas, uint32_t light_count,
                  rh::engine::ICommandBuffer *cmd_buffer );

    ShadowsPass( const ShadowsInitParams &params );
    ~ShadowsPass();

    void UpdateUI();

    void RecordTriLights( const std::vector<PackedLight> &lights,
                          const DirectX::XMFLOAT4X3 &transform, int inst_id );
    void Reset();

  private:
    rh::engine::IDeviceState          &Device;
    RTSceneDescription                *mScene;
    CameraDescription                 *mCamera;
    ScopedPointer<LightSamplingPass>   mLightPopulationPass;
    ScopedPointer<VisibilityReusePass> mVisibilityReusePass;
    ScopedPointer<SpatialReusePass>    mSpatialReusePass;
    ScopedPointer<SpatialReusePass>    mSpatialReusePass2;
    ScopedPointer<rh::engine::IBuffer> TempReservoirBuffer;
    ScopedPointer<rh::engine::IBuffer> ResultReservoirBuffer;
    ScopedPointer<rh::engine::IBuffer> TriangleLights;

    ScopedPointer<BilateralFilterPass> mBilFil0;
    ScopedPointer<VATAColorFilterPass> mVarianceTAFilter;

    ShadowsProperties mPassParams{};

    ScopedPointer<rh::engine::IDescriptorSetAllocator> mDescSetAlloc;
    ScopedPointer<rh::engine::IDescriptorSetLayout>    mRayTraceSetLayout;
    ScopedPointer<rh::engine::IDescriptorSet>          mRayTraceSet;

    ScopedPointer<rh::engine::IPipelineLayout>          mPipeLayout;
    ScopedPointer<rh::engine::VulkanRayTracingPipeline> mPipeline;

    ScopedPointer<rh::engine::IShader> mRayGenShader;
    ScopedPointer<rh::engine::IShader> mClosestHitShader{};
    ScopedPointer<rh::engine::IShader> mAnyHitShader;
    ScopedPointer<rh::engine::IShader> mMissShader;

    ScopedPointer<rh::engine::IBuffer> mShaderBindTable;

    ScopedPointer<rh::engine::IImageBuffer> mShadowsBuffer;
    ScopedPointer<rh::engine::IImageView>   mShadowsBufferView;

    ScopedPointer<rh::engine::IImageBuffer> mTempBlurShadowsBuffer;
    ScopedPointer<rh::engine::IImageView>   mTempBlurShadowsBufferView;
    ScopedPointer<rh::engine::IImageBuffer> mBlurredShadowsBuffer;
    ScopedPointer<rh::engine::IImageView>   mBlurredShadowsBufferView;

    ScopedPointer<rh::engine::ISampler> mTextureSampler;
    uint32_t                            mWidth;
    uint32_t                            mHeight;
    bool                                EnableDenoiser = true;

    ScopedPointer<rh::engine::IBuffer> mParamsBuffer;
    uint32_t                           TriLightsCpuBufferCount{ 0 };
    std::array<PackedLight, 10240>     TriLightsCpuBuffer{};
};
} // namespace restir
} // namespace rh::rw::engine