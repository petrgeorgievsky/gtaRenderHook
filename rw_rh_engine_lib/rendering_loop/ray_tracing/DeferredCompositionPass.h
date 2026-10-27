//
// Created by peter on 04.07.2020.
//
#pragma once
#include <Engine/Common/ScopedPtr.h>
#include <cstdint>
#include <rendering_loop/render_texture_buffer.h>
namespace rh::engine
{
class IImageView;
class IImageBuffer;
class ICommandBuffer;
class IShader;
class VulkanComputePipeline;
class IPipelineLayout;
class IDescriptorSetLayout;
class IDescriptorSetAllocator;
class IDescriptorSet;
class IBuffer;
class IDeviceState;
} // namespace rh::engine
namespace rh::rw::engine
{
using rh::engine::ScopedPointer;
class CameraDescription;
struct DeferredCompositionPassParams
{
    rh::engine::IDeviceState &Device;
    CameraDescription *       Camera;

    rh::engine::IImageView *mAlbedoBuffer;
    rh::engine::IImageView *mNormalDepthBuffer;
    rh::engine::IImageView *mMaterialParamsBuffer;
    rh::engine::IImageView *mAOBuffer;
    rh::engine::IImageView *mLightingBuffer;
    rh::engine::IImageView *mReflectionBuffer;
    rh::engine::IBuffer *   mSkyCfg;
    uint32_t                mWidth;
    uint32_t                mHeight;
};
class DeferredCompositionPass
{
  public:
    DeferredCompositionPass( const DeferredCompositionPassParams &params );

    void                    Execute( rh::engine::ICommandBuffer *dest );
    rh::engine::IImageView *GetResultView() { return mOutputBuffer.View; }

    // Rebinds the lighting input, so the denoiser can be toggled at runtime.
    void SetLightingBuffer( rh::engine::IImageView *view );

    // 0 = final image, 1 = lighting (shadows), 2 = AO, 3 = reflections.
    void SetDebugView( uint32_t mode );

  private:
    DeferredCompositionPassParams                      mPassParams;
    RenderTextureBuffer                                mOutputBuffer;
    ScopedPointer<rh::engine::IShader>                 mCompositionShader;
    ScopedPointer<rh::engine::VulkanComputePipeline>   mPipeline;
    ScopedPointer<rh::engine::IPipelineLayout>         mPipelineLayout;
    ScopedPointer<rh::engine::IDescriptorSetLayout>    mDescSetLayout;
    ScopedPointer<rh::engine::IDescriptorSetAllocator> mDescAllocator;
    ScopedPointer<rh::engine::IDescriptorSet>          mDescSet;
    ScopedPointer<rh::engine::IBuffer>                 mDebugParamsBuffer;
    uint32_t                                           mDebugView = 0;
};
} // namespace rh::rw::engine