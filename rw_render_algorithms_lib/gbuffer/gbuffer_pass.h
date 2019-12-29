#pragma once
#include <Engine/Common/IGPUResource.h>
#include <stdint.h>
#include <vector>
namespace rh::engine {
class IRenderingContext;
}; // namespace rh::engine

struct GBufferDesc
{
    std::vector<uint32_t> bufferFormats;
    uint32_t width, height;
};

class GBufferPass
{
public:
    GBufferPass( const GBufferDesc &desc );
    ~GBufferPass();

    void PrepareFrame( rh::engine::IRenderingContext *context );
    void EndFrame( rh::engine::IRenderingContext *context );
    rh::engine::IGPUResource *GetGBuffer( uint32_t id );
    void UpdateSize( uint32_t w, uint32_t h );

private:
    std::vector<rh::engine::GPUResourcePtr> mGBufferTextures;
    GBufferDesc mCurrentGBufferDesc;
};
