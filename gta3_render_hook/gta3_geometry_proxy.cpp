#include "gta3_geometry_proxy.h"

RpGeometryRw35::~RpGeometryRw35() {}

void *RpGeometryRw35::GetResEntry()
{
    return static_cast<RpGeometryGTA3 *>( m_pGeometryImpl )->repEntry;
}

RwResEntry *&RpGeometryRw35::GetResEntryRef()
{
    return static_cast<RpGeometryGTA3 *>( m_pGeometryImpl )->repEntry;
}

int32_t RpGeometryRw35::GetVertexCount()
{
    return static_cast<RpGeometryGTA3 *>( m_pGeometryImpl )->numVertices;
}

uint32_t RpGeometryRw35::GetFlags()
{
    return static_cast<RpGeometryGTA3 *>( m_pGeometryImpl )->flags;
}

RpMeshHeader *RpGeometryRw35::GetMeshHeader()
{
    return static_cast<RpGeometryGTA3 *>( m_pGeometryImpl )->mesh;
}

int32_t RpGeometryRw35::GetMorphTargetCount()
{
    return static_cast<RpGeometryGTA3 *>( m_pGeometryImpl )->numMorphTargets;
}

RpMorphTarget *RpGeometryRw35::GetMorphTarget( uint32_t id )
{
    return &static_cast<RpGeometryGTA3 *>( m_pGeometryImpl )->morphTarget[id];
}

RwTexCoords *RpGeometryRw35::GetTexCoordSetPtr( uint32_t id )
{
    return static_cast<RpGeometryGTA3 *>( m_pGeometryImpl )->texCoords[id];
}

RwRGBA *RpGeometryRw35::GetVertexColorPtr()
{
    return static_cast<RpGeometryGTA3 *>( m_pGeometryImpl )->preLitLum;
}

void RpGeometryRw35::Unlock()
{
    static_cast<RpGeometryGTA3 *>( m_pGeometryImpl )->lockedSinceLastInst = 0;
}

int32_t RpGeometryRw35::GetTriangleCount()
{
    return static_cast<RpGeometryGTA3 *>( m_pGeometryImpl )->numTriangles;
}

RpTriangle *RpGeometryRw35::GetTrianglePtr()
{
    return static_cast<RpGeometryGTA3 *>( m_pGeometryImpl )->triangles;
}
