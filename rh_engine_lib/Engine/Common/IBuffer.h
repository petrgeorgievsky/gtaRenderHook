#pragma once
#include <cstdint>

namespace rh::engine
{

enum BufferUsage : uint32_t
{
    VertexBuffer      = 0b1u,
    IndexBuffer       = 0b10u,
    ConstantBuffer    = 0b100u,
    StagingBuffer     = 0b1000u,
    RayTracingScratch = 0b10000u,
    StorageBuffer     = 0b100000u,
};

enum BufferFlags
{
    // Initialize once, don't allow to update
    Immutable,
    Dynamic
};

struct BufferCreateInfo
{
    uint32_t    mSize;
    uint32_t    mUsage;
    BufferFlags mFlags;
    void *      mInitDataPtr;
};

class IBuffer
{
  public:
    virtual ~IBuffer()                                     = default;
    virtual void Update( const void *data, uint32_t size ) = 0;
    virtual void Update( const void *data, uint32_t size, uint32_t offset ) = 0;
    virtual void *Lock()                                                    = 0;
    virtual void  Unlock()                                                  = 0;
};

} // namespace rh::engine
