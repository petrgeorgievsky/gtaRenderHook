#pragma once
#include "D3D1XBuffer.h"
class CD3D1XDynamicIndexBuffer :
    public CD3D1XBuffer
{
public:
    CD3D1XDynamicIndexBuffer( unsigned int maxIndexCount );
    ~CD3D1XDynamicIndexBuffer();
};

