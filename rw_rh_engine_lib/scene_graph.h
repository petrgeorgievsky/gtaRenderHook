//
// Created by peter on 16.04.2020.
//
#pragma once

#include "common_headers.h"
#include <cstdint>
#include <ipc/MemoryWriter.h>
namespace rh::rw::engine
{

struct PointLight
{
    float mPos[3];
    float mRadius;
    float mColor[4];
};

struct FrameInfo
{
    bool                camera_updated = false;
    int32_t             mClearFlags;
    RwRGBA              mClearColor;
    float               mClearDepth;
    uint32_t            mClearPadding;
    DirectX::XMFLOAT4X4 mView;
    DirectX::XMFLOAT4X4 mProj;
    DirectX::XMFLOAT4X4 mViewInv;
    DirectX::XMFLOAT4X4 mProjInv;
    DirectX::XMFLOAT4X4 mProjPrev;
    DirectX::XMFLOAT4X4 mViewPrev;
    float               mSkyTopColor[4];
    float               mSkyBottomColor[4];
    float               mSunDir[4];
    PointLight          mFirst4PointLights[1024];
    uint32_t            mLightCount = 0;
};

struct SceneGraph
{
    FrameInfo mFrameInfo;
    uint16_t  mFrameId;
};

SceneGraph *GetCurrentSceneGraph();

void SerializeSceneGraph( MemoryWriter &memory_writer );
} // namespace rh::rw::engine