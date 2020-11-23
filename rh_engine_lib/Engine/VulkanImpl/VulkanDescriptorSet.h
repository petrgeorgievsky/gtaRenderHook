#include "Engine/Common/IDescriptorSet.h"
#include <common.h>

namespace rh::engine
{
struct VulkanDescriptorSetCreateParams
{
    // Dependencies...
    vk::Device         mDevice;
    vk::DescriptorPool mPool;
    // Params
    vk::DescriptorSet mDescSet;
};

class VulkanDescriptorSet : public IDescriptorSet
{
  public:
    VulkanDescriptorSet( const VulkanDescriptorSetCreateParams &create_params );
    ~VulkanDescriptorSet();
    DescriptorType GetType( uint32_t binding_id ) override;
                   operator vk::DescriptorSet() { return mDescSet; }

  private:
    vk::Device         mDevice;
    vk::DescriptorPool mPool;
    vk::DescriptorSet  mDescSet;
};
} // namespace rh::engine