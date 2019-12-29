#pragma once
#include <RWUtils/RwAPI.h>

namespace RH_RWAPI {

constexpr RwInt32 GeometryFormatGetNumTexCoordSets( RwInt32 _fmt );

RpGeometry* _RpGeometryCreate( RwInt32 numVerts, RwInt32 numTriangles, RwUInt32 format );

static RwBool _GeometryAnnihilate( RpGeometry* geometry );

RwBool _RpGeometryDestroy( RpGeometry* geometry );

RwInt32 _RpGeometryAddMorphTargets( RpGeometry& geometry, RwInt32 mtcount ) noexcept;

RwInt32 _RpGeometryAddMorphTarget( RpGeometry& geometry ) noexcept;

RpGeometry* _RpGeometryAddRef( RpGeometry* geometry );

}