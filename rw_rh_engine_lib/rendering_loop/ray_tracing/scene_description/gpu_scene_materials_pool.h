//
// Created by peter on 24.10.2020.
//
#pragma once
#include <rw_engine/rh_backend/material_backend.h>
#include <rw_engine/rh_backend/mesh_rendering_backend.h>
#include <vector>
namespace rh
{
namespace engine
{
class IDescriptorSet;
class IImageView;
} // namespace engine
namespace rw::engine
{

struct MaterialUpdateData
{
    RefCountedBuffer *mIndexRemapBuffer;
    MaterialData *    mMaterials;
    uint32_t          mMaterialCount;
};

class GPUSceneMaterialsPool
{
  public:
    GPUSceneMaterialsPool( rh::engine::IDescriptorSet *desc_set,
                           uint64_t                    max_buffer_count,
                           uint32_t                    materials_binding,
                           uint32_t                    index_remap_binding );

    void StoreMaterialData( MaterialUpdateData &model, uint64_t instance_id );
    void ResetFrame();

  private:
    rh::engine::IDescriptorSet *       mGPUPool;
    std::vector<rh::engine::IBuffer *> mMaterialBuffers{};
    uint32_t                           mMaterialsBinding;
    uint32_t                           mIndexRemapBinding;
};
} // namespace rw::engine
} // namespace rh
