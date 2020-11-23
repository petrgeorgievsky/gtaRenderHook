#pragma once
#include "D3D1XBuffer.h"
template <class T>
class CD3D1XConstantBuffer :
    public CD3D1XBuffer
{
public:
    CD3D1XConstantBuffer();
    ~CD3D1XConstantBuffer();
    void Update()
    {
        CD3D1XBuffer::Update( &data );
    }
    T    data{};
};

template<class T>
inline CD3D1XConstantBuffer<T>::CD3D1XConstantBuffer() : CD3D1XBuffer( sizeof( T ), D3D11_USAGE_DYNAMIC,
                                                                       D3D11_BIND_CONSTANT_BUFFER,
                                                                       D3D11_CPU_ACCESS_WRITE )
{

}

template<class T>
inline CD3D1XConstantBuffer<T>::~CD3D1XConstantBuffer()
{
}
