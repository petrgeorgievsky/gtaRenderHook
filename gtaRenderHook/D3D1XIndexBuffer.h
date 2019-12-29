#pragma once
#include "D3D1XBuffer.h"
class CD3D1XIndexBuffer :
    public CD3D1XBuffer
{
public:
    CD3D1XIndexBuffer( unsigned int indexCount, const D3D11_SUBRESOURCE_DATA *data );
    ~CD3D1XIndexBuffer();
};

