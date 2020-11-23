#pragma once
#include "../../common_headers.h"
#include <cstdint>

struct RwCamera;

namespace rh
{

namespace engine
{
class IWindow;
class ISyncPrimitive;
class ICommandBuffer;
class IFrameBuffer;
class IRenderPass;
class ISwapchain;
} // namespace engine

namespace rw::engine
{
/*
constexpr auto gFrameResourceCacheSize = 4;
constexpr auto gFramebufferCacheSize   = 4;

struct PerFrameResources
{
    rh::engine::ISyncPrimitive *mImageAquire;
    rh::engine::ISyncPrimitive *mRenderExecute;
    rh::engine::ICommandBuffer *mCmdBuffer;
    bool                        mBufferIsRecorded = false;
};*/

struct BackendCameraExt
{
    /* PerFrameResources         mFrameResourceCache[gFrameResourceCacheSize];
     rh::engine::IFrameBuffer *mFramebufferCache[gFramebufferCacheSize];
     rh::engine::ISwapchain *  mSwapchain;
     rh::engine::IRenderPass * mRenderPass;
     uint32_t                  mClearFlags;
     RwRGBA                    mClearColor;
     float                     mClearDepth;
     int                       mCurrentFrameId;
     int                       mCurrentFramebufferId;*/
};

extern int32_t gBackendCameraExtOffset;
void *BackendCameraCtor( void *object, [[maybe_unused]] int32_t offsetInObject,
                         [[maybe_unused]] int32_t sizeInObject );
void *BackendCameraDtor( void *object, [[maybe_unused]] int32_t offsetInObject,
                         [[maybe_unused]] int32_t sizeInObject );
/* Plugin Attach */
int32_t BackendCameraPluginAttach();

/* Open/Close */
// void BackendCameraOpen();
// void BackendCameraClose();

BackendCameraExt *GetBackendCameraExt( RwCamera *camera );

} // namespace rw::engine
} // namespace rh
