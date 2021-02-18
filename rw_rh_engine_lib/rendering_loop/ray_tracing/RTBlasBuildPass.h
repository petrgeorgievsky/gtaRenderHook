//
// Created by peter on 26.06.2020.
//
#pragma once
#include <Engine/Common/IDeviceState.h>
#include <Engine/ResourcePool.h>
#include <cstdint>
#include <queue>
#include <vector>

namespace rh::engine
{
class VulkanCommandBuffer;
class IBuffer;
class IDeviceState;
} // namespace rh::engine

namespace rh::rw::engine
{

struct BLASMeshData
{
    void *mBLAS;
    bool  mBlasBuilt = false;
};

template <typename T> struct PoolEntry
{
    T    mData;
    bool mHasEntry = false;
};
class EngineResourceHolder;
struct BlasBuildPassCreateInfo
{
    rh::engine::IDeviceState &Device;
    EngineResourceHolder &    Resources;
};

class RTBlasBuildPass
{
  public:
    RTBlasBuildPass( const BlasBuildPassCreateInfo &info );
    ~RTBlasBuildPass();
    void RequestBlasBuild( uint64_t mesh_id );
    void Execute();
    bool Completed() const;
    rh::engine::CommandBufferSubmitInfo
    GetSubmitInfo( rh::engine::ISyncPrimitive *dependency );

    const BLASMeshData &GetBlas( uint64_t mesh_id ) const
    {
        return mBLASPool[mesh_id].mData;
    }

  private:
    rh::engine::IDeviceState &           Device;
    EngineResourceHolder &               Resources;
    std::vector<uint64_t>                mNewBLASList;
    std::queue<uint64_t>                 mBLASQueue;
    std::vector<PoolEntry<BLASMeshData>> mBLASPool;
    rh::engine::VulkanCommandBuffer *    mBlasCmdBuffer;
    rh::engine::IBuffer *                mScratchBuffer     = nullptr;
    rh::engine::ISyncPrimitive *         mBlasBuilt         = nullptr;
    uint64_t                             mScratchBufferSize = 0;
    bool                                 mCompleted         = false;
};

} // namespace rh::rw::engine