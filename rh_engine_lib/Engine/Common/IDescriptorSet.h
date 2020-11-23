#pragma once
#include "types/descriptor_type.h"
#include "types/shader_stage.h"
#include <cstdint>

namespace rh::engine
{
class IDescriptorSet
{
  public:
    virtual ~IDescriptorSet()                             = default;
    IDescriptorSet()                                      = default;
    IDescriptorSet( const IDescriptorSet & )              = delete;
    virtual DescriptorType GetType( uint32_t binding_id ) = 0;
};
} // namespace rh::engine