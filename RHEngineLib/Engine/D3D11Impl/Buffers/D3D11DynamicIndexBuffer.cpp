#include "stdafx.h"
#include "D3D11DynamicIndexBuffer.h"

RHEngine::D3D11DynamicIndexBuffer::D3D11DynamicIndexBuffer(ID3D11Device* device, unsigned int maxIndexCount) :
    D3D11Buffer(device, { maxIndexCount * sizeof(RwImVertexIndex), D3D11_USAGE_DYNAMIC, D3D11_BIND_INDEX_BUFFER, D3D11_CPU_ACCESS_WRITE })
{
}

RHEngine::D3D11DynamicIndexBuffer::~D3D11DynamicIndexBuffer()
{
}
