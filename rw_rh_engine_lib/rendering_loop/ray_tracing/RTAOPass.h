//
// Created by peter on 27.06.2020.
//
#pragma once

#include <Engine/Common/ScopedPtr.h>

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
class VarAwareTempAccumFilterPipe;
class VATAFilterPass;
class BilateralFilterPipeline;
class BilateralFilterPass;
struct RTAOInitParams
{
    rh::engine::IDeviceState &Device;

    RTSceneDescription *         mScene;
    CameraDescription *          mCamera;
    VarAwareTempAccumFilterPipe *mTAFilterPipeline;
    BilateralFilterPipeline *    mBilateralFilterPipe;
    rh::engine::IImageView *     mNormalsView;
    rh::engine::IImageView *     mPrevNormalsView;
    rh::engine::IImageView *     mMotionVectorsView;
    uint32_t                     mWidth;
    uint32_t                     mHeight;
};

class RTAOPass
{
  public:
    RTAOPass( const RTAOInitParams &params );
    rh::engine::IImageView *GetAOView();

    void Execute( void *tlas, rh::engine::ICommandBuffer *cmd_buffer );
    void UpdateUI();

  private:
    rh::engine::IDeviceState &         Device;
    RTSceneDescription *               mScene;
    CameraDescription *                mCamera;
    ScopedPointer<BilateralFilterPass> mBilFil0;
    ScopedPointer<VATAFilterPass>      mVarianceTAFilter;

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

    ScopedPointer<rh::engine::IImageBuffer> mAOBuffer[2];
    ScopedPointer<rh::engine::IImageView>   mAOBufferView[2];
    ScopedPointer<rh::engine::IImageBuffer> mTempBlurAOBuffer;
    ScopedPointer<rh::engine::IImageView>   mTempBlurAOBufferView;
    ScopedPointer<rh::engine::IImageBuffer> mBlurredAOBuffer;
    ScopedPointer<rh::engine::IImageView>   mBlurredAOBufferView;

    ScopedPointer<rh::engine::IImageBuffer> mNoiseBuffer;
    ScopedPointer<rh::engine::IImageView>   mNoiseBufferView;

    ScopedPointer<rh::engine::ISampler> mTextureSampler;
    uint32_t                            mWidth;
    uint32_t                            mHeight;
    ScopedPointer<rh::engine::IBuffer>  mParamsBuffer;

    struct AOParams
    {
        float    min_distance  = 0.001f;
        float    max_distance  = 1.5f;
        float    max_draw_dist = 1000.0f;
        uint32_t time_stamp    = 0;
    } mParams;
};

} // namespace rh::rw::engine