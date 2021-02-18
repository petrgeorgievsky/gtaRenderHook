//
// Created by peter on 27.06.2020.
//

#pragma once
#include <Engine/Common/IDescriptorSetLayout.h>
#include <map>

namespace rh::engine
{
class IDescriptorSetAllocator;
class IDeviceState;
} // namespace rh::engine

namespace rh::rw::engine
{

class DescriptorGenerator
{
    struct DescriptorSetDesc
    {
        std::vector<rh::engine::DescriptorBinding> bindings;
        uint32_t                                   max_count = 0;
    };
    rh::engine::IDeviceState &            Device;
    std::map<uint32_t, DescriptorSetDesc> DescriptorSets;

  public:
    DescriptorGenerator( rh::engine::IDeviceState &device );
    DescriptorGenerator &AddDescriptor( uint32_t set, uint32_t binding,
                                        uint32_t                   register_id,
                                        rh::engine::DescriptorType type,
                                        uint32_t count, uint32_t shader_stages,
                                        uint16_t flags = 0 );

    rh::engine::IDescriptorSetLayout *
    FinalizeDescriptorSet( uint32_t set, uint32_t max_count );

    rh::engine::IDescriptorSetAllocator *FinalizeAllocator();
};
} // namespace rh::rw::engine