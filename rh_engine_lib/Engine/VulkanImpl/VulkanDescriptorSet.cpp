#include "VulkanDescriptorSet.h"
#include <DebugUtils/DebugLogger.h>

rh::engine::VulkanDescriptorSet::VulkanDescriptorSet(
    const VulkanDescriptorSetCreateParams &create_params )
    : mDevice( create_params.mDevice ), mPool( create_params.mPool ),
      mDescSet( create_params.mDescSet )
{
    std::stringstream ss;
    ss << "create dset" << std::hex
       << reinterpret_cast<uint64_t>( (VkDescriptorSet)mDescSet );
    rh::debug::DebugLogger::Log( ss.str() );
}

rh::engine::VulkanDescriptorSet::~VulkanDescriptorSet()
{
    std::stringstream ss;
    ss << "destroy dset" << std::hex << reinterpret_cast<uint64_t>( this );
    rh::debug::DebugLogger::Log( ss.str() );
    mDevice.freeDescriptorSets( mPool, { mDescSet } );
}
rh::engine::DescriptorType
rh::engine::VulkanDescriptorSet::GetType( uint32_t binding_id )
{
    return DescriptorType::Sampler;
}
