//
// Created by peter on 02.08.2020.
//
#pragma once

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
} // namespace rh::engine

namespace rh::rw::engine
{
class ComputeVelocityBuffer
{
  public:
    ComputeVelocityBuffer( rh::engine::IImageView *world_pos );

    void                    Execute( rh::engine::ICommandBuffer *dest );
    rh::engine::IImageView *GetResultView() { return mOutputBufferView; }

  private:
    rh::engine::IImageBuffer *           mOutputBuffer;
    rh::engine::IImageView *             mOutputBufferView;
    rh::engine::IShader *                mCompositionShader;
    rh::engine::VulkanComputePipeline *  mPipeline;
    rh::engine::IPipelineLayout *        mPipelineLayout;
    rh::engine::IDescriptorSetLayout *   mDescSetLayout;
    rh::engine::IDescriptorSetAllocator *mDescAllocator;
    rh::engine::IDescriptorSet *         mDescSet;
};

} // namespace rh::rw::engine