#pragma once
#include "types/shader_stage.h"
#include "types/descriptor_type.h"
#include "ArrayProxy.h"
#include <cstdint>

namespace rh::engine
{

struct DescriptorBinding
{
    std::uint32_t  mBindingId;
    DescriptorType mDescriptorType;
    std::uint32_t  mCount;
    std::uint32_t  mShaderStages;
    // D3D11 specific
    std::uint32_t  mRegisterId;
};
struct DescriptorSetLayoutCreateParams
{
    ArrayProxy<DescriptorBinding> mBindings;
};
class IDescriptorSetLayout
{
  public:
    virtual ~IDescriptorSetLayout()                      = default;
    IDescriptorSetLayout()                               = default;
    IDescriptorSetLayout( const IDescriptorSetLayout & ) = delete;
};
} // namespace rh::engine