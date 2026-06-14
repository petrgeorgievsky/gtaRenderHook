#include "VulkanDescriptorSet.h"

#include "VulkanCommon.h"

#include <DebugUtils/DebugLogger.h>
namespace rh::engine
{
VulkanDescriptorSet::VulkanDescriptorSet(
    const VulkanDescriptorSetCreateParams &create_params )
    : mDevice( create_params.mDevice ), mPool( create_params.mPool ),
      mDescSet( create_params.mDescSet )
{
}

VulkanDescriptorSet::~VulkanDescriptorSet()
{
    CALL_VK_API( mDevice.freeDescriptorSets( mPool, { mDescSet } ),
                 TEXT( "Failed to free descriptor sets!" ) );
}

DescriptorType VulkanDescriptorSet::GetType( uint32_t /*binding_id*/ )
{
    return DescriptorType::Sampler;
}
} // namespace rh::engine