// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "stdafx.h"
#include "D3D1XDynamicVertexBuffer.h"


CD3D1XDynamicVertexBuffer::CD3D1XDynamicVertexBuffer(unsigned int vertexSize, unsigned int maxVertexCount):
	CD3D1XBuffer(vertexSize*maxVertexCount, D3D11_USAGE_DYNAMIC, D3D11_BIND_VERTEX_BUFFER,D3D11_CPU_ACCESS_WRITE)
{
}


CD3D1XDynamicVertexBuffer::~CD3D1XDynamicVertexBuffer()
{
}
