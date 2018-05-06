// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "stdafx.h"
#include "D3D1XIndexBuffer.h"


CD3D1XIndexBuffer::CD3D1XIndexBuffer(unsigned int indexCount, const D3D11_SUBRESOURCE_DATA *data):
	CD3D1XBuffer::CD3D1XBuffer(indexCount*sizeof(RxVertexIndex), D3D11_USAGE_IMMUTABLE, D3D11_BIND_INDEX_BUFFER, 0, 0, 0, data)
{
}


CD3D1XIndexBuffer::~CD3D1XIndexBuffer()
{
}
