//
// Created by peter on 27.02.2021.
//
#pragma once

namespace rh::rw::engine
{
struct FrameState;

class RenderGraphResource
{
  public:
    virtual void Update( const FrameState &state ) = 0;
    virtual ~RenderGraphResource()                 = default;
    static constexpr uint64_t UninitializedId      = 0xFFFFFFFFFFFFFFFF;
};

} // namespace rh::rw::engine