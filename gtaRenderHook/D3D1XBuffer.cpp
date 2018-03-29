#include "stdafx.h"
#include "D3D1XBuffer.h"
#include "RwD3D1XEngine.h"
#include "D3DRenderer.h"

CD3D1XBuffer::CD3D1XBuffer(unsigned int size, D3D11_USAGE usage, D3D11_BIND_FLAG bindingFlags, D3D11_CPU_ACCESS_FLAG cpuAccessFlags, 
	unsigned int miscFlags, unsigned int elementSize)
{
	m_uiSize = size;
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = usage;
	bd.ByteWidth = m_uiSize;
	bd.BindFlags = bindingFlags;
	bd.StructureByteStride = elementSize;
	bd.CPUAccessFlags = cpuAccessFlags;
	bd.MiscFlags = miscFlags;
	if(FAILED(GET_D3D_DEVICE->CreateBuffer(&bd, nullptr, &m_pBuffer)))
		g_pDebug->printError("Failed to create d3d11 hardware buffer");
}


CD3D1XBuffer::~CD3D1XBuffer()
{
	if (m_pBuffer) {
		m_pBuffer->Release();
		m_pBuffer = nullptr;
	}
}

void CD3D1XBuffer::Update(void * data)
{
	auto context = GET_D3D_CONTEXT;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	context->Map(m_pBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	memcpy(mappedResource.pData, data, m_uiSize);
	context->Unmap(m_pBuffer, 0);
}
