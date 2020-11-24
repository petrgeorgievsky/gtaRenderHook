#include "VulkanDescriptorSetAllocator.h"
#include "VulkanConvert.h"
#include "VulkanDescriptorSet.h"
#include "VulkanDescriptorSetLayout.h"
#include <DebugUtils/DebugLogger.h>

using namespace rh::engine;
VulkanDescriptorSetAllocator::VulkanDescriptorSetAllocator(
    const VulkanDescriptorSetAllocatorCreateParams &create_params )
    : mDevice( create_params.mDevice )
{
    vk::DescriptorPoolCreateInfo        ci_impl{};
    std::vector<vk::DescriptorPoolSize> pool_sizes{};
    pool_sizes.reserve( create_params.mDescriptorPools.Size() );
    // TODO: add ability to free desc_pool entries
    std::ranges::transform( create_params.mDescriptorPools,
                            std::back_inserter( pool_sizes ),
                            []( const DescriptorPoolSize &pool_size ) {
                                vk::DescriptorPoolSize vk_impl{};
                                vk_impl.descriptorCount = pool_size.mCount;
                                vk_impl.type = Convert( pool_size.mType );
                                return vk_impl;
                            } );

    ci_impl.maxSets       = create_params.mSetLimit;
    ci_impl.poolSizeCount = static_cast<uint32_t>( pool_sizes.size() );
    ci_impl.pPoolSizes    = pool_sizes.data();
    ci_impl.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;
    mPool         = mDevice.createDescriptorPool( ci_impl );
}

VulkanDescriptorSetAllocator::~VulkanDescriptorSetAllocator()
{
    mDevice.destroyDescriptorPool( mPool );
}

std::vector<IDescriptorSet *>
rh::engine::VulkanDescriptorSetAllocator::AllocateDescriptorSets(
    const DescriptorSetsAllocateParams &params )
{
    vk::DescriptorSetAllocateInfo alloc_i{};
    alloc_i.descriptorPool = mPool;
    std::vector<vk::DescriptorSetLayout> set_layouts;
    set_layouts.reserve( params.mLayouts.Size() );
    std::ranges::for_each(
        params.mLayouts, [&set_layouts]( IDescriptorSetLayout *layout ) {
            set_layouts.push_back(
                *dynamic_cast<VulkanDescriptorSetLayout *>( layout ) );
        } );

    alloc_i.pSetLayouts        = set_layouts.data();
    alloc_i.descriptorSetCount = static_cast<uint32_t>( set_layouts.size() );
    auto result_impl           = mDevice.allocateDescriptorSets( alloc_i );

    std::vector<IDescriptorSet *> result{};
    result.reserve( result_impl.size() );

    std::ranges::transform( result_impl, std::back_inserter( result ),
                            [this]( const auto &desc ) {
                                return new VulkanDescriptorSet( {
                                    .mDevice  = mDevice,
                                    .mPool    = mPool,
                                    .mDescSet = desc,
                                } );
                            } );
    return result;
}
