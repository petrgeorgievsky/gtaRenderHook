//
// Created by peter on 15.05.2020.
//
#pragma once
#include <Engine/Common/ArrayProxy.h>
#include <Engine/Common/IDeviceState.h>
#include <Engine/Common/ScopedPtr.h>
#include <cstdint>
#include <rw_engine/rh_backend/skinned_mesh_backend.h>
#include <vector>

namespace rh::engine
{
class VulkanComputePipeline;
class VulkanCommandBuffer;
class IDescriptorSetAllocator;
class IPipelineLayout;
class IDescriptorSetLayout;
class IDescriptorSet;
class IBuffer;
class IShader;
class ISyncPrimitive;
} // namespace rh::engine
namespace rh::rw::engine
{
struct AnimatedMeshDrawCall
{
    uint64_t            mInstanceId;
    uint64_t            mMaterialListStart;
    uint64_t            mMaterialListCount;
    SkinMeshData        mData;
    DirectX::XMFLOAT4X3 mTransform;
};
using rh::engine::ScopedPointer;
class SkinAnimationPipeline
{
  public:
    SkinAnimationPipeline( uint32_t max_anims );
    ~SkinAnimationPipeline();

    std::vector<AnimatedMeshDrawCall> AnimateSkinnedMeshes(
        const rh::engine::ArrayProxy<SkinDrawCallInfo> &draw_calls );
    rh::engine::CommandBufferSubmitInfo
    GetAnimateSubmitInfo( rh::engine::ISyncPrimitive *dependency );

  private:
    ScopedPointer<rh::engine::IShader>                 mAnimShader;
    ScopedPointer<rh::engine::VulkanComputePipeline>   mPipeline;
    ScopedPointer<rh::engine::IPipelineLayout>         mPipelineLayout;
    ScopedPointer<rh::engine::IDescriptorSetLayout>    mDescSetLayout;
    ScopedPointer<rh::engine::IDescriptorSetAllocator> mDescAllocator;
    ScopedPointer<rh::engine::VulkanCommandBuffer>     mCmdBuffer;
    ScopedPointer<rh::engine::ISyncPrimitive>          mAnimateFinish;

    std::vector<rh::engine::IDescriptorSet *> mDescSetPool;
    std::vector<rh::engine::IBuffer *>        mBoneMatrixPool;
    std::unordered_map<uint64_t, std::array<DirectX::XMFLOAT4X3, 256>>
             mAnimationCache;
    uint32_t mMaxAnims{};
};
} // namespace rh::rw::engine