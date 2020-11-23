#pragma once
#include "Engine/Common/IDescriptorSetLayout.h"

namespace rh::engine
{
class D3D11DescriptorSetLayout : public IDescriptorSetLayout
{
  public:
    D3D11DescriptorSetLayout( const DescriptorSetLayoutCreateParams &desc );
    ~D3D11DescriptorSetLayout() override;
    friend class D3D11DescriptorSet;

  private:
    std::vector<DescriptorBinding> mDescription;
};

} // namespace rh::engine
