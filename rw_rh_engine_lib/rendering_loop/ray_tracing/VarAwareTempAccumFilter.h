//
// Created by peter on 22.10.2020.
//
#pragma once

#include <Engine/Common/ScopedPtr.h>
#include <cstdint>

namespace rh::engine
{
class IDeviceState;
class IImageView;
class IShader;
class IPipelineLayout;
class VulkanComputePipeline;
class IBuffer;
class IImageBuffer;
class ISampler;
class ICommandBuffer;
class IDescriptorSetAllocator;
class IDescriptorSet;
class IDescriptorSetLayout;
} // namespace rh::engine

namespace rh::rw::engine
{
template <typename T> using SPtr = rh::engine::ScopedPointer<T>;

struct VATAPassParam
{
    // dependencies
    uint32_t                mWidth{};
    uint32_t                mHeight{};
    rh::engine::IImageView *mInputValue;
    rh::engine::IImageView *mPrevDepth;
    rh::engine::IImageView *mCurrentDepth;
    rh::engine::IImageView *mMotionVectors;
};

class VarAwareTempAccumFilterPipe;

class VATAFilterPass
{
  public:
    VATAFilterPass( VarAwareTempAccumFilterPipe *pipeline,
                    const VATAPassParam &        params );
    void Execute( rh::engine::ICommandBuffer *dest );

    rh::engine::IImageView *GetAccumulatedValue()
    {
        return mAccumulateValueView;
    }
    rh::engine::IImageView *GetBlurIntensity()
    {
        return mBlurStrengthValueView;
    }

  private:
    VarAwareTempAccumFilterPipe *    mParent{};
    SPtr<rh::engine::IDescriptorSet> mAccumDescSet;
    SPtr<rh::engine::IDescriptorSet> mReprojDescSet;
    uint32_t                         mWidth{};
    uint32_t                         mHeight{};
    struct TAParams
    {
        float accumulation = 0.95;
        float pad[3];
    } mParams;
    SPtr<rh::engine::IBuffer> mParamsBuffer;

    SPtr<rh::engine::IImageBuffer> mReProjectValueBuffer;
    SPtr<rh::engine::IImageView>   mReProjectValueView;
    SPtr<rh::engine::IImageBuffer> mTemporalSamplePerPixel[2];
    SPtr<rh::engine::IImageView>   mTemporalSamplePerPixelView[2];
    SPtr<rh::engine::IImageBuffer> mAccumulateValueBuffer;
    SPtr<rh::engine::IImageView>   mAccumulateValueView;
    SPtr<rh::engine::IImageBuffer> mBlurStrengthBuffer;
    SPtr<rh::engine::IImageView>   mBlurStrengthValueView;

    friend class VarAwareTempAccumFilterPipe;
};

/**
 * Temporal value accumulation filter, with variance estimation
 * In:
 * 1) Estimated value from current frame
 * 2) Previous frame depth buffer
 * 3) Current frame depth buffer
 * Out:
 * 1) Accumulated value & variance
 * 2) Blur intensity
 */
class VarAwareTempAccumFilterPipe
{
  public:
    VarAwareTempAccumFilterPipe( rh::engine::IDeviceState &device );

    VATAFilterPass *GetFilter( const VATAPassParam &params );

  private:
    rh::engine::IDeviceState &                Device;
    SPtr<rh::engine::IDescriptorSetAllocator> mDescSetAlloc;

    // Pipeline for re-projection pass
    SPtr<rh::engine::VulkanComputePipeline> mReProjectPipeline;
    SPtr<rh::engine::IShader>               mReProjectShader;
    SPtr<rh::engine::IPipelineLayout>       mReProjectLayout;
    SPtr<rh::engine::IDescriptorSetLayout>  mReProjectDescSetLayout;

    // Pipeline for accumulation pass
    SPtr<rh::engine::VulkanComputePipeline> mAccumulatePipeline;
    SPtr<rh::engine::IShader>               mAccumulateShader;
    SPtr<rh::engine::IPipelineLayout>       mAccumulateLayout;
    SPtr<rh::engine::IDescriptorSetLayout>  mAccumulateDescSetLayout;

    friend class VATAFilterPass;
};
} // namespace rh::rw::engine