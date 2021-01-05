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
    uint32_t    mSize;
    bool        mOwner;
};

enum SharedMemoryTaskType : int64_t
{
    CREATE_WINDOW = 1,
    DESTROY_WINDOW,
    GET_ADAPTER_COUNT,
    GET_ADAPTER_INFO,
    GET_CURRENT_ADAPTER,
    SET_CURRENT_ADAPTER,
    GET_VIDEO_MODE_COUNT,
    GET_VIDEO_MODE,
    GET_CURRENT_VIDEO_MODE,
    MESH_LOAD,
    MESH_DELETE,
    TEXTURE_LOAD,
    DESTROY_RASTER,
    MATERIAL_LOAD,
    MATERIAL_DELETE,
    SKINNED_MESH_LOAD,
    SKINNED_MESH_UNLOAD,
    RENDER,
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

constexpr auto EmptySerializer   = []( MemoryWriter && ) {};
constexpr auto EmptyDeserializer = []( MemoryReader && ) {};

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