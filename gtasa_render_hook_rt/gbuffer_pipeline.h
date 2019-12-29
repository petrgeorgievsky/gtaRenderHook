#pragma once
#include <Engine/Common/irenderingpipeline.h>

namespace RHEngine {
class IRenderingContext;
}; // namespace RHEngine

class GBufferPipeline : public RHEngine::IRenderingPipeline
{
public:
    GBufferPipeline();
    virtual ~GBufferPipeline() override;
    virtual void DrawMesh( RHEngine::IRenderingContext *context,
                           RHEngine::IPrimitiveBatch *model ) override;

    void PrepareFrame( RHEngine::IRenderingContext *context );
    void EndFrame( RHEngine::IRenderingContext *context );
    void *GetGBuffer( uint32_t id );

private:
    void *mBaseVS = nullptr;
    void *mNoTexPS = nullptr;
    void *mTexPS = nullptr;
    void *mVertexDecl = nullptr;
    void *mGBufferTexture[4];
    void *mGBufferDepthTexture;
};
