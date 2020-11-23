#pragma once
#include "ArrayProxy.h"
#include "types/input_element_type.h"
#include "types/shader_stage.h"
#include "types/vertex_bind_rate.h"
#include <Engine/Common/types/blend_state.h>
#include <Engine/Common/types/depth_stencil_state.h>
#include <Engine/Common/types/topology_type.h>
#include <string>
#include <vector>

namespace rh::engine
{
class IRenderPass;
class IPipelineLayout;
class IShader;

struct ShaderStageDesc
{
    ShaderStage mStage;
    IShader *   mShader;
    std::string mEntryPoint;
};

struct VertexInputElementDesc
{
    uint32_t         mBinding;
    uint32_t         mLocation;
    InputElementType mFormat;
    uint32_t         mOffset;
    std::string      mSemantics;
    uint32_t         mSemanticsId;
};

struct VertexBindingDesc
{
    uint32_t          mBinding;
    uint32_t          mStride;
    VertexBindingRate mInputRate;
};

struct VertexInputStateDesc
{
    ArrayProxy<VertexBindingDesc>      mVertexBindingLayout;
    ArrayProxy<VertexInputElementDesc> mVertexInputLayout;
};

struct RasterPipelineCreateParams
{
    IRenderPass *                mRenderPass;
    IPipelineLayout *            mLayout;
    std::vector<ShaderStageDesc> mShaderStages;
    VertexInputStateDesc         mVertexInputStateDesc;
    Topology                     mTopology;
    BlendState                   mBlendState;
    DepthStencilState            mDepthStencilState;
};
class IPipeline
{
  public:
    virtual ~IPipeline() = default;
};
} // namespace rh::engine