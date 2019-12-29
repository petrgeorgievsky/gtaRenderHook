#pragma once
#include <Engine/Common/IGPUResource.h>
#include <Engine/Common/irenderingpipeline.h>

namespace rh::engine {
class IRenderingContext;
class IGPUResource;
}; // namespace rh::engine

class GBufferPipeline : public rh::engine::IRenderingPipeline
{
public:
    GBufferPipeline();
    virtual ~GBufferPipeline() override;
    virtual void DrawMesh( rh::engine::IRenderingContext *context,
                           rh::engine::IPrimitiveBatch *model ) override;

private:
    rh::engine::GPUResourcePtr mBaseVS;
    rh::engine::GPUResourcePtr mNoTexPS;
    rh::engine::GPUResourcePtr mTexPS;
    rh::engine::GPUResourcePtr mVertexDecl;
};
