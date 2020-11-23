#pragma once
#include "Engine/Common/IRenderPass.h"

namespace rh::engine
{

class D3D11RenderPass : public IRenderPass
{
  public:
    D3D11RenderPass( const RenderPassCreateParams &create_info );
    ~D3D11RenderPass() override;
    const RenderPassCreateParams &Info() { return mInfo; }

  private:
    RenderPassCreateParams mInfo;
};

} // namespace rh::engine