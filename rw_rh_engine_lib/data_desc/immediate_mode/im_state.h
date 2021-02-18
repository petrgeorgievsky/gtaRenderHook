//
// Created by peter on 16.02.2021.
//
#pragma once
#include <cstdint>
namespace rh::rw::engine
{
struct ImmediateState
{
    uint64_t Raster;

    uint8_t ColorBlendSrc{};
    uint8_t ColorBlendDst{};
    uint8_t ColorBlendOp{};
    uint8_t BlendEnable{};

    uint8_t ZTestEnable{};
    uint8_t ZWriteEnable{};
    uint8_t StencilEnable{};
    ImmediateState();
    void Update( int32_t nState, void *pParam );
};
} // namespace rh::rw::engine