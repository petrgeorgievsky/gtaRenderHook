//
// Created by peter on 30.06.2020.
//
#include <Engine/Common/ScopedPtr.h>
#include <vector>
namespace rh::engine
{
class VulkanComputePipeline;
class IDescriptorSetAllocator;
class IPipelineLayout;
class IDescriptorSetLayout;
class IDescriptorSet;
class IBuffer;
class IShader;
class ICommandBuffer;
class IImageView;
class IImageBuffer;
} // namespace rh::engine

namespace rh::rw::engine
{
using rh::engine::ScopedPointer;
struct BilateralFilterPassParams
{
    rh::engine::IImageView *mInputImage;
    rh::engine::IImageView *mTempImage;
    rh::engine::IImageView *mOutputImage;
    rh::engine::IImageView *mNormalDepthBuffer;
    rh::engine::IImageView *mBlurStrength = nullptr;

    rh::engine::IImageBuffer *mTempImageBuffer;
    rh::engine::IImageBuffer *mOutputImageBuffer;
};
class BilateralFilterPipeline;

class BilateralFilterPass
{
  public:
    BilateralFilterPass( BilateralFilterPipeline *        pipeline,
                         const BilateralFilterPassParams &params );
    void Execute( rh::engine::ICommandBuffer *dest );

  private:
    BilateralFilterPassParams                 mPassParams;
    ScopedPointer<rh::engine::IDescriptorSet> mDescSetH;
    ScopedPointer<rh::engine::IDescriptorSet> mDescSetV;
    BilateralFilterPipeline *                 mParent;
    rh::engine::IImageBuffer *                mTempImageBuffer;
    rh::engine::IImageBuffer *                mOutputImageBuffer;
    bool                                      mDynamic = false;
    friend class BilateralFilterPipeline;
};

class BilateralFilterPipeline
{
  public:
    BilateralFilterPipeline();
    BilateralFilterPass *GetPass( const BilateralFilterPassParams &params );

  private:
    ScopedPointer<rh::engine::IShader>               mBilateralBlurShader;
    ScopedPointer<rh::engine::VulkanComputePipeline> mPipeline;
    ScopedPointer<rh::engine::IPipelineLayout>       mPipelineLayout;
    ScopedPointer<rh::engine::IDescriptorSetLayout>  mDescSetLayout;

    ScopedPointer<rh::engine::IShader> mDynamicBilateralBlurShader;
    ScopedPointer<rh::engine::IShader> mDynamicBilateralBlurHShader;
    ScopedPointer<rh::engine::IShader> mDynamicBilateralBlurVShader;
    ScopedPointer<rh::engine::VulkanComputePipeline> mPipelineDynamicBlur;
    ScopedPointer<rh::engine::VulkanComputePipeline> mPipelineDynamicBlurH;
    ScopedPointer<rh::engine::VulkanComputePipeline> mPipelineDynamicBlurV;
    ScopedPointer<rh::engine::IPipelineLayout>       mPipelineDynamicBlurLayout;
    ScopedPointer<rh::engine::IDescriptorSetLayout>  mDynamicBlurDescSetLayout;

    ScopedPointer<rh::engine::IDescriptorSetAllocator> mDescAllocator;
    BilateralFilterPassParams                          mPassParams{};
    std::vector<rh::engine::IDescriptorSet *>          mDescSetPool;
    friend class BilateralFilterPass;
};
} // namespace rh::rw::engine
