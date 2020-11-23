#pragma once
#include <Engine/Common/irenderingpipeline.h>

namespace RHEngine
{
class IRenderingContext;
class D3D11ConstantBuffer;
class D3D11PrimitiveBatch;
}

struct MaterialCB
{
    float MaterialColor[4] = {1,1,1,1};
    float Roughness=1.0f;
    float Metallness=0;
    float IOR;
    float Emmisivness;
};

class ForwardPBRPipeline : public RHEngine::IRenderingPipeline
{
public:
    ForwardPBRPipeline();
    virtual ~ForwardPBRPipeline() override;
    virtual void DrawMesh( RHEngine::IRenderingContext *context,
                           RHEngine::IPrimitiveBatch *model ) override;

private:
    MaterialCB mDefMaterialCBData{};
    void *mMaterialCB = nullptr;
    void* mBaseVS = nullptr;
    void* mNoTexPS = nullptr;
    void* mTexPS = nullptr;
    void* mVertexDecl = nullptr;
    void* mSelectedPS = nullptr;
};
