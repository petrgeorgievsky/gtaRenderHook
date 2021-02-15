//
// Created by peter on 27.06.2020.
//

#include "CameraDescription.h"
#include "rendering_loop/DescriptorGenerator.h"
#include <Engine/Common/IDeviceState.h>
#include <render_driver/render_driver.h>
#include <rw_engine/system_funcs/rw_device_system_globals.h>
#include <scene_graph.h>

namespace rh::rw::engine
{
using namespace rh::engine;

CameraDescription::CameraDescription()
{

    auto &device = gRenderDriver->GetDeviceState();

    DescriptorGenerator descriptorGenerator{};
    descriptorGenerator.AddDescriptor(
        0, 0, 0, DescriptorType::ROBuffer, 1,
        ShaderStage::Vertex | ShaderStage::Pixel | ShaderStage::RayGen |
            ShaderStage::RayHit | ShaderStage::Compute );
    mCameraSetLayout = descriptorGenerator.FinalizeDescriptorSet( 0, 1 );

    mDescSetAlloc = descriptorGenerator.FinalizeAllocator();

    std::array tex_layout_array = { mCameraSetLayout };

    mCameraSet = mDescSetAlloc->AllocateDescriptorSets(
        { .mLayouts = tex_layout_array } )[0];

    // setup camera stuff
    mCameraBuffer =
        device.CreateBuffer( { .mSize  = sizeof( DirectX::XMFLOAT4X4 ) * 6,
                               .mUsage = BufferUsage::ConstantBuffer,
                               .mFlags = BufferFlags::Dynamic,
                               .mInitDataPtr = nullptr } );

    std::array<BufferUpdateInfo, 1> buff_ui = {
        { 0, sizeof( DirectX::XMFLOAT4X4 ) * 6, mCameraBuffer } };
    device.UpdateDescriptorSets( { .mSet            = mCameraSet,
                                   .mBinding        = 0,
                                   .mDescriptorType = DescriptorType::ROBuffer,
                                   .mBufferUpdateInfo = buff_ui } );
}

CameraDescription::~CameraDescription()
{
    delete mCameraSet;
    delete mCameraBuffer;
    delete mCameraSetLayout;
    delete mDescSetAlloc;
}

void CameraDescription::Update( FrameInfo *frame )
{
    mCameraBuffer->Update( &frame->mView, sizeof( DirectX::XMFLOAT4X4 ) * 6 );
}

} // namespace rh::rw::engine