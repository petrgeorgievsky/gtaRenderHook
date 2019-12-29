#pragma once
#include <Engine/Common/irenderingpipeline.h>

namespace rh::engine {
class IRenderingContext;
class D3D11ConstantBuffer;
class D3D11PrimitiveBatch;
class IGPUResource;
}; // namespace rh::engine

struct MaterialCB
{
    float MaterialColor[4] = {1,1,1,1};
    float Roughness=1.0f;
    float Metallness=0;
    float IOR;
    float Emmisivness;
};

class ForwardPBRPipeline : public rh::engine::IRenderingPipeline
{
public:
    ForwardPBRPipeline();
    virtual ~ForwardPBRPipeline() override;
    virtual void DrawMesh( rh::engine::IRenderingContext *context,
                           rh::engine::IPrimitiveBatch *model ) override;

private:
    MaterialCB mDefMaterialCBData{};
    rh::engine::IGPUResource *mMaterialCB = nullptr;
    rh::engine::IGPUResource *mBaseVS = nullptr;
    rh::engine::IGPUResource *mNoTexPS = nullptr;
    rh::engine::IGPUResource *mTexPS = nullptr;
    rh::engine::IGPUResource *mVertexDecl = nullptr;
    rh::engine::IGPUResource *mSelectedPS = nullptr;
};
