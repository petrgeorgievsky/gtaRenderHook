// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "stdafx.h"
#include "D3D1XVertexBuffer.h"


CD3D1XVertexBuffer::CD3D1XVertexBuffer(unsigned int size, const D3D11_SUBRESOURCE_DATA *data):
	CD3D1XBuffer::CD3D1XBuffer(size, D3D11_USAGE_IMMUTABLE, D3D11_BIND_VERTEX_BUFFER, 0, 0, 0, data)
{
	
}


CD3D1XVertexBuffer::~CD3D1XVertexBuffer()
{
}
