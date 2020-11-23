#pragma once
#include "ArrayProxy.h"
#include "types/shader_stage.h"
#include "types/descriptor_type.h"
#include "IDescriptorSet.h"
#include "IDescriptorSetLayout.h"
#include <cstdint>

namespace rh::engine
{

struct DescriptorPoolSize
{
    DescriptorType mType;
    std::uint32_t  mCount;
};

struct DescriptorSetAllocatorCreateParams
{
    ArrayProxy<DescriptorPoolSize> mDescriptorPools;
    std::uint32_t                  mMaxSets;
};

struct DescriptorSetsAllocateParams
{
    ArrayProxy<IDescriptorSetLayout*> mLayouts;
};

class IDescriptorSetAllocator
{
  public:
    virtual ~IDescriptorSetAllocator()                   = default;
    IDescriptorSetAllocator()                            = default;
    IDescriptorSetAllocator( const IDescriptorSetAllocator & ) = delete;
    virtual std::vector<IDescriptorSet*> AllocateDescriptorSets( const DescriptorSetsAllocateParams &) = 0;
};
} // namespace rh::engine