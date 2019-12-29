#pragma once

#include <RWUtils/RwAPI.h>

namespace RH_RWAPI
{

struct binMeshHeader
{
    RwUInt32            flags;
    RwUInt32            numMeshes;
    RwUInt32            totalIndicesInMesh;
};

struct binMesh
{
    RwUInt32            numIndices;
    RwInt32             matIndex;
};

RpMeshHeader* rpMeshRead( void* stream, const RpGeometry* geometry, const RpMaterialList* matList ) noexcept;

bool readGeometryMesh( void* stream, RpGeometry* geometry ) noexcept;


extern uint32_t g_nMeshSerialNum;
}