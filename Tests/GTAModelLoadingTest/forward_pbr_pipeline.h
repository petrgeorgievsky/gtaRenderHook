#pragma once
#include <Engine/Common/IShader.h>
#include <Engine/Common/irenderingpipeline.h>
#include <rw_engine/rh_backend/mesh_rendering_backend.h>

namespace rh::engine
{
class IRenderingContext;
class D3D11ConstantBuffer;
class IPipelineLayout;
class IDescriptorSetLayout;
class IPipeline;
class IDescriptorSetAllocator;
class IDescriptorSet;
} // namespace rh::engine

struct MaterialCB
{
    float MaterialColor[4] = { 1, 1, 1, 1 };
    float Roughness        = 1.0f;
    float Metallness       = 0;
    float IOR{};
    float Emmisivness{};
};

struct ShaderInfo
{
    rh::engine::ShaderDesc desc;
    rh::engine::IShader *  shader{};
};

class ForwardPBRPipeline : public rh::rw::engine::RenderingPipeline
{
  public:
    ForwardPBRPipeline();
    ~ForwardPBRPipeline();

    void Init( rh::engine::IRenderPass *render_pass ) override;

    void Draw( rh::engine::ICommandBuffer *           cmd_buff,
               const rh::rw::engine::BackendMeshData &mesh ) override;

  private:
    rh::engine::IDescriptorSetLayout *        mCameraSetLayout;
    rh::engine::IDescriptorSetLayout *        mModelSetLayout;
    ShaderInfo                                mBaseVertex{};
    ShaderInfo                                mBasePixel{};
    rh::engine::IPipelineLayout *             mPipeLayout;
    rh::engine::IPipeline *                   mPipelineImpl;
    rh::engine::IDescriptorSetAllocator *     mDescSetAllocator;
    rh::engine::IDescriptorSet *              mCameraDescSet;
    std::vector<rh::engine::IDescriptorSet *> mModelDescSet;
    bool                                      mInitialized = false;
};
