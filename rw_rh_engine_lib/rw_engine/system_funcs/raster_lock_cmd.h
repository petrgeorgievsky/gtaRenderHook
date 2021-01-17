//
// Created by peter on 07.01.2021.
//

#pragma once

#include <cstdint>
namespace rh::rw::engine
{

struct ImageLockParams
{
    static constexpr uint8_t LockRead  = 1;
    static constexpr uint8_t LockWrite = 2;

    uint64_t mImageId  = 0xBADF00D;
    uint32_t mWidth    = 1;
    uint32_t mHeight   = 1;
    uint32_t mDepth    = 1;
    uint8_t  mLockMode = LockWrite;
    uint8_t  mMipLevel = 0;
    uint16_t mPadding;
};

struct ImageLockResultData
{
    uint32_t mLockDataHeight;
    uint32_t mLockDataStride;
};
struct ImageLockResult : ImageLockResultData
{
    void *mData = nullptr;
};
class ImageLockCmdImpl
{
  public:
    ImageLockResult Invoke( const ImageLockParams &params );
    static void     RegisterCallHandler();
};
} // namespace rh::rw::engine