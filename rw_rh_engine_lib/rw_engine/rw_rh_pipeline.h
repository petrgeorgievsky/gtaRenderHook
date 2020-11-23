#pragma once
#include <common_headers.h>
#include <cstdint>
#include <functional>
#include <span>
#include <vector>

namespace rh::engine
{
class IRenderingContext;
class IRenderingPipeline;
class IBuffer;
} // namespace rh::engine
struct RpAtomic;
struct RpMeshHeader;
struct RwResEntry;
struct RpMorphTarget;
struct RwTexCoords;
struct RwRGBA;
struct RpTriangle;

struct MeshSplitData
{
    uint32_t numIndex{};

    uint32_t baseIndex{};

    uint32_t numVertices{};

    uint32_t startIndex{};

    // !TODO: REMOVE THIS
    void *material = nullptr;
};

struct RenderMeshData
{
    rh::engine::IBuffer *      mIndexBuffer;
    rh::engine::IBuffer *      mVertexBuffer;
    std::vector<MeshSplitData> mMaterialInfo;
};

namespace rh::rw::engine
{

struct ResEnty : RwResEntry
{
    uint64_t meshData;
    uint16_t batchId;
    uint16_t frameId;
};

enum RenderStatus
{
    Failure,
    NotInstanced,
    Instanced
};

class RpGeometryInterface
{
  public:
    virtual ~RpGeometryInterface() = default;
    void                      Init( void *geometry );
    virtual void *            GetResEntry()                    = 0;
    virtual RwResEntry *&     GetResEntryRef()                 = 0;
    virtual int32_t           GetVertexCount()                 = 0;
    virtual int32_t           GetTriangleCount()               = 0;
    virtual RpTriangle *      GetTrianglePtr()                 = 0;
    virtual uint32_t          GetFlags()                       = 0;
    virtual int32_t           GetMorphTargetCount()            = 0;
    virtual RpMeshHeader *    GetMeshHeader() const            = 0;
    virtual RpMorphTarget *   GetMorphTarget( uint32_t id )    = 0;
    virtual RwTexCoords *     GetTexCoordSetPtr( uint32_t id ) = 0;
    virtual RwRGBA *          GetVertexColorPtr()              = 0;
    virtual void              Unlock()                         = 0;
    virtual std::span<RpMesh> GetMeshList() const              = 0;
    virtual void *            GetThis() { return m_pGeometryImpl; }

  protected:
    void *m_pGeometryImpl = nullptr;
};

class RpGeometryRw36 : public RpGeometryInterface
{
  public:
    ~RpGeometryRw36() override {}
    void *        GetResEntry() override;
    RwResEntry *& GetResEntryRef() override;
    int32_t       GetVertexCount() override;
    uint32_t      GetFlags() override;
    RpMeshHeader *GetMeshHeader() const override;

    int32_t        GetMorphTargetCount() override;
    RpMorphTarget *GetMorphTarget( uint32_t id ) override;
    RwTexCoords *  GetTexCoordSetPtr( uint32_t id ) override;
    RwRGBA *       GetVertexColorPtr() override;
    void           Unlock() override;

    // RpGeometryInterface interface
  public:
    int32_t           GetTriangleCount() override;
    RpTriangle *      GetTrianglePtr() override;
    std::span<RpMesh> GetMeshList() const override;
};

RenderStatus RwRHInstanceAtomic( RpAtomic *           atomic,
                                 RpGeometryInterface *geom_io );

void MeshGetNumVerticesMinIndex( const uint16_t *indices, uint32_t size,
                                 uint32_t &numVertices, uint32_t &min );

void DrawAtomic( RpAtomic *atomic, RpGeometryInterface *geom_io,
                 const std::function<void( ResEnty *entry )> &render_callback );

} // namespace rh::rw::engine
