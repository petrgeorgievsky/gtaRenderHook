//
// Created by peter on 17.02.2021.
//

#include "resource_mgr.h"
namespace rh::rw::engine
{

EngineResourceHolder::EngineResourceHolder()
    : RasterPool( 11000, []( RasterData &data, uint64_t ) { data.Release(); } ),
      SkinMeshPool( 1000,
                    []( SkinMeshData &data, uint64_t id ) {
                        if ( data.mIndexBuffer &&
                             RefCountedBuffer::Release( data.mIndexBuffer ) )
                            delete data.mIndexBuffer;
                        if ( data.mVertexBuffer &&
                             RefCountedBuffer::Release( data.mVertexBuffer ) )
                            delete data.mVertexBuffer;
                    } ),
      MeshPool( 10000, []( BackendMeshData &obj, uint64_t id ) {
          if ( obj.mIndexBuffer &&
               RefCountedBuffer::Release( obj.mIndexBuffer ) )
              delete obj.mIndexBuffer;
          if ( obj.mVertexBuffer &&
               RefCountedBuffer::Release( obj.mVertexBuffer ) )
              delete obj.mVertexBuffer;
      } )
{
}

void EngineResourceHolder::GC()
{
    SkinMeshPool.CollectGarbage( 1000 );
    MeshPool.CollectGarbage( 120 );
    RasterPool.CollectGarbage( 100 );
}
} // namespace rh::rw::engine