//
// Created by peter on 09.10.2020.
//
#pragma once

namespace rh::engine::tests
{
class IMouseState
{
  public:
    virtual void Capture() = 0;
    virtual void Release() = 0;
};

} // namespace rh::engine::tests