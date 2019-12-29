#pragma once
#include "../Common/ISimple2DRenderer.h"
#include "Buffers\D3D11DynamicIndexBuffer.h"
#include "Buffers\D3D11DynamicVertexBuffer.h"
#include "D3D11Common.h"
#include "D3D11GPUAllocator.h"
#include "D3D11InputLayout.h"
#include "Engine/Common/IGPUResource.h"
#include "ImageBuffers/D3D11Texture2D.h"
#include "Shaders\D3D11PixelShader.h"
#include "Shaders\D3D11VertexShader.h"

namespace rh::engine {

struct VertexBatch
{
    unsigned int offset;
    unsigned int size;
};

class D3D11Im2DPipeline final : public ISimple2DRenderer
{
public:
    D3D11Im2DPipeline( IGPUAllocator &allocator );
    ~D3D11Im2DPipeline() override;

    void Shutdown() override;

    void Draw( void *impl,
               PrimitiveType prim,
               Simple2DVertex *verticles,
               unsigned int vertexCount ) override;

    void DrawIndexed( void *impl,
                      PrimitiveType prim,
                      Simple2DVertex *vertices,
                      unsigned int numVertices,
                      VertexIndex *indices,
                      unsigned int numIndices ) override;

    void BindTexture( void *texture ) override;

private:
    static const unsigned int m_nVertexBufferCapacity;
    static const unsigned int m_nIndexBufferCapacity;

    GPUResourcePtr m_pVertexDecl = nullptr;
    GPUResourcePtr m_pVertexBuffer = nullptr;
    void *m_pTextureContext = nullptr;
    GPUResourcePtr m_pIndexBuffer = nullptr;
    GPUResourcePtr m_pBaseVS = nullptr;
    GPUResourcePtr m_pNoTexPS = nullptr;
    GPUResourcePtr m_pTexPS = nullptr;
};
}; // namespace rh::engine
