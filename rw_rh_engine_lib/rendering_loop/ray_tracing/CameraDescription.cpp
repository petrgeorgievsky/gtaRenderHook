//
// Created by peter on 27.06.2020.
//

#include "CameraDescription.h"
#include "rendering_loop/DescriptorGenerator.h"
#include <Engine/Common/IDeviceState.h>
#include <data_desc/frame_info.h>

namespace rh::rw::engine
{
using namespace rh::engine;

uint64_t CameraDescription::Id = UninitializedId;

CameraDescription::CameraDescription( const RendererBase &renderer )
    : Device( renderer.Device )
{
    DescriptorGenerator descriptorGenerator{ Device };
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
        Device.CreateBuffer( { .mSize        = sizeof( CameraState ),
                               .mUsage       = BufferUsage::ConstantBuffer,
                               .mFlags       = BufferFlags::Dynamic,
                               .mInitDataPtr = nullptr } );

    std::array<BufferUpdateInfo, 1> buff_ui = {
        { { 0, sizeof( CameraState ), mCameraBuffer } } };
    Device.UpdateDescriptorSets( { .mSet            = mCameraSet,
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

void CameraDescription::Update( const FrameState &state )
{
    mCameraBuffer->Update( &state.Viewport->Camera, sizeof( CameraState ) );
}

} // namespace rh::rw::engine