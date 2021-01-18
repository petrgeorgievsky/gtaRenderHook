#pragma once
#include <common_headers.h>
#include <rw_engine/rw_rh_pipeline.h>

struct RpGeometryGTA3
{
    RwObject object; /* Generic type */

    uint32_t flags; /* Geometry flags */

    uint16_t
        lockedSinceLastInst; /* What has been locked since we last instanced
- for re-instancing */
    int16_t refCount;        /* Reference count (for keeping track of atomics
       referencing        geometry) */

    int32_t numTriangles; /* Quantity of various things (polys, verts and morph
targets) */
    int32_t numVertices;
    int32_t numMorphTargets;
    int32_t numTexCoordSets;

    RpMaterialList matList;

    RpTriangle *triangles; /* The triangles */

    RwRGBA *preLitLum; /* The pre-lighting values */

    RwTexCoords *texCoords[rwMAXTEXTURECOORDS]; /* Texture coordinates */

    float somestuff[3];

    RpMeshHeader *mesh; /* The mesh - groups polys of the same material */

    RwResEntry *repEntry; /* Information for an instance */

    RpMorphTarget *morphTarget; /* The Morph Target */
};

class RpGeometryRw35 : public rh::rw::engine::RpGeometryInterface
{
  public:
    virtual ~RpGeometryRw35() override;
    void *            GetResEntry() override;
    RwResEntry *&     GetResEntryRef() override;
    int32_t           GetVertexCount() override;
    uint32_t          GetFlags() override;
    RpMeshHeader *    GetMeshHeader() const override;
    std::span<RpMesh> GetMeshList() const override;

    int32_t        GetMorphTargetCount() override;
    RpMorphTarget *GetMorphTarget( uint32_t id ) override;
    RwTexCoords *  GetTexCoordSetPtr( uint32_t id ) override;
    RwRGBA *       GetVertexColorPtr() override;
    void           Unlock() override;

    // RpGeometryInterface interface
  public:
    int32_t     GetTriangleCount() override;
    RpTriangle *GetTrianglePtr() override;
};
