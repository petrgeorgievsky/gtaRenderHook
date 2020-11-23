#pragma once
#include "Engine/Common/IDescriptorSetAllocator.h"

namespace rh::engine
{
class D3D11DescriptorSetAllocator : public IDescriptorSetAllocator
{
  public:
    D3D11DescriptorSetAllocator( const DescriptorSetAllocatorCreateParams& desc );
    ~D3D11DescriptorSetAllocator() override;

  private:
    std::vector<IDescriptorSet *>
    AllocateDescriptorSets( const DescriptorSetsAllocateParams & ) override;
};

} // namespace rh::engine
