//
// Created by peter on 26.06.2020.
//
#pragma once
#include <Engine/Common/IDeviceState.h>
#include <Engine/Common/ScopedPtr.h>
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
    EngineResourceHolder     &Resources;
    uint32_t                  ScratchBufferCount    = 2;
    uint32_t                  ScratchBufferBaseSize = 2 * 1024 * 1024;
};

struct ScratchBuffer
{
    uint64_t                                       Size = 0;
    rh::engine::ScopedPointer<rh::engine::IBuffer> Data{};
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
        return BLASPool[mesh_id].mData;
    }

  private:
    rh::engine::IDeviceState &Device;
    EngineResourceHolder     &Resources;

    std::vector<uint64_t>                NewBLASList;
    std::queue<uint64_t>                 BLASQueue;
    std::vector<PoolEntry<BLASMeshData>> BLASPool;
    rh::engine::VulkanCommandBuffer     *BlasCmdBuffer;
    std::vector<ScratchBuffer>           ScratchBuffers{};
    rh::engine::ISyncPrimitive          *BlasBuilt = nullptr;
    uint8_t                              ScratchBufferCount;
    bool                                 IsCompleted = false;
};

} // namespace rh::rw::engine