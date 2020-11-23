//
// Created by peter on 26.10.2020.
//
#pragma once

#include <Engine/Common/ScopedPtr.h>
#include <cstdint>
namespace rh::engine
{
class IImageView;
class IImageBuffer;
class ICommandBuffer;
class IBuffer;
class IShader;
class VulkanComputePipeline;
class IPipelineLayout;
class IDescriptorSetLayout;
class IDescriptorSetAllocator;
class IDescriptorSet;
} // namespace rh::engine

namespace rh::rw::engine
{
template <typename T> using SPtr = rh::engine::ScopedPointer<T>;
struct DebugPipelineInitParams
{
    uint32_t                mWidth;
    uint32_t                mHeight;
    rh::engine::IImageView *mDebugView{};
    rh::engine::IBuffer *   mTiledLightsList{};
};
class DebugPipeline
{
  public:
    DebugPipeline( const DebugPipelineInitParams &params );

    void                    Execute( rh::engine::ICommandBuffer *dest );
    rh::engine::IImageView *GetResultView() { return mOutputBufferView; }

  private:
    uint32_t                                  mWidth;
    uint32_t                                  mHeight;
    SPtr<rh::engine::IImageBuffer>            mOutputBuffer;
    SPtr<rh::engine::IImageView>              mOutputBufferView;
    SPtr<rh::engine::IShader>                 mCompositionShader;
    SPtr<rh::engine::VulkanComputePipeline>   mPipeline;
    SPtr<rh::engine::IPipelineLayout>         mPipelineLayout;
    SPtr<rh::engine::IDescriptorSetLayout>    mDescSetLayout;
    SPtr<rh::engine::IDescriptorSetAllocator> mDescAllocator;
    SPtr<rh::engine::IDescriptorSet>          mDescSet;
};

} // namespace rh::rw::engine