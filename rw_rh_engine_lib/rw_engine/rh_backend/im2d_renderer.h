//
// Created by peter on 25.06.2020.
//
#pragma once
#include <Engine/Common/IShader.h>
#include <Engine/Common/ScopedPtr.h>
#include <Engine/ResourcePool.h>
#include <render_client/im2d_state_recorder.h>
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

struct ShaderInfo
{
    rh::engine::ShaderDesc desc;
    rh::engine::IShader *  shader;
};
struct RasterData;
using RasterPoolType = rh::engine::ResourcePool<RasterData>;
class CameraDescription;
class Im2DRenderer
{
  public:
    Im2DRenderer( rh::engine::IDeviceState &device, RasterPoolType &raster_pool,
                  CameraDescription *      cdsec,
                  rh::engine::IRenderPass *render_pass );
    ~Im2DRenderer();

    rh::engine::IPipeline *GetCachedPipeline( uint64_t hash );

    uint64_t Render( const Im2DRenderState &     state,
                     rh::engine::ICommandBuffer *cmd_buffer );
    void     DrawQuad( rh::engine::IImageView *    texture,
                       rh::engine::ICommandBuffer *cmd_buffer );
    void     DrawDepthMask( rh::engine::IImageView *    texture,
                            rh::engine::ICommandBuffer *cmd_buffer );
    void     DrawQuad( uint64_t                    texture_id,
                       rh::engine::ICommandBuffer *cmd_buffer );
    void     Reset();

  private:
    rh::engine::IDescriptorSet *      GetRasterDescSet( uint64_t id );
    rh::engine::IDeviceState &        Device;
    RasterPoolType &                  RasterPool;
    CameraDescription *               mCamDesc;
    rh::engine::IDescriptorSetLayout *mTextureDescSetLayout;
    rh::engine::IDescriptorSetLayout *mGlobalSetLayout;
    ShaderInfo                        mBaseVertex{};
    ShaderInfo                        mNoTexPixel{};
    ShaderInfo                        mTexPixel{};
    ShaderInfo                        mDepthMaskPixel{};
    rh::engine::IPipelineLayout *     mTexLayout;
    rh::engine::IPipelineLayout *     mNoTexLayout;
    rh::engine::IPipeline *           mPipelineTex;
    rh::engine::IPipeline *           mPipelineNoTex;
    // TODO: Maybe move out of here
    rh::engine::IPipeline *mDepthMaskPipeline;

    using PipeHashTable =
        std::unordered_map<uint64_t,
                           rh::engine::ScopedPointer<rh::engine::IPipeline>>;

    PipeHashTable mIm2DPipelines;

    rh::engine::IDescriptorSetAllocator *     mDescSetAllocator;
    rh::engine::IDescriptorSet *              mBaseDescSet;
    rh::engine::IBuffer *                     mVertexBuffer;
    rh::engine::IBuffer *                     mIndexBuffer;
    rh::engine::IRenderPass *                 mRenderPass;
    std::vector<rh::engine::IDescriptorSet *> mDescriptorSetPool;
    uint64_t                                  mDescriptorSetPoolId = 0;
    std::unordered_map<uint64_t, rh::engine::IDescriptorSet *> mTextureCache;
    uint64_t              mVertexBufferOffset = 0;
    rh::engine::ISampler *mTextureSampler;
    rh::engine::IBuffer * mGlobalsBuffer;
};
} // namespace rh::rw::engine
