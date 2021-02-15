//
// Created by peter on 24.04.2020.
//
#pragma once
#include "material_backend.h"
#include <Engine/ResourcePool.h>
#include <common.h>
#include <scene_graph.h>
#include <span>
#include <vector>

namespace rh
{
namespace engine
{
class IBuffer;
class IShader;
class IRenderPass;
class ICommandBuffer;
class IDescriptorSetLayout;
class IPipelineLayout;
class VulkanRayTracingPipeline;
class IDescriptorSet;
class IDescriptorSetAllocator;
class IImageBuffer;
class IImageView;
} // namespace engine

namespace rw::engine
{
struct GeometrySplit
{
    uint32_t mVertexOffset;
    uint32_t mIndexOffset;
    uint32_t mVertexCount;
    uint32_t mIndexCount;
};

struct GeometryMaterial
{
    int64_t mDiffuseRasterIdx;
    RwRGBA  mDiffuseColor;
    float   mSpecular;
};

class RefCountedBuffer
{
  public:
    RefCountedBuffer( rh::engine::IBuffer *b ) : mData( b ) { mRefCount = 1; }
    static bool          Release( RefCountedBuffer *buffer );
    void                 AddRef() { mRefCount++; }
    rh::engine::IBuffer *Get() { return mData; }

  private:
    rh::engine::IBuffer *mData;
    uint64_t             mRefCount = 0;
};

struct BackendMeshData
{
    RefCountedBuffer *            mIndexBuffer;
    RefCountedBuffer *            mVertexBuffer;
    uint64_t                      mIndexCount;
    uint64_t                      mVertexCount;
    std::vector<GeometrySplit>    mSplits;
    std::vector<GeometryMaterial> mMaterials;
    uint32_t                      mMaterialOffset;
};

struct VertexDescPosOnly
{
    float x, y, z, w;
};

struct VertexDescPosColor : VertexDescPosOnly
{
    uint8_t color[4];
};

struct VertexDescPosColorUV : VertexDescPosColor
{
    float u, v;
};

struct VertexDescPosColorUVNormals
{
    float    x, y, z, w;
    float    u, v, uw, uv;
    float    nx, ny, nz, nw;
    float    lmx = 0, lmy = 0, lmz = 0, lmw = 0;
    float    wx = 0, wy = 0, wz = 0, ww = 0;
    uint32_t bone_indices;
    uint8_t  color[4];
    uint32_t material_idx;
    uint32_t padd;
};

struct BackendMeshInitData
{
    uint64_t                      mIndexCount;
    uint64_t                      mVertexCount;
    uint16_t *                    mIndexData;
    VertexDescPosColorUVNormals * mVertexData;
    std::vector<GeometrySplit>    mSplits;
    std::vector<GeometryMaterial> mMaterials;
};

uint64_t CreateBackendMesh( const BackendMeshInitData &initData );
void     CreateBackendMeshImpl( void *memory );
void     DestroyBackendMesh( uint64_t id );

/// TODO: Reiterate on this
class RenderingPipeline
{
  public:
    virtual void Init( rh::engine::IRenderPass *render_pass ) = 0;

    virtual void Draw( rh::engine::ICommandBuffer *cmd_buff,
                       const BackendMeshData &     mesh ) = 0;
};
class RayTracingTestPipe;
class BackendRenderer
{
  public:
    BackendRenderer();

    void InitPipelines( FrameInfo *              frame,
                        rh::engine::IRenderPass *main_render_pass );

    void RegisterPipeline( RenderingPipeline *, uint64_t id );

    uint64_t Render( void *memory, rh::engine::ICommandBuffer *cmd_buffer );
    void     DispatchSomeRays( FrameInfo *                 pInfo,
                               rh::engine::ICommandBuffer *pBuffer );
    rh::engine::IImageView *GetRaytraceView();

  private:
    std::unordered_map<uint64_t, RenderingPipeline *> mPipelines;
    void *                                            mTlas = nullptr;
    void *              mTlasInstanceBuffer                 = nullptr;
    RayTracingTestPipe *mRTPipe                             = nullptr;
};

struct DrawCallInfo
{
    uint64_t            mDrawCallId;
    uint64_t            mMeshId;
    uint64_t            mPipelineId;
    uint64_t            mMaterialListStart;
    uint64_t            mMaterialListCount;
    DirectX::XMFLOAT4X3 mWorldTransform;
};

class BackendRendererClient
{
  public:
    BackendRendererClient();

    uint64_t                Serialize( MemoryWriter &memory_writer );
    void                    Flush();
    std::span<MaterialData> AllocateDrawCallMaterials( uint64_t count );
    void                    RecordDrawCall( const DrawCallInfo &info );

  private:
    std::vector<DrawCallInfo> MeshData;
    std::vector<MaterialData> MaterialsData;
    uint64_t                  DrawCallCount;
    uint64_t                  MaterialCount;
};

} // namespace rw::engine
} // namespace rh