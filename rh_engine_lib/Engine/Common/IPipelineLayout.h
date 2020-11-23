#pragma once
#include "types/shader_stage.h"
#include <string>
#include "ArrayProxy.h"
#include "IDescriptorSetLayout.h"

namespace rh::engine
{
class IRenderPass;
class IShader;
struct PipelineLayoutCreateParams
{
    ArrayProxy<IDescriptorSetLayout*> mSetLayouts;
};
class IPipelineLayout
{
  public:
    virtual ~IPipelineLayout()                 = default;
    IPipelineLayout()                          = default;
    IPipelineLayout( const IPipelineLayout & ) = delete;
};
} // namespace rh::engine