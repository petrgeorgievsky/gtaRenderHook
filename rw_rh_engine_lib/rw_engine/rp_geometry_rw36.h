//
// Created by peter on 19.02.2021.
//

#pragma once
#include "i_rp_geometry.h"

namespace rh::rw::engine
{
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
} // namespace rh::rw::engine