#pragma once
#include <cstdint>
struct RpGeometry;
struct RwSphere;

struct rpGeometryList
{
    RpGeometry **geometries;
    int32_t numGeoms;
};
namespace rh::rw::engine {

struct _rpTriangle
{
    uint32_t vertex01;   /* V0 index in top 16 bits, V1 index in bottom 16 bits */
    uint32_t vertex2Mat; /* V2 index in top 16 bits, Material index in bottom 16 bit */
};

rpGeometryList *GeometryListStreamRead( void *stream, rpGeometryList *geomList );
rpGeometryList *GeometryListDeinitialize( rpGeometryList *geomList );
RpGeometry *RpGeometryStreamRead( void *stream );
RpGeometry *RpGeometryCreate( int32_t numVerts, int32_t numTriangles, uint32_t format );
int32_t RpGeometryDestroy( RpGeometry *geometry );
RpGeometry *RpGeometryAddRef( RpGeometry *geometry );

int32_t RpGeometryAddMorphTargets( RpGeometry *geometry, int32_t mtcount ) noexcept;

int32_t RpGeometryAddMorphTarget( RpGeometry *geometry ) noexcept;
} // namespace rw_rh_engine
