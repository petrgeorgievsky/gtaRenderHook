#pragma once
#include <unordered_map>
namespace rh::engine {
class IGPUResource;
}
namespace rw_raytracing_lib {

struct TextureAddress
{
    uint32_t id;
    uint8_t mip_level;
    uint8_t scaleX;
    uint8_t scaleY;
    uint8_t padd;
};

class RayTracingTextureCache
{
public:
    RayTracingTextureCache();

    bool PushTexture( void *ptr );
    TextureAddress *GetTextureAddress( void *ptr );
    void *GetPoolPtr() { return m_pTexturePool; }

private:
    std::unordered_map<void *, TextureAddress> m_pTextureAddressCache;
    uint32_t mTextureIdCounters[9];
    rh::engine::IGPUResource *m_pTexturePool;
};
} // namespace rw_raytracing_lib
