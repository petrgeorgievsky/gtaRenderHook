//
// Created by peter on 27.06.2020.
//
#pragma once
#include <Engine/Common/ScopedPtr.h>
#include <Engine/ResourcePool.h>
#include <array>
#include <common.h>
#include <rw_engine/rh_backend/mesh_rendering_backend.h>

namespace rh::engine
{
class IBuffer;
class IDescriptorSetAllocator;
class IDescriptorSetLayout;
class IDescriptorSet;
} // namespace rh::engine
namespace rh::rw::engine
{
using rh::engine::ScopedPointer;
struct SceneObjDesc
{
    uint32_t            objId;
    uint32_t            txtOffset;
    uint32_t            align_a;
    uint32_t            align_b;
    DirectX::XMFLOAT4X4 transform;
    DirectX::XMFLOAT4X4 transfomIT;
    DirectX::XMFLOAT4X4 prevTransfom;
};

class GPUTexturePool;
class GPUModelBuffersPool;
class GPUSceneMaterialsPool;

class RTSceneDescription
{
  public:
    RTSceneDescription();
    virtual ~RTSceneDescription();

    rh::engine::IDescriptorSetLayout *DescLayout();
    rh::engine::IDescriptorSet *      DescSet();

    void RecordDrawCall( const DrawCallInfo &dc, const MaterialData *materials,
                         uint64_t material_count );
    void Update();

  private:
    std::vector<SceneObjDesc>                          mSceneDesc;
    std::vector<MaterialData>                          mSceneMaterials;
    uint64_t                                           mDrawCalls = 0;
    uint64_t                                           mMaterials = 0;
    ScopedPointer<rh::engine::IDescriptorSetAllocator> mDescSetAlloc;
    ScopedPointer<rh::engine::IDescriptorSetLayout>    mSceneSetLayout;
    ScopedPointer<rh::engine::IDescriptorSet>          mSceneSet;
    ScopedPointer<rh::engine::IBuffer>                 mSceneDescBuffer;
    ScopedPointer<rh::engine::IBuffer>                 mMaterialDescBuffer;
    ScopedPointer<GPUTexturePool>                      mTexturePool;
    ScopedPointer<GPUModelBuffersPool>                 mModelBuffersPool;
    ScopedPointer<GPUSceneMaterialsPool>               mSceneMaterialsPool;
    std::unordered_map<uint64_t, DirectX::XMFLOAT4X4>  mPrevTransformMap;
};

} // namespace rh::rw::engine