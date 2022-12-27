//
// Created by peter on 26.06.2020.
//
#pragma once
#include <Engine/Common/IDeviceState.h>
#include <vector>
namespace rh::engine
{
class VulkanCommandBuffer;
class IBuffer;
class VulkanTopLevelAccelerationStructure;
struct VkAccelerationStructureInstanceKHR;
} // namespace rh::engine

namespace rh::rw::engine
{
class RTTlasBuildPass
{
  public:
    RTTlasBuildPass( rh::engine::IDeviceState &device );
    virtual ~RTTlasBuildPass();

    rh::engine::VulkanTopLevelAccelerationStructure *
    Execute( std::vector<rh::engine::VkAccelerationStructureInstanceKHR>
                 &&instance_buffer );

    rh::engine::CommandBufferSubmitInfo
    GetSubmitInfo( rh::engine::ISyncPrimitive *dependency );

  private:
    rh::engine::IDeviceState        &Device;
    rh::engine::VulkanCommandBuffer *mTlasCmdBuffer     = nullptr;
    rh::engine::IBuffer             *mTlasScratchBuffer = nullptr;
    rh::engine::IBuffer             *mTlasBuffer;
};
} // namespace rh::rw::engine