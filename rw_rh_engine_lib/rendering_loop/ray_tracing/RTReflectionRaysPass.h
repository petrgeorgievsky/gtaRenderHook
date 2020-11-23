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

struct RTReflectionInitParams
{
    uint32_t                 mWidth;
    uint32_t                 mHeight;
    RTSceneDescription *     mScene;
    CameraDescription *      mCamera;
    BilateralFilterPipeline *mBilateralFilterPipe;
    rh::engine::IImageView * mNormalsView;
    rh::engine::IImageView * mMaterialsView;
    rh::engine::IBuffer *    mSkyCfg;
};

class RTReflectionRaysPass
{
  public:
    RTReflectionRaysPass( const RTReflectionInitParams &params );

    void Execute( void *tlas, rh::engine::ICommandBuffer *cmd_buffer,
                  const FrameInfo &frame );

    rh::engine::IImageView *GetReflectionView()
    {
        return mFilteredReflectionBufferView;
    }

  private:
    uint32_t                           mWidth;
    uint32_t                           mHeight;
    RTSceneDescription *               mScene;
    CameraDescription *                mCamera;
    ScopedPointer<BilateralFilterPass> mBilFil0;

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
};
} // namespace rh::rw::engine