//
// Created by peter on 03.06.2020.
//

#pragma once
#include <string>
namespace rh::engine
{
class IBuffer;
class ICommandBuffer;
class ISyncPrimitive;
class VulkanDebugUtils
{
  public:
    static void SetDebugName( IBuffer *buffer, std::string name );
    static void SetDebugName( ICommandBuffer *buffer, std::string name );
    static void SetDebugName( ISyncPrimitive *sp, std::string name );
};
} // namespace rh::engine