//
// Created by peter on 04.05.2020.
//
#pragma once
#include <Engine/Common/IShader.h>
#include <array>
#include <cstdint>
#include <vector>

namespace rh::engine
{
class IRenderPass;
class IPipeline;
class IDescriptorSetLayout;
class IPipelineLayout;
class IImageBuffer;
class IImageView;
class IFrameBuffer;
class ICommandBuffer;
} // namespace rh::engine
namespace rh::rw::engine
{
struct GBufferDesc
{
    std::vector<uint32_t> bufferFormats;
    uint32_t              width, height;
};
class GBufferPass
{
  public:
    void InitializeFrameBuffer();
    void InitializeRenderPass();
    void InitializePipeline();

    void RenderMesh();

  private:
    std::array<rh::engine::IImageBuffer *, 2> mFramebufferImages;
    std::array<rh::engine::IImageView *, 2>   mFramebufferImageViews;
    rh::engine::IFrameBuffer *                mFramebuffer;
    rh::engine::IRenderPass *                 mRenderPass;
    rh::engine::ShaderDesc                    mVsDesc;
    rh::engine::IShader *                     mVertexShader;
    rh::engine::ShaderDesc                    mPsDesc;
    rh::engine::IShader *                     mPixelShader;
    rh::engine::IPipeline *                   mPipeline;
    rh::engine::IDescriptorSetLayout *        mCameraSetLayout;
    rh::engine::IDescriptorSetLayout *        mModelSetLayout;
    rh::engine::IPipelineLayout *             mPipeLayout;
    rh::engine::ICommandBuffer *              mCommandBuffer;
};
} // namespace rh::rw::engine
