#pragma once
#include "D3D1XBuffer.h"
class CD3D1XDynamicVertexBuffer :
    public CD3D1XBuffer
{
public:
    CD3D1XDynamicVertexBuffer( unsigned int vertexSize, unsigned int maxVertexCount );
    ~CD3D1XDynamicVertexBuffer();
};

