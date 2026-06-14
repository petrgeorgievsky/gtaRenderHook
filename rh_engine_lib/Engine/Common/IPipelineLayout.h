#pragma once
#include <Engine/Common/ArrayProxy.h>
#include <Engine/Common/IDescriptorSetLayout.h>

namespace rh::engine
{

class IRenderPass;
class IShader;

struct PipelineLayoutCreateParams
{
    ArrayProxy<IDescriptorSetLayout *> mSetLayouts;
};

class IPipelineLayout
{
  public:
    virtual ~IPipelineLayout()                 = default;
    IPipelineLayout()                          = default;
    IPipelineLayout( const IPipelineLayout & ) = delete;
};

} // namespace rh::engine