#pragma once
#include <Engine/Common/ArrayProxy.h>
#include <Engine/Common/IDescriptorSet.h>
#include <Engine/Common/IDescriptorSetLayout.h>
#include <Engine/Common/types/descriptor_type.h>

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
    ArrayProxy<IDescriptorSetLayout *> mLayouts;
};

class IDescriptorSetAllocator
{
  public:
    virtual ~IDescriptorSetAllocator()                         = default;
    IDescriptorSetAllocator()                                  = default;
    IDescriptorSetAllocator( const IDescriptorSetAllocator & ) = delete;
    virtual std::vector<IDescriptorSet *>
    AllocateDescriptorSets( const DescriptorSetsAllocateParams & ) = 0;
};

} // namespace rh::engine