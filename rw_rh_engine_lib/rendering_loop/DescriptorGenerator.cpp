//
// Created by peter on 27.06.2020.
//

#include "DescriptorGenerator.h"
#include <Engine/Common/IDeviceState.h>
#include <render_driver/render_driver.h>
#include <rw_engine/system_funcs/rw_device_system_globals.h>

namespace rh::rw::engine
{
using namespace rh::engine;
DescriptorGenerator::DescriptorGenerator( IDeviceState &device )
    : Device( device )
{
}
IDescriptorSetLayout *
DescriptorGenerator::FinalizeDescriptorSet( uint32_t set, uint32_t max_count )
{
    DescriptorSets[set].max_count = max_count;
    return Device.CreateDescriptorSetLayout( { DescriptorSets[set].bindings } );
}

rh::engine::IDescriptorSetAllocator *DescriptorGenerator::FinalizeAllocator()
{
    uint32_t                        max_sets = 0;
    std::vector<DescriptorPoolSize> pool_sizes;
    for ( auto &&[id, info] : DescriptorSets )
    {
        for ( const auto &binding : info.bindings )
        {
            auto pool_iter =
                std::ranges::find_if( pool_sizes, [&binding]( auto pool ) {
                    return pool.mType == binding.mDescriptorType;
                } );

            if ( pool_iter == pool_sizes.end() )
                pool_sizes.push_back( { binding.mDescriptorType,
                                        binding.mCount * info.max_count } );
            else
                pool_iter->mCount += binding.mCount * info.max_count;
        }
        max_sets += info.max_count;
    }

    return Device.CreateDescriptorSetAllocator(
        { .mDescriptorPools = pool_sizes, .mMaxSets = max_sets } );
}

DescriptorGenerator &DescriptorGenerator::AddDescriptor(
    uint32_t set, uint32_t binding, uint32_t register_id,
    rh::engine::DescriptorType type, uint32_t count, uint32_t shader_stages,
    uint16_t flags )
{
    DescriptorSets[set].bindings.push_back(
        { binding, type, count, shader_stages, flags, register_id } );
    return *this;
}
} // namespace rh::rw::engine