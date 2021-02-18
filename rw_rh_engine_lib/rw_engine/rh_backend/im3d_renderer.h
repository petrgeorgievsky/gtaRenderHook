//
// Created by peter on 21.11.2020.
//

#pragma once

#include <Engine/Common/IShader.h>
#include <Engine/Common/ScopedPtr.h>
#include <Engine/ResourcePool.h>
#include <unordered_map>
#include <vector>
namespace rh::engine
{
class IWindow;
class ISyncPrimitive;
class IImageBuffer;
class IImageView;
class IPipeline;
class IPipelineLayout;
class IDescriptorSet;
class IDescriptorSetAllocator;
class IBuffer;
class ICommandBuffer;
class IDescriptorSetLayout;
class IRenderPass;
class ISampler;
class IDeviceState;
} // namespace rh::engine
namespace rh::rw::engine
{
template <typename T> using SPtr = rh::engine::ScopedPointer<T>;
struct SInfo
{
    rh::engine::ShaderDesc    desc;
    SPtr<rh::engine::IShader> shader;
};

class CameraDescription;
struct Im3DRenderState;
struct RasterData;
using RasterPoolType = rh::engine::ResourcePool<RasterData>;
class Im3DRenderer
{
  public:
    Im3DRenderer( rh::engine::IDeviceState &device, RasterPoolType &raster_pool,
                  CameraDescription *      cdsec,
                  rh::engine::IRenderPass *render_pass );
    ~Im3DRenderer();

    rh::engine::IPipeline *GetCachedPipeline( uint64_t hash );

    uint64_t Render( const Im3DRenderState &     state,
                     rh::engine::ICommandBuffer *cmd_buffer );
    void     Reset();

  private:
    rh::engine::IDescriptorSet *           GetRasterDescSet( uint64_t id );
    rh::engine::IDeviceState &             Device;
    RasterPoolType &                       RasterPool;
    CameraDescription *                    mCamDesc;
    rh::engine::IRenderPass *              mRenderPass;
    SPtr<rh::engine::IDescriptorSetLayout> mTextureDescSetLayout;
    SPtr<rh::engine::IDescriptorSetLayout> mObjectSetLayout;
    SInfo                                  mBaseVertex{};
    SInfo                                  mNoTexPixel{};
    SInfo                                  mTexPixel{};
    SPtr<rh::engine::IPipelineLayout>      mTexLayout;
    SPtr<rh::engine::IPipelineLayout>      mNoTexLayout;

    using PipeHashTable =
        std::unordered_map<uint64_t,
                           rh::engine::ScopedPointer<rh::engine::IPipeline>>;

    PipeHashTable mIm3DPipelines;

    SPtr<rh::engine::IDescriptorSetAllocator> mDescSetAllocator;
    SPtr<rh::engine::IBuffer>                 mVertexBuffer;
    SPtr<rh::engine::IBuffer>                 mIndexBuffer;
    SPtr<rh::engine::IBuffer>                 mMatrixBuffer;
    std::vector<rh::engine::IDescriptorSet *> mDescriptorSetPool;
    std::vector<rh::engine::IDescriptorSet *> mMatrixDescriptorSetPool;
    uint64_t                                  mDescriptorSetPoolId = 0;
    uint64_t                                  mVertexBufferOffset  = 0;
    SPtr<rh::engine::ISampler>                mTextureSampler;
};
} // namespace rh::rw::engine
