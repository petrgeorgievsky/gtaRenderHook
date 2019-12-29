#pragma once
#include <RWUtils/RwAPI.h>

namespace RH_RWAPI
{

struct rpGeometryList
{
    RpGeometry** geometries;
    RwInt32             numGeoms;
};

rpGeometryList* _GeometryListStreamRead( void* stream, rpGeometryList* geomList );
}