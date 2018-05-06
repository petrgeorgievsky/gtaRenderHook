// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "stdafx.h"
#include "D3D1XDynamicIndexBuffer.h"


CD3D1XDynamicIndexBuffer::CD3D1XDynamicIndexBuffer(unsigned int maxIndexCount):
	CD3D1XBuffer(maxIndexCount*sizeof(RwImVertexIndex), D3D11_USAGE_DYNAMIC, D3D11_BIND_INDEX_BUFFER, D3D11_CPU_ACCESS_WRITE)
{
}


CD3D1XDynamicIndexBuffer::~CD3D1XDynamicIndexBuffer()
{
}
