//
// Created by peter on 17.04.2020.
//

#pragma once
#include "MemoryReader.h"
#include "MemoryWriter.h"
#include <Windows.h>
#include <cstdint>
#include <functional>
#include <string>
namespace rh::rw::engine
{
struct SharedMemoryTaskQueueInfo
{
    std::string mName;
    int32_t     mSize;
    bool        mOwner;
};

enum SharedMemoryTaskType : int64_t
{
    CREATE_WINDOW          = 1,
    GET_VIDEO_MODE         = 2,
    GET_CURRENT_VIDEO_MODE = 3,
    GET_VIDEO_MODE_COUNT   = 4,
    GET_ADAPTER_COUNT      = 5,
    TEXTURE_LOAD           = 6,
    RENDER                 = 7,
    MESH_LOAD              = 8,
    DESTROY_WINDOW         = 9,
    DESTROY_RASTER         = 10,
    MATERIAL_LOAD          = 11,
    MATERIAL_DELETE        = 12,
    MESH_DELETE            = 13,
    SKINNED_MESH_LOAD,
    SKINNED_MESH_UNLOAD,
};

class SharedMemoryTask
{
  public:
    explicit SharedMemoryTask( std::function<void( void *memory )> &&execute )
        : mExecute( std::move( execute ) )
    {
    }

  private:
    std::function<void( void *memory )> mExecute;
    friend class SharedMemoryTaskQueue;
};

constexpr auto EmptySerializer   = []( MemoryWriter &&memory_writer ) {};
constexpr auto EmptyDeserializer = []( MemoryReader &&memory_reader ) {};

class SharedMemoryTaskQueue
{
  public:
    explicit SharedMemoryTaskQueue( const SharedMemoryTaskQueueInfo &info );
    ~SharedMemoryTaskQueue();

    [[maybe_unused]] void
    RegisterTask( int64_t id, std::unique_ptr<SharedMemoryTask> &&task );
    [[maybe_unused]] void ExecuteTask(
        int64_t                                  id,
        std::function<void( MemoryWriter && )> &&serializer = EmptySerializer,
        std::function<void( MemoryReader && )> &&deserializer =
            EmptyDeserializer );
    [[maybe_unused]] void TaskLoop();

    [[maybe_unused]] void WaitForExit();
    [[maybe_unused]] void SendExitEvent();

  private:
    void * mMappedMemory{};
    HANDLE mSharedMemory{};
    HANDLE mMemoryMutex{};
    HANDLE mTaskStartEvent{};
    HANDLE mTaskFinishEvent{};
    HANDLE mExitEvent{};
    std::unordered_map<int64_t, std::unique_ptr<SharedMemoryTask>> mTaskMap;
};

enum class IPCRenderMode
{
    MultiThreadedRenderer,
    CrossProcessClient,
    CrossProcessRenderer,
};

class IPCSettings
{
  public:
    static IPCRenderMode mMode;
    static std::string   mProcessName;
};

} // namespace rh::rw::engine