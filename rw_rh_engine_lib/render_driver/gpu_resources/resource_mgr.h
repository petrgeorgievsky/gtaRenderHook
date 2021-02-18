//
// Created by peter on 17.02.2021.
//

#pragma once
#include <Engine/ResourcePool.h>
#include <render_driver/gpu_resources/raster_pool.h>
#include <rw_engine/rh_backend/mesh_rendering_backend.h>
#include <rw_engine/rh_backend/skinned_mesh_backend.h>

namespace rh::engine
{
class IDeviceState;
class IWindow;
class IImageBuffer;
class IImageView;
} // namespace rh::engine

namespace rh::rw::engine
{
class EngineResourceHolder
{
  public:
    EngineResourceHolder();

    void GC();

    rh::engine::ResourcePool<RasterData> &GetRasterPool() { return RasterPool; }
    rh::engine::ResourcePool<SkinMeshData> &GetSkinMeshPool()
    {
        return SkinMeshPool;
    }
    rh::engine::ResourcePool<BackendMeshData> &GetMeshPool()
    {
        return MeshPool;
    }

  private:
    // Resources
    rh::engine::ResourcePool<RasterData>      RasterPool;
    rh::engine::ResourcePool<SkinMeshData>    SkinMeshPool;
    rh::engine::ResourcePool<BackendMeshData> MeshPool;
};
} // namespace rh::rw::engine