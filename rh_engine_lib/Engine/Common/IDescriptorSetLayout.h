#pragma once
#include "ArrayProxy.h"
#include "types/descriptor_type.h"
#include "types/shader_stage.h"
#include <cstdint>

namespace rh::engine
{

enum DescriptorFlags : uint16_t
{
    dfUpdateAfterBind          = 0x1,
    dfUpdateUnusedWhilePending = 0x10,
    dfPartiallyBound           = 0x100,
    dfVariableDescriptorCount  = 0x1000,
};

struct DescriptorBinding
{
    std::uint32_t  mBindingId;
    DescriptorType mDescriptorType;
    std::uint32_t  mCount;
    std::uint32_t  mShaderStages;
    std::uint16_t  mFlags;
    // D3D11 specific
    std::uint32_t mRegisterId;
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