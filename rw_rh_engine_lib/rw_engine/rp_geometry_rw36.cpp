//
// Created by peter on 19.02.2021.
//

#include "rp_geometry_rw36.h"

namespace rh::rw::engine
{
void *RpGeometryRw36::GetResEntry()
{
    return static_cast<RpGeometry *>( m_pGeometryImpl )->repEntry;
}

RwResEntry *&RpGeometryRw36::GetResEntryRef()
{
    return static_cast<RpGeometry *>( m_pGeometryImpl )->repEntry;
}

int32_t RpGeometryRw36::GetVertexCount()
{
    return static_cast<RpGeometry *>( m_pGeometryImpl )->numVertices;
}

int32_t RpGeometryRw36::GetMorphTargetCount()
{
    return static_cast<RpGeometry *>( m_pGeometryImpl )->numMorphTargets;
}

RpMorphTarget *RpGeometryRw36::GetMorphTarget( uint32_t id )
{
    return &static_cast<RpGeometry *>( m_pGeometryImpl )->morphTarget[id];
}

RwTexCoords *RpGeometryRw36::GetTexCoordSetPtr( uint32_t id )
{
    return static_cast<RpGeometry *>( m_pGeometryImpl )->texCoords[id];
}

RwRGBA *RpGeometryRw36::GetVertexColorPtr()
{
    return static_cast<RpGeometry *>( m_pGeometryImpl )->preLitLum;
}

void RpGeometryRw36::Unlock()
{
    static_cast<RpGeometry *>( m_pGeometryImpl )->lockedSinceLastInst = 0;
}

int32_t RpGeometryRw36::GetTriangleCount()
{
    return static_cast<RpGeometry *>( m_pGeometryImpl )->numTriangles;
}

RpTriangle *RpGeometryRw36::GetTrianglePtr()
{
    return static_cast<RpGeometry *>( m_pGeometryImpl )->triangles;
}

uint32_t RpGeometryRw36::GetFlags()
{
    return static_cast<RpGeometry *>( m_pGeometryImpl )->flags;
}

RpMeshHeader *RpGeometryRw36::GetMeshHeader() const
{
    return static_cast<RpGeometry *>( m_pGeometryImpl )->mesh;
}
std::span<RpMesh> RpGeometryRw36::GetMeshList() const
{
    auto  header     = GetMeshHeader();
    auto *mesh_begin = reinterpret_cast<RpMesh *>( header + 1 );
    return std::span<RpMesh>( mesh_begin, mesh_begin + header->numMeshes );
}
} // namespace rh::rw::engine