//
// Created by peter on 13.05.2020.
//
#pragma once
#include "mesh_rendering_backend.h"

namespace rh::rw::engine
{

struct SkinnedMeshInitData
{
    uint64_t                     mIndexCount;
    uint64_t                     mVertexCount;
    uint16_t *                   mIndexData;
    VertexDescPosColorUVNormals *mVertexData;
    std::vector<GeometrySplit>   mSplits;
};

struct SkinMeshData
{
    RefCountedBuffer *mIndexBuffer  = nullptr;
    RefCountedBuffer *mVertexBuffer = nullptr;
    uint64_t          mIndexCount;
    uint64_t          mVertexCount;
};

uint64_t CreateSkinMesh( const SkinnedMeshInitData &initData );
void     DestroySkinMesh( uint64_t id );

} // namespace rh::rw::engine
