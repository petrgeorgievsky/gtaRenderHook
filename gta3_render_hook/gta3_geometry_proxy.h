#pragma once
#include <common_headers.h>
#include <rw_engine/rw_rh_pipeline.h>

struct RpGeometryGTA3
{
    RwObject object; /* Generic type */

    RwUInt32 flags; /* Geometry flags */

    RwUInt16 lockedSinceLastInst; /* What has been locked since we last instanced
- for re-instancing */
    RwInt16 refCount;             /* Reference count (for keeping track of atomics referencing
geometry) */

    RwInt32 numTriangles; /* Quantity of various things (polys, verts and morph
targets) */
    RwInt32 numVertices;
    RwInt32 numMorphTargets;
    RwInt32 numTexCoordSets;

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

static RpGeometryRw35 g_rw35GeometryProxy;
