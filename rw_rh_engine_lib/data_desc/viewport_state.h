//
// Created by peter on 17.02.2021.
//

#pragma once
#include <common_headers.h>
namespace rh::rw::engine
{
struct CameraState
{
    DirectX::XMFLOAT4X4 mView;
    DirectX::XMFLOAT4X4 mProj;
    DirectX::XMFLOAT4X4 mViewInv;
    DirectX::XMFLOAT4X4 mProjInv;
    DirectX::XMFLOAT4X4 mProjPrev;
    DirectX::XMFLOAT4X4 mViewPrev;
};
struct MainViewportState
{
    int32_t     ClearFlags;
    RwRGBA      ClearColor;
    float       ClearDepth;
    uint32_t    ClearPadding;
    CameraState Camera;
};
} // namespace rh::rw::engine