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

struct SkinDrawCallInfo
{
    uint64_t            mMeshId;
    uint64_t            mSkinId;
    uint64_t            mMaterialListStart;
    uint64_t            mMaterialListCount;
    DirectX::XMFLOAT4X3 mWorldTransform;
    DirectX::XMFLOAT4X3 mBoneTransform[256];
};

uint64_t CreateSkinMesh( const SkinnedMeshInitData &initData );
void     CreateSkinMeshImpl( void *memory );
void     DestroySkinMesh( uint64_t id );
void     DestroySkinMeshImpl( void *memory );

class SkinRendererClient
{
  public:
    SkinRendererClient();

    uint64_t Serialize( MemoryWriter &writer );
    void     Flush();

    std::span<MaterialData> AllocateDrawCallMaterials( uint64_t count );
    void                    RecordDrawCall( const SkinDrawCallInfo &info );

  private:
    std::vector<MaterialData>     MaterialsData;
    std::vector<SkinDrawCallInfo> MeshData;
    uint64_t                      DrawCallCount;
    uint64_t                      MaterialCount;
};

} // namespace rh::rw::engine
