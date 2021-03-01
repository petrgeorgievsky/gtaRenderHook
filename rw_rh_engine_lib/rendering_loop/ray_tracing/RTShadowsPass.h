//
// Created by peter on 03.08.2020.
//

#pragma once
#include <Engine/Common/ScopedPtr.h>

#include <array>
#include <cstdint>
namespace rh::engine
{
class IImageView;
class IShader;
class IPipelineLayout;
class VulkanRayTracingPipeline;
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

struct RTShadowsInitParams
{
    rh::engine::IDeviceState &        Device;
    uint32_t                          mWidth;
    uint32_t                          mHeight;
    RTSceneDescription *              mScene;
    CameraDescription *               mCamera;
    VarAwareTempAccumColorFilterPipe *mTAFilterPipeline;
    BilateralFilterPipeline *         mBilFilterPipe;
    rh::engine::IBuffer *             mTileBuffer;
    rh::engine::IBuffer *             mLightIdxBuffer;
    rh::engine::IBuffer *             mLightBuffer;
    rh::engine::IImageView *          mNormalsView;
    rh::engine::IImageView *          mPrevNormalsView;
    rh::engine::IImageView *          mMotionVectorsView;
    rh::engine::IBuffer *             mSkyCfg;
};

class RTShadowsPass
{

  public:
    rh::engine::IImageView *GetShadowsView(); // { return mShadowsBufferView; }

    void Execute( void *tlas, rh::engine::ICommandBuffer *cmd_buffer );

    RTShadowsPass( const RTShadowsInitParams &params );

  private:
    rh::engine::IDeviceState &         Device;
    RTSceneDescription *               mScene;
    CameraDescription *                mCamera;
    ScopedPointer<BilateralFilterPass> mBilFil0;
    ScopedPointer<VATAColorFilterPass> mVarianceTAFilter;

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

    ScopedPointer<rh::engine::IImageBuffer> mNoiseBuffer;
    ScopedPointer<rh::engine::IImageView>   mNoiseBufferView;

    ScopedPointer<rh::engine::ISampler> mTextureSampler;
    uint32_t                            mWidth;
    uint32_t                            mHeight;

    ScopedPointer<rh::engine::IBuffer> mParamsBuffer;
    ScopedPointer<rh::engine::IBuffer> mLightsBuffer;
    struct ShadowParams
    {
        float    sun_light_radius = 0.5;
        float    min_distance     = 0.01f;
        float    max_distance     = 5000.0f;
        uint32_t time_stamp       = 0;
    } mParams;
};
} // namespace rh::rw::engine