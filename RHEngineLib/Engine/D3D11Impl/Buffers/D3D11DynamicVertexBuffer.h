#pragma once
#include "D3D11Buffer.h"
namespace RHEngine {
    class D3D11DynamicVertexBuffer :
        public D3D11Buffer
    {
    public:
        D3D11DynamicVertexBuffer(ID3D11Device* device, unsigned int vertexSize, unsigned int maxVertexCount);
        ~D3D11DynamicVertexBuffer();
    };
};