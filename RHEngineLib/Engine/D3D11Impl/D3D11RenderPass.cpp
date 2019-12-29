#include "D3D11RenderPass.h"

using namespace rh::engine;

D3D11RenderPass::D3D11RenderPass( const RenderPassCreateParams &create_info )
{
    mInfo = create_info;
}

D3D11RenderPass::~D3D11RenderPass() {}
