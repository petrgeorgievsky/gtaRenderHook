#pragma once
#include "Common/IPipeline.h"

namespace rh::engine
{
class D3D11Pipeline : public IPipeline
{
  private:
    /* data */
  public:
    D3D11Pipeline( /* args */ );
    ~D3D11Pipeline() override;
};

} // namespace rh::engine
