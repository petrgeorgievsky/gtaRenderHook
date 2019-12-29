#pragma once
#include <common.h>

namespace RHEngine
{
class IRenderingContext;
class D3D11ConstantBuffer;
class D3D11PrimitiveBatch;
};

struct RHVertexDescPosOnly
{
    float x, y, z, w;
};

struct RHVertexDescPosColor : RHVertexDescPosOnly
{
    RwRGBA color;
};

struct RHVertexDescPosColorUV : RHVertexDescPosColor
{
    float u, v;
};

struct RHVertexDescPosColorUVNormals : RHVertexDescPosColorUV
{
    float nx, ny, nz;
};

struct MaterialCB
{
    float MaterialColor[4] = {1,1,1,1};
    float Roughness=1.0f;
    float Metallness=0;
    float IOR;
    float Emmisivness;
};

class IMaterialBufferProvider 
{
public:
    virtual MaterialCB* GetBuffer( std::string texture_name ) = 0;
};

struct PBRRenderingParams 
{
    bool highlightModel = false;
};

class ForwardPBRPipeline 
{
public:
    ForwardPBRPipeline( IMaterialBufferProvider* matbuf_prov );
    ~ForwardPBRPipeline();
    void DrawObject( RHEngine::IRenderingContext* context, RHEngine::D3D11PrimitiveBatch* model, const PBRRenderingParams& params );
private:
    MaterialCB mDefMaterialCBData{};
    RHEngine::D3D11ConstantBuffer* mMaterialCB = nullptr;
    IMaterialBufferProvider* mMatBufferProvider = nullptr;
    void* mBaseVS = nullptr;
    void* mNoTexPS = nullptr;
    void* mTexPS = nullptr;
    void* mVertexDecl = nullptr;
    void* mSelectedPS = nullptr;
};