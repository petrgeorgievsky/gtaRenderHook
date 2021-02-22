//
// Created by peter on 24.10.2020.
//

#pragma once

//
// Created by peter on 22.10.2020.
//
#pragma once

#include <Engine/Common/ScopedPtr.h>
#include <cstdint>

namespace rh::engine
{
class IDeviceState;
class IImageView;
class IShader;
class IPipelineLayout;
class VulkanComputePipeline;
class IBuffer;
class IImageBuffer;
class ISampler;
class ICommandBuffer;
class IDescriptorSetAllocator;
class IDescriptorSet;
class IDescriptorSetLayout;
} // namespace rh::engine

namespace rh::rw::engine
{
template <typename T> using SPtr = rh::engine::ScopedPointer<T>;

class CameraDescription;
struct AnalyticLightsState;
struct TiledLightCullingParams
{
    // dependencies
    rh::engine::IDeviceState &mDevice;
    CameraDescription *       mCameraDesc;

    uint32_t                mWidth{};
    uint32_t                mHeight{};
    rh::engine::IImageView *mInputValue;
    rh::engine::IImageView *mCurrentDepth;
};

struct TileConfig
{
    uint32_t max_light_count;
    uint32_t max_light_in_tile;
    uint32_t min_depth;
    uint32_t max_depth;
};

class TiledLightCulling
{
  public:
    TiledLightCulling( const TiledLightCullingParams &params );

    void                 Execute( rh::engine::ICommandBuffer *dest,
                                  const AnalyticLightsState & info );
    rh::engine::IBuffer *GetTileListBuffer() { return mTileBuffer; }
    rh::engine::IBuffer *GetLightIdxListBuffer() { return mLightIdxListBuffer; }
    rh::engine::IBuffer *GetLightBuffer() { return mLightBuffer; }

  private:
    CameraDescription *mCameraDesc;
    uint32_t           mWidth{};
    uint32_t           mHeight{};
    TileConfig         Config{};

    SPtr<rh::engine::IDescriptorSetAllocator> mDescSetAlloc;

    // Pipeline for light tile building
    SPtr<rh::engine::VulkanComputePipeline> mBuildTilesPipeline;
    SPtr<rh::engine::IShader>               mBuildTilesShader;
    SPtr<rh::engine::IPipelineLayout>       mBuildTilesLayout;
    SPtr<rh::engine::IDescriptorSetLayout>  mBuildTilesDescSetLayout;
    SPtr<rh::engine::IDescriptorSet>        mBuildTilesDescSet;
    SPtr<rh::engine::IBuffer>               mTileConfigBuffer;
    SPtr<rh::engine::IBuffer>               mTileBuffer;
    SPtr<rh::engine::IBuffer>               mLightIdxListBuffer;
    SPtr<rh::engine::IBuffer>               mLightBuffer;
};
} // namespace rh::rw::engine