#pragma once
#include <Engine/Common/types/input_element_type.h>
#include <Engine/Common/types/topology_type.h>
#include <Engine/Common/types/vertex_bind_rate.h>
#include <common.h>

#include "Engine/Common/types/attachment_load_op.h"
#include "Engine/Common/types/attachment_store_op.h"
#include "Engine/Common/types/blend_op.h"
#include "Engine/Common/types/descriptor_type.h"
#include "Engine/Common/types/image_buffer_format.h"
#include "Engine/Common/types/image_layout.h"
#include "Engine/Common/types/memory_access_flags.h"
#include "Engine/Common/types/pipeline_bind_point.h"
#include "Engine/Common/types/pipeline_stages.h"
#include "Engine/Common/types/shader_stage.h"

namespace rh::engine
{

constexpr vk::ImageLayout Convert( ImageLayout rh_layout )
{
    switch ( rh_layout )
    {
    case ImageLayout::Undefined: return vk::ImageLayout::eUndefined;
    case ImageLayout::General: return vk::ImageLayout::eGeneral;
    case ImageLayout::ColorAttachment:
        return vk::ImageLayout::eColorAttachmentOptimal;
    case ImageLayout::DepthStencilAttachment:
        return vk::ImageLayout::eDepthStencilAttachmentOptimal;
    case ImageLayout::DepthStencilReadOnly:
        return vk::ImageLayout::eDepthStencilReadOnlyOptimal;
    case ImageLayout::ShaderReadOnly:
        return vk::ImageLayout::eShaderReadOnlyOptimal;
    case ImageLayout::TransferSrc: return vk::ImageLayout::eTransferSrcOptimal;
    case ImageLayout::TransferDst: return vk::ImageLayout::eTransferDstOptimal;
    case ImageLayout::Preinitialized: return vk::ImageLayout::ePreinitialized;
    case ImageLayout::DepthReadOnlyStencil:
        return vk::ImageLayout::eDepthReadOnlyStencilAttachmentOptimal;
    case ImageLayout::DepthAttachmentStencilReadOnly:
        return vk::ImageLayout::eDepthAttachmentStencilReadOnlyOptimal;
    case ImageLayout::PresentSrc: return vk::ImageLayout::ePresentSrcKHR;
    case ImageLayout::SharedPresent: return vk::ImageLayout::eSharedPresentKHR;
    case ImageLayout::ShadingRateOptimal:
        return vk::ImageLayout::eShadingRateOptimalNV;
    case ImageLayout::FragmentDensityMapOptimal:
        return vk::ImageLayout::eFragmentDensityMapOptimalEXT;
    }
    return vk::ImageLayout::eUndefined;
}

constexpr vk::Format Convert( ImageBufferFormat rh_format )
{
    switch ( rh_format )
    {
    case ImageBufferFormat::Unknown: return vk::Format::eUndefined;
    case ImageBufferFormat::BC1: return vk::Format::eBc1RgbaUnormBlock;
    case ImageBufferFormat::BC2: return vk::Format::eBc2UnormBlock;
    case ImageBufferFormat::BC3: return vk::Format::eBc3UnormBlock;
    case ImageBufferFormat::BC4: return vk::Format::eBc4UnormBlock;
    case ImageBufferFormat::BC5: return vk::Format::eBc5UnormBlock;
    case ImageBufferFormat::BC6H: return vk::Format::eBc6HUfloatBlock;
    case ImageBufferFormat::BC7: return vk::Format::eBc7UnormBlock;
    case ImageBufferFormat::RGBA32: return vk::Format::eR32G32B32A32Sfloat;
    case ImageBufferFormat::RGB32: return vk::Format::eR32G32B32Sfloat;
    case ImageBufferFormat::RGBA16: return vk::Format::eR16G16B16A16Sfloat;
    case ImageBufferFormat::RGB10A2: return vk::Format::eA2R10G10B10UnormPack32;
    case ImageBufferFormat::RG11B10: return vk::Format::eB10G11R11UfloatPack32;
    case ImageBufferFormat::RGBA8: return vk::Format::eR8G8B8A8Unorm;
    case ImageBufferFormat::RG32: return vk::Format::eR32G32Sfloat;
    case ImageBufferFormat::RG16: return vk::Format::eR16G16Sfloat;
    case ImageBufferFormat::RG8: return vk::Format::eR8G8Unorm;
    // may be incorrect
    case ImageBufferFormat::R32G8: return vk::Format::eR32G32Sfloat;
    case ImageBufferFormat::R32: return vk::Format::eR32Sfloat;
    case ImageBufferFormat::R16: return vk::Format::eR16Sfloat;
    case ImageBufferFormat::R8: return vk::Format::eR8Unorm;
    case ImageBufferFormat::R8Uint: return vk::Format::eR8Uint;
    case ImageBufferFormat::B5G6R5: return vk::Format::eB5G6R5UnormPack16;
    case ImageBufferFormat::BGR5A1: return vk::Format::eA1R5G5B5UnormPack16;
    case ImageBufferFormat::BGRA8: return vk::Format::eB8G8R8A8Unorm;
    case ImageBufferFormat::BGR8: return vk::Format::eB8G8R8Unorm;
    // may be incorrect
    case ImageBufferFormat::A8: return vk::Format::eR8Unorm;
    case ImageBufferFormat::BGRA4: return vk::Format::eB4G4R4A4UnormPack16;
    case ImageBufferFormat::D24S8: return vk::Format::eD32SfloatS8Uint;
    }
    return vk::Format::eUndefined;
}

constexpr vk::AttachmentLoadOp Convert( LoadOp rh_load_op )
{
    switch ( rh_load_op )
    {
    case LoadOp::Load: return vk::AttachmentLoadOp::eLoad;
    case LoadOp::Clear: return vk::AttachmentLoadOp::eClear;
    case LoadOp::DontCare: return vk::AttachmentLoadOp::eDontCare;
    }
    return vk::AttachmentLoadOp::eDontCare;
}

constexpr vk::AttachmentStoreOp Convert( StoreOp rh_store_op )
{
    switch ( rh_store_op )
    {
    case StoreOp::Store: return vk::AttachmentStoreOp::eStore;
    case StoreOp::DontCare: return vk::AttachmentStoreOp::eDontCare;
    }
    return vk::AttachmentStoreOp::eDontCare;
}

constexpr vk::PipelineBindPoint Convert( PipelineBindPoint rh_pipe_point )
{
    switch ( rh_pipe_point )
    {
    case PipelineBindPoint::Graphics: return vk::PipelineBindPoint::eGraphics;
    case PipelineBindPoint::Compute: return vk::PipelineBindPoint::eCompute;
    case PipelineBindPoint::RayTracing:
        return vk::PipelineBindPoint::eRayTracingNV;
    }
    return vk::PipelineBindPoint::eGraphics;
}

constexpr vk::DescriptorType Convert( DescriptorType type )
{
    switch ( type )
    {
    case rh::engine::DescriptorType::Sampler:
        return vk::DescriptorType::eSampler;
    case rh::engine::DescriptorType::ROBuffer:
        return vk::DescriptorType::eUniformBuffer;
    case rh::engine::DescriptorType::ROTexture:
        return vk::DescriptorType::eSampledImage;
    case rh::engine::DescriptorType::RWTexture:
        return vk::DescriptorType::eStorageTexelBuffer;
    case rh::engine::DescriptorType::RWBuffer:
        return vk::DescriptorType::eStorageBuffer;
    case DescriptorType::RTAccelerationStruct:
        return vk::DescriptorType::eAccelerationStructureNV;
    case DescriptorType::StorageTexture:
        return vk::DescriptorType::eStorageImage;
    }
    return {};
}

constexpr vk::PipelineStageFlagBits Convert( PipelineStage rh_pipe_stage )
{
    switch ( rh_pipe_stage )
    {
    case PipelineStage::PrePass: return vk::PipelineStageFlagBits::eTopOfPipe;
    case PipelineStage::DrawIndirect:
        return vk::PipelineStageFlagBits::eDrawIndirect;
    case PipelineStage::VertexInput:
        return vk::PipelineStageFlagBits::eVertexInput;
    case PipelineStage::VertexShader:
        return vk::PipelineStageFlagBits::eVertexShader;
    case PipelineStage::TessControl:
        return vk::PipelineStageFlagBits::eTessellationControlShader;
    case PipelineStage::TessEvaluation:
        return vk::PipelineStageFlagBits::eTessellationEvaluationShader;
    case PipelineStage::GeometryShader:
        return vk::PipelineStageFlagBits::eGeometryShader;
    case PipelineStage::PixelShader:
        return vk::PipelineStageFlagBits::eFragmentShader;
    case PipelineStage::EarlyFragmentTests:
        return vk::PipelineStageFlagBits::eEarlyFragmentTests;
    case PipelineStage::LateFragmentTests:
        return vk::PipelineStageFlagBits::eLateFragmentTests;
    case PipelineStage::ColorAttachmentOutput:
        return vk::PipelineStageFlagBits::eColorAttachmentOutput;
    case PipelineStage::ComputeShader:
        return vk::PipelineStageFlagBits::eComputeShader;
    case PipelineStage::Transfer: return vk::PipelineStageFlagBits::eTransfer;
    case PipelineStage::PostPass:
        return vk::PipelineStageFlagBits::eBottomOfPipe;
    case PipelineStage::Host: return vk::PipelineStageFlagBits::eHost;
    case PipelineStage::GraphicsPipelineEnd:
        return vk::PipelineStageFlagBits::eAllGraphics;
    case PipelineStage::AllPipelineEnd:
        return vk::PipelineStageFlagBits::eAllCommands;
    case PipelineStage::BuildAcceleration:
        return vk::PipelineStageFlagBits::eAccelerationStructureBuildNV;
    case PipelineStage::RayTracing:
        return vk::PipelineStageFlagBits::eRayTracingShaderNV;
    }
    return {};
}

constexpr vk::AccessFlagBits Convert( MemoryAccessFlags rh_pipe_stage )
{
    switch ( rh_pipe_stage )
    {
    case MemoryAccessFlags::Unknown: return {};
    case MemoryAccessFlags::IndirectCommandRead:
        return vk::AccessFlagBits::eIndirectCommandRead;
    case MemoryAccessFlags::IndexRead: return vk::AccessFlagBits::eIndexRead;
    case MemoryAccessFlags::VertexAttributeRead:
        return vk::AccessFlagBits::eVertexAttributeRead;
    case MemoryAccessFlags::UniformRead:
        return vk::AccessFlagBits::eUniformRead;
    case MemoryAccessFlags::InputAttachmentRead:
        return vk::AccessFlagBits::eInputAttachmentRead;
    case MemoryAccessFlags::ShaderRead: return vk::AccessFlagBits::eShaderRead;
    case MemoryAccessFlags::ShaderWrite:
        return vk::AccessFlagBits::eShaderWrite;
    case MemoryAccessFlags::ColorAttachmentRead:
        return vk::AccessFlagBits::eColorAttachmentRead;
    case MemoryAccessFlags::ColorAttachmentWrite:
        return vk::AccessFlagBits::eColorAttachmentWrite;
    case MemoryAccessFlags::DepthStencilAttachmentRead:
        return vk::AccessFlagBits::eDepthStencilAttachmentRead;
    case MemoryAccessFlags::DepthStencilAttachmentWrite:
        return vk::AccessFlagBits::eDepthStencilAttachmentWrite;
    case MemoryAccessFlags::TransferRead:
        return vk::AccessFlagBits::eTransferRead;
    case MemoryAccessFlags::TransferWrite:
        return vk::AccessFlagBits::eTransferWrite;
    case MemoryAccessFlags::HostRead: return vk::AccessFlagBits::eHostRead;
    case MemoryAccessFlags::HostWrite: return vk::AccessFlagBits::eHostWrite;
    case MemoryAccessFlags::MemoryRead: return vk::AccessFlagBits::eMemoryRead;
    case MemoryAccessFlags::MemoryWrite:
        return vk::AccessFlagBits::eMemoryWrite;
    case MemoryAccessFlags::AccelerationStructureRead:
        return vk::AccessFlagBits::eAccelerationStructureReadNV;
    case MemoryAccessFlags::AccelerationStructureWrite:
        return vk::AccessFlagBits::eAccelerationStructureWriteNV;
    }
    return {};
}

constexpr vk::ShaderStageFlagBits Convert( ShaderStage stage )
{
    switch ( stage )
    {
    case Compute: return vk::ShaderStageFlagBits::eCompute;
    case Geometry: return vk::ShaderStageFlagBits::eGeometry;
    case Pixel: return vk::ShaderStageFlagBits::eFragment;
    case Vertex: return vk::ShaderStageFlagBits::eVertex;
    case Domain: return vk::ShaderStageFlagBits::eTessellationControl;
    case Hull: return vk::ShaderStageFlagBits::eTessellationEvaluation;
    case RayGen: return vk::ShaderStageFlagBits::eRaygenNV;
    case RayMiss: return vk::ShaderStageFlagBits::eMissNV;
    case RayHit: return vk::ShaderStageFlagBits::eClosestHitNV;
    case RayAnyHit: return vk::ShaderStageFlagBits::eAnyHitNV;
    }
    return {};
}

constexpr vk::VertexInputRate Convert( VertexBindingRate rate )
{
    switch ( rate )
    {
    case VertexBindingRate::PerInstance: return vk::VertexInputRate::eInstance;
    case VertexBindingRate::PerVertex: return vk::VertexInputRate::eVertex;
    }
    return {};
}

constexpr vk::Format Convert( InputElementType el_type )
{
    switch ( el_type )
    {
    case InputElementType::Float: return vk::Format::eR32Sfloat;
    case InputElementType::Vec2fp16: return vk::Format::eR16G16Sfloat;
    case InputElementType::Vec2fp32: return vk::Format::eR32G32Sfloat;
    case InputElementType::Vec2fp8: return vk::Format::eR8G8Unorm;
    case InputElementType::Vec3fp32: return vk::Format::eR32G32B32Sfloat;
    case InputElementType::Vec4fp16: return vk::Format::eR16G16B16A16Sfloat;
    case InputElementType::Vec4fp32: return vk::Format::eR32G32B32A32Sfloat;
    case InputElementType::Vec4fp8: return vk::Format::eR8G8B8A8Unorm;
    case InputElementType::Uint32: return vk::Format::eR32Uint;
    case InputElementType::Unknown: return vk::Format::eUndefined;
    }
    return vk::Format::eUndefined;
}

constexpr vk::PrimitiveTopology Convert( Topology el_type )
{
    switch ( el_type )
    {
    case Topology::TriangleList: return vk::PrimitiveTopology::eTriangleList;
    case Topology::LineList: return vk::PrimitiveTopology::eLineList;
    case Topology::PointList: return vk::PrimitiveTopology::ePointList;
    }
    return {};
}

constexpr vk::BlendFactor Convert( BlendOp op )
{
    switch ( op )
    {
    case BlendOp::Zero: return vk::BlendFactor::eZero;
    case BlendOp::One: return vk::BlendFactor::eOne;
    case BlendOp::SrcColor: return vk::BlendFactor::eSrcColor;
    case BlendOp::InvSrcColor: return vk::BlendFactor::eOneMinusSrcColor;
    case BlendOp::SrcAlpha: return vk::BlendFactor::eSrcAlpha;
    case BlendOp::InvSrcAlpha: return vk::BlendFactor::eOneMinusSrcAlpha;
    case BlendOp::DestAlpha: return vk::BlendFactor::eDstAlpha;
    case BlendOp::InvDestAlpha: return vk::BlendFactor::eOneMinusDstAlpha;
    case BlendOp::DestColor: return vk::BlendFactor::eDstColor;
    case BlendOp::InvDestColor: return vk::BlendFactor::eOneMinusDstColor;
    case BlendOp::SrcAlphaSat: return vk::BlendFactor::eSrcAlphaSaturate;
    case BlendOp::BlendFactor:
        /// TODO: Somthing is wrong here - investigate(incorrect conversion)
        return vk::BlendFactor::eConstantColor;
    case BlendOp::Src1Color: return vk::BlendFactor::eSrc1Color;
    case BlendOp::InvSrc1Color: return vk::BlendFactor::eOneMinusSrc1Color;
    case BlendOp::Src1Alpha: return vk::BlendFactor::eSrc1Alpha;
    case BlendOp::InvSrc1Alpha: return vk::BlendFactor::eOneMinusSrc1Alpha;
    }
    return {};
}
} // namespace rh::engine
