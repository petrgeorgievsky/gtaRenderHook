#pragma once
#include "gta_sa_ptrs.h"
template <typename T> class CRenderMeshPool
{
  public:
    CRenderMeshPool( size_t capacity = 1000 ) : mCapacity( capacity )
    {
        mData.resize( mCapacity );
    }

    void ExecuteForEach( std::function<void( const T & )> f )
    {
        for ( size_t i = 0; i < mSize; i++ )
            f( mData[i] );
    }
    void Push( T &&mesh )
    {
        if ( mSize + 1 > mCapacity )
        {
            mCapacity *= 2;
            mData.resize( mCapacity );
        }

        mData[mSize++] = mesh;
    }
    void Clean() { mSize = 0; }

  private:
    std::vector<T> mData{};
    size_t         mCapacity = 1000;
    size_t         mSize     = 0;
};
