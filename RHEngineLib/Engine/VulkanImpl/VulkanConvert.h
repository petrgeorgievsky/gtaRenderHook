#pragma once
#include <common.h>

#include "Engine/Common/types/attachment_load_op.h"
#include "Engine/Common/types/attachment_store_op.h"
#include "Engine/Common/types/image_buffer_format.h"
#include "Engine/Common/types/image_layout.h"
#include "Engine/Common/types/pipeline_bind_point.h"

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
    case ImageBufferFormat::B5G6R5: return vk::Format::eB5G6R5UnormPack16;
    case ImageBufferFormat::BGR5A1: return vk::Format::eB5G5R5A1UnormPack16;
    case ImageBufferFormat::BGRA8: return vk::Format::eB8G8R8A8Unorm;
    case ImageBufferFormat::BGR8: return vk::Format::eB8G8R8Unorm;
    // may be incorrect
    case ImageBufferFormat::A8: return vk::Format::eR8Unorm;
    case ImageBufferFormat::BGRA4: return vk::Format::eB4G4R4A4UnormPack16;
    }
}

constexpr vk::AttachmentLoadOp Convert( LoadOp rh_load_op )
{
    switch ( rh_load_op )
    {
    case LoadOp::Load: return vk::AttachmentLoadOp::eLoad;
    case LoadOp::Clear: return vk::AttachmentLoadOp::eClear;
    case LoadOp::DontCare: return vk::AttachmentLoadOp::eDontCare;
    }
}

constexpr vk::AttachmentStoreOp Convert( StoreOp rh_store_op )
{
    switch ( rh_store_op )
    {
    case StoreOp::Store: return vk::AttachmentStoreOp::eStore;
    case StoreOp::DontCare: return vk::AttachmentStoreOp::eDontCare;
    }
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
}

/*constexpr vk::SubpassDescription Convert( PipelineBindPoint rh_pipe_point )
{
    vk::SubpassDescription result{};

}*/
} // namespace rh::engine
