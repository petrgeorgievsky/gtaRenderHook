//
// Created by peter on 27.06.2020.
//

#pragma once
#include "RTSceneDescription.h"
#include <Engine/Common/ScopedPtr.h>
namespace rh::engine
{
class IDescriptorSetAllocator;
class IDescriptorSetLayout;
class IDescriptorSet;
class IPipelineLayout;
class IShader;
class IBuffer;
class IImageBuffer;
class IImageView;
class ISampler;
class VulkanComputePipeline;
class ICommandBuffer;
class VulkanRayTracingPipeline;
} // namespace rh::engine

namespace rh::rw::engine
{
template <typename T> using SPtr = ScopedPointer<T>;

class CameraDescription;
class BilateralFilterPipeline;
class BilateralFilterPass;
class VATAColorFilterPass;
class VarAwareTempAccumColorFilterPipe;

struct RTReflectionInitParams
{
    rh::engine::IDeviceState &Device;

    uint32_t                          mWidth;
    uint32_t                          mHeight;
    RTSceneDescription *              mScene;
    CameraDescription *               mCamera;
    VarAwareTempAccumColorFilterPipe *mVarTAColorFilterPipe;
    BilateralFilterPipeline *         mBilateralFilterPipe;
    rh::engine::IImageView *          mNormalsView;
    rh::engine::IImageView *          mPrevNormalsView;
    rh::engine::IImageView *          mMotionVectorsView;
    rh::engine::IImageView *          mMaterialsView;
    rh::engine::IBuffer *             mSkyCfg;
};

class RTReflectionRaysPass
{
  public:
    RTReflectionRaysPass( const RTReflectionInitParams &params );

    void Execute( void *tlas, rh::engine::ICommandBuffer *cmd_buffer );

    rh::engine::IImageView *GetReflectionView()
    {
        return mFilteredReflectionBufferView;
    }

  private:
    rh::engine::IDeviceState &         Device;
    uint32_t                           mWidth;
    uint32_t                           mHeight;
    RTSceneDescription *               mScene;
    CameraDescription *                mCamera;
    ScopedPointer<BilateralFilterPass> mBilFil0;
    ScopedPointer<VATAColorFilterPass> mVarTAColorPass;

    SPtr<rh::engine::IDescriptorSetAllocator> mDescSetAlloc;
    SPtr<rh::engine::IDescriptorSetLayout>    mRayTraceSetLayout;
    SPtr<rh::engine::IDescriptorSet>          mRayTraceSet;
    SPtr<rh::engine::IDescriptorSetLayout>    mBlurStrSetLayout;
    SPtr<rh::engine::IDescriptorSet>          mBlurStrSet;

    SPtr<rh::engine::IPipelineLayout>          mPipeLayout;
    SPtr<rh::engine::VulkanRayTracingPipeline> mPipeline;

    SPtr<rh::engine::IPipelineLayout>       mBlurStrPipeLayout;
    SPtr<rh::engine::VulkanComputePipeline> mBlurStrPipeline;

    SPtr<rh::engine::IShader> mRayGenShader;
    SPtr<rh::engine::IShader> mClosestHitShader;
    SPtr<rh::engine::IShader> mAnyHitShader;
    SPtr<rh::engine::IShader> mMissShader;
    SPtr<rh::engine::IShader> mShadowMissShader;

    SPtr<rh::engine::IShader> mRefRoughShader;

    SPtr<rh::engine::IBuffer> mShaderBindTable;

    SPtr<rh::engine::IImageBuffer> mReflectionBuffer;
    SPtr<rh::engine::IImageView>   mReflectionBufferView;
    SPtr<rh::engine::IImageBuffer> mReflectionBlurStrBuffer;
    SPtr<rh::engine::IImageView>   mReflectionBlurStrBufferView;
    SPtr<rh::engine::IImageBuffer> mTempBlurReflectionBuffer;
    SPtr<rh::engine::IImageView>   mTempBlurReflectionBufferView;
    SPtr<rh::engine::IImageBuffer> mFilteredReflectionBuffer;
    SPtr<rh::engine::IImageView>   mFilteredReflectionBufferView;

    SPtr<rh::engine::IImageBuffer> mNoiseBuffer;
    SPtr<rh::engine::IImageView>   mNoiseBufferView;

    SPtr<rh::engine::ISampler> mTextureSampler;

    ScopedPointer<rh::engine::IBuffer> mParamsBuffer;

    struct ReflParams
    {
        float    min_distance  = 0.001f;
        float    max_distance  = 10.0f;
        float    max_draw_dist = 1000.0f;
        uint32_t time_stamp    = 0;
    } mParams;
};
} // namespace rh::rw::engine