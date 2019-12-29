#pragma once
#include <cstdint>
namespace rh::engine {
class IRenderingContext;
class IRenderingPipeline;
} // namespace rh::engine
struct RpAtomic;
struct RpMeshHeader;
struct RwResEntry;
struct RpMorphTarget;
struct RwTexCoords;
struct RwRGBA;
struct RpTriangle;
namespace rh::rw::engine {

enum RenderStatus { Failure, NotInstanced, Instanced };

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

struct VertexDescPosColorUVNormals : VertexDescPosColorUV
{
    float nx, ny, nz;
};

class RpGeometryInterface
{
public:
    virtual ~RpGeometryInterface() = default;
    void Init( void *geometry );
    virtual void *GetResEntry() = 0;
    virtual RwResEntry *&GetResEntryRef() = 0;
    virtual int32_t GetVertexCount() = 0;
    virtual int32_t GetTriangleCount() = 0;
    virtual RpTriangle *GetTrianglePtr() = 0;
    virtual uint32_t GetFlags() = 0;
    virtual int32_t GetMorphTargetCount() = 0;
    virtual RpMeshHeader *GetMeshHeader() = 0;
    virtual RpMorphTarget *GetMorphTarget( uint32_t id ) = 0;
    virtual RwTexCoords *GetTexCoordSetPtr( uint32_t id ) = 0;
    virtual RwRGBA *GetVertexColorPtr() = 0;
    virtual void Unlock() = 0;

protected:
    void *m_pGeometryImpl = nullptr;
};

class RpGeometryRw36 : public RpGeometryInterface
{
public:
    virtual ~RpGeometryRw36() override {}
    void *GetResEntry() override;
    RwResEntry *&GetResEntryRef() override;
    int32_t GetVertexCount() override;
    uint32_t GetFlags() override;
    RpMeshHeader *GetMeshHeader() override;

    int32_t GetMorphTargetCount() override;
    RpMorphTarget *GetMorphTarget( uint32_t id ) override;
    RwTexCoords *GetTexCoordSetPtr( uint32_t id ) override;
    RwRGBA *GetVertexColorPtr() override;
    void Unlock() override;

    // RpGeometryInterface interface
public:
    int32_t GetTriangleCount() override;
    RpTriangle *GetTrianglePtr() override;
};

RenderStatus RwRHInstanceAtomic( RpAtomic *atomic, RpGeometryInterface *geom_io );

void MeshGetNumVerticesMinIndex( const uint16_t *indices,
                                 uint32_t size,
                                 uint32_t &numVertices,
                                 uint32_t &min );

void DrawAtomic( RpAtomic *atomic,
                 RpGeometryInterface *geom_io,
                 rh::engine::IRenderingContext *context,
                 rh::engine::IRenderingPipeline *pipeline );

} // namespace rw_rh_engine
