#pragma once
#include <common_headers.h>
#include <iostream>
#include <unordered_map>
struct ItemDefenitionEntry
{
    std::string dff_name;
    RpClump *dff_model;
    uint32_t blas_inst_id;
    float draw_dist;
};

struct ItemPlacementEntry
{
    DirectX::XMFLOAT4X4 world_transform;
    uint32_t item_defenition_id;
    float current_dist_to_cam;
    uint32_t padd[2];
};
namespace rw_raytracing_lib {
class RayTracingScene;
class RayTracer;
} // namespace rw_raytracing_lib

class GTAMapLoader
{
public:
    void LoadIDE( const std::string &path );
    std::vector<uint32_t> LoadIPL( const std::string &path, uint8_t version );
    std::vector<uint32_t> LoadBinaryIPL( const std::string &path );
    std::vector<RpClump *> LoadModels( const std::string &path, std::vector<uint32_t> ids );
    void BuildScene( rw_raytracing_lib::RayTracingScene *rt_scene,
                     rw_raytracing_lib::RayTracer *ray_tracer,
                     const DirectX::XMFLOAT4 &camPos );
    void RenderScene( void *pipeline, void *per_obj_cb );

private:
    std::unordered_map<uint32_t, ItemDefenitionEntry> mIDEntries;
    std::vector<ItemPlacementEntry> mIPLEntries;
};
