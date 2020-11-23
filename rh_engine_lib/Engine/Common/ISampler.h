#pragma once
#include <Engine\Common\types\sampler.h>

namespace rh::engine
{
struct SamplerDesc
{
    // Params
    Sampler mInfo;
};

class ISampler
{
  public:
    ISampler( /* args */ ) = default;
    virtual ~ISampler()    = default;
};

} // namespace rh::engine
