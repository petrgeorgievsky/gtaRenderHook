#pragma once
#include "D3D1XBuffer.h"
class CD3D1XVertexBuffer : public CD3D1XBuffer
{
public:
    CD3D1XVertexBuffer( unsigned int size, const D3D11_SUBRESOURCE_DATA *data );
    ~CD3D1XVertexBuffer();
};

