//
// Created by peter on 24.04.2020.
//
#pragma once
#include "data_desc/light_system/packed_light.h"
#include "material_backend.h"
#include <Engine/ResourcePool.h>
#include <common.h>
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
    std::vector<PackedLight>      EmissiveTriangles;
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
    float    emissive = 0.0f;
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
void     DestroyBackendMesh( uint64_t id );

/// TODO: Reiterate on this
class RenderingPipeline
{
  public:
    virtual void Init( rh::engine::IRenderPass *render_pass ) = 0;

    virtual void Draw( rh::engine::ICommandBuffer *cmd_buff,
                       const BackendMeshData &     mesh ) = 0;
};

} // namespace rw::engine
} // namespace rh