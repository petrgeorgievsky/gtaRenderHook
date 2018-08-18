#include "stdafx.h"
#include "D3D11Buffer.h"
#include "../D3D11Common.h"

RHEngine::D3D11Buffer::D3D11Buffer(ID3D11Device* device, const D3D11BufferInfo &info,
    unsigned int miscFlags, unsigned int elementSize, const D3D11_SUBRESOURCE_DATA * initialData)
{
    m_uiSize = info.size;
    D3D11_BUFFER_DESC bd{};
    bd.Usage = info.usage;
    bd.ByteWidth = m_uiSize;
    bd.BindFlags = info.bindingFlags;
    bd.StructureByteStride = elementSize;
    bd.CPUAccessFlags = info.cpuAccessFlags;
    bd.MiscFlags = miscFlags;

    CALL_D3D_API(device->CreateBuffer(&bd, initialData, &m_pBuffer), TEXT("Failed to create d3d11 hardware buffer"));
}

RHEngine::D3D11Buffer::~D3D11Buffer()
{
    if (m_pBuffer) {
        m_pBuffer->Release();
        m_pBuffer = nullptr;
    }
}

void RHEngine::D3D11Buffer::Update(ID3D11DeviceContext* context, void * data, int size)
{
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    context->Map(m_pBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    memcpy(mappedResource.pData, data, size < 0 ? m_uiSize : size);
    context->Unmap(m_pBuffer, 0);
}

void RHEngine::D3D11Buffer::SetDebugName(const String &name)
{
    //g_pDebug->SetD3DName(m_pBuffer, name + "(D3D1XBuffer)");
}
