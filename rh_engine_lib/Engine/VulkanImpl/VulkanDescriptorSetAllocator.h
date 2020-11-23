#pragma once
#include "Engine/Common/IDescriptorSetAllocator.h"
#include <common.h>

namespace rh::engine
{
struct VulkanDescriptorSetAllocatorCreateParams
{
    // Dependencies...
    vk::Device mDevice;
    // Params
    ArrayProxy<DescriptorPoolSize> mDescriptorPools;
    uint32_t                       mSetLimit;
};

class VulkanDescriptorSetAllocator: public IDescriptorSetAllocator
{
  public:
    VulkanDescriptorSetAllocator(
        const VulkanDescriptorSetAllocatorCreateParams &create_params );
    ~VulkanDescriptorSetAllocator() override;
    std::vector<IDescriptorSet *>
    AllocateDescriptorSets( const DescriptorSetsAllocateParams & ) override;

  private:
    vk::Device mDevice;
    vk::DescriptorPool mPool;
};
} // namespace rh::engine