#pragma once
#include <RWUtils/RwAPI.h>
namespace RH_RWAPI
{

RpGeometry* _RpGeometryStreamRead( void* stream );

struct _rpTriangle
{
    RwUInt32            vertex01; /* V0 index in top 16 bits, V1 index in bottom 16 bits */
    RwUInt32            vertex2Mat; /* V2 index in top 16 bits, Material index in bottom 16 bit */
};

struct _rpMorphTarget
{
    RwSphere            boundingSphere;
    RwBool              pointsPresent;
    RwBool              normalsPresent;
};

}
