#pragma once
#include <cstdint>

namespace rh::engine
{

enum BufferUsage
{
    VertexBuffer   = 0b1,
    IndexBuffer    = 0b10,
    ConstantBuffer = 0b100,
};

struct BufferCreateInfo
{
    uint32_t mSize;
    uint32_t mUsage;
};

class IBuffer
{
  public:
    virtual ~IBuffer()                                     = default;
    virtual void Update( const void *data, uint32_t size ) = 0;
};

} // namespace rh::engine
