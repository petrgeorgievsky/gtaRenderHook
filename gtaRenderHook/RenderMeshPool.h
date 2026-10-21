#pragma once
#include "gta_sa_ptrs.h"
template <typename T> class CRenderMeshPool
{
  public:
    CRenderMeshPool( size_t capacity = 1000 ) { mData.resize( capacity ); }

    void ExecuteForEach( std::function<void( const T & )> f )
    {
        for ( size_t i = 0; i < mSize; i++ )
            f( mData[i] );
    }
    void Push( T &&mesh )
    {
        // Grow from the vector's own size, so the bound and the storage can never disagree.
        if ( mSize >= mData.size() )
            mData.resize( mData.empty() ? 1000 : mData.size() * 2 );

        mData[mSize++] = mesh;
    }
    void Clean() { mSize = 0; }

  private:
    std::vector<T> mData{};
    size_t         mSize = 0;
};
