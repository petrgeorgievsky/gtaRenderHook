//
// Created by peter on 17.04.2020.
//
#include "ipc_utils.h"

#include <Engine/Common/IDeviceState.h>
#include <Engine/EngineConfigBlock.h>
#include <Engine/VulkanImpl/VulkanDeviceState.h>
#include <rw_engine/system_funcs/rw_device_system_globals.h>

namespace rh::rw::engine
{
void InitClient() { gRenderClient = std::make_unique<RenderClient>(); }
void ShutdownClient() { gRenderClient.reset(); }

void InitRenderer() { gRenderDriver = std::make_unique<RenderDriver>(); }

void ShutdownRenderer() { gRenderDriver.reset(); }
} // namespace rh::rw::engine