#pragma once
#include "D3D11Buffer.h"
namespace RHEngine {
    class D3D11DynamicIndexBuffer :
        public D3D11Buffer
    {
    public:
        D3D11DynamicIndexBuffer(ID3D11Device* device, unsigned int maxIndexCount);
        ~D3D11DynamicIndexBuffer();
    };
};