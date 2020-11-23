//
// Created by peter on 17.04.2020.
//

#include "shared_memory_queue_client.h"
#include <DebugUtils/DebugLogger.h>
#include <Windows.h>
#include <cassert>
#include <sstream>

namespace rh::rw
{
engine::IPCRenderMode engine::IPCSettings::mMode =
    IPCRenderMode::MultiThreadedRenderer;
std::string engine::IPCSettings::mProcessName{};

engine::SharedMemoryTaskQueue::SharedMemoryTaskQueue(
    const SharedMemoryTaskQueueInfo &info )
{

    // Create synchronisation primitives
    SECURITY_ATTRIBUTES event_attr{};
    event_attr.nLength        = sizeof( SECURITY_ATTRIBUTES );
    event_attr.bInheritHandle = true;

    std::stringstream ss{};
    // Create shared memory for IPC
    if ( info.mOwner )
    {
        mSharedMemory = CreateFileMapping(
            INVALID_HANDLE_VALUE, // use paging file
            &event_attr,          // default security
            PAGE_READWRITE,       // read/write access
            0,                    // maximum object size (high-order DWORD)
            info.mSize,           // maximum object size (low-order DWORD)
            info.mName.c_str() ); // name of mapping object
    }
    else
    {
        mSharedMemory =
            OpenFileMapping( FILE_MAP_ALL_ACCESS, // read/write access
                             FALSE, // maximum object size (high-order DWORD)
                             info.mName.c_str() ); // name of mapping object
    }
    if ( mSharedMemory == nullptr )
    {

        std::string error_msg =
            info.mOwner ? "Failed to create shared memory, x86 to x64 "
                          "render mode is unavailable! Error code:"
                        : "Failed to open shared memory, x86 to x64 "
                          "render mode is unavailable! Error code:";
        ss << error_msg << GetLastError();

        debug::DebugLogger::Log( ss.str(), debug::LogLevel::Error );
        return;
    }
    std::string log_msg = info.mOwner ? "Succesfuly create shared memory "
                                      : "Succesfuly opened shared memory ";
    ss << log_msg << info.mName;
    debug::DebugLogger::Log( ss.str(), debug::LogLevel::Info );
    ss.str( "" );

    // Map shared memory view
    mMappedMemory = MapViewOfFile( mSharedMemory,       // handle to map object
                                   FILE_MAP_ALL_ACCESS, // read/write permission
                                   0, 0, info.mSize );

    if ( mMappedMemory == nullptr )
    {

        ss << "Failed to map shared memory, x86 to x64 "
              "render mode is unavailable! Error code:"
           << GetLastError();

        debug::DebugLogger::Log( ss.str(), debug::LogLevel::Error );

        CloseHandle( mSharedMemory );
        return;
    }

    ss << "Succesfuly mapped shared memory " << info.mName;

    debug::DebugLogger::Log( ss.str(), debug::LogLevel::Info );
    ss.str( "" );

    auto task_start_name  = info.mName + "TaskStartEvent";
    auto task_finish_name = info.mName + "TaskFinishEvent";
    auto exit_event_name  = info.mName + "ExitEvent";
    auto task_mutex       = info.mName + "Mtx";

    auto create_event_safe_lambda = [&ss]( SECURITY_ATTRIBUTES event_attr,
                                           const std::string & name ) {
        auto event = CreateEvent( &event_attr, FALSE, FALSE, name.c_str() );
        if ( event == nullptr )
        {
            ss << "Failed to create " << name
               << "event! Error code:" << GetLastError();

            debug::DebugLogger::Log( ss.str(), debug::LogLevel::Error );
        }
        else
        {
            ss << "Created " << name << " event";
            debug::DebugLogger::Log( ss.str(), debug::LogLevel::Info );
        }

        ss.str( "" );
        return event;
    };

    auto create_mutex_safe_lambda = [&ss]( SECURITY_ATTRIBUTES event_attr,
                                           const std::string & name ) {
        auto mtx = CreateMutex( &event_attr, FALSE, name.c_str() );
        if ( mtx == nullptr )
        {
            ss << "Failed to create " << name << "mutex"
               << "Error code:" << GetLastError();

            debug::DebugLogger::Log( ss.str(), debug::LogLevel::Error );
        }
        else
        {
            ss << "Created " << name << " mutex";
            debug::DebugLogger::Log( ss.str(), debug::LogLevel::Info );
        }

        ss.str( "" );
        return mtx;
    };

    auto open_event_safe_lambda = [&ss]( SECURITY_ATTRIBUTES event_attr,
                                         const std::string & name ) {
        auto event = OpenEvent( EVENT_ALL_ACCESS, FALSE, name.c_str() );
        if ( event == nullptr )
        {
            ss << "Failed to open " << name
               << "event! Error code:" << GetLastError();

            debug::DebugLogger::Log( ss.str(), debug::LogLevel::Error );
        }
        else
        {
            ss << "Opened " << name << " event";
            debug::DebugLogger::Log( ss.str(), debug::LogLevel::Info );
        }

        ss.str( "" );
        return event;
    };

    auto open_mutex_safe_lambda = [&ss]( SECURITY_ATTRIBUTES event_attr,
                                         const std::string & name ) {
        auto mtx = OpenMutex( MUTEX_ALL_ACCESS, FALSE, name.c_str() );
        if ( mtx == nullptr )
        {
            ss << "Failed to open " << name << "mutex"
               << "Error code:" << GetLastError();

            debug::DebugLogger::Log( ss.str(), debug::LogLevel::Error );
        }
        else
        {
            ss << "Opened " << name << " mutex";
            debug::DebugLogger::Log( ss.str(), debug::LogLevel::Info );
        }

        ss.str( "" );
        return mtx;
    };

    if ( info.mOwner )
    {
        mTaskStartEvent =
            create_event_safe_lambda( event_attr, task_start_name );

        mTaskFinishEvent =
            create_event_safe_lambda( event_attr, task_finish_name );

        mExitEvent = create_event_safe_lambda( event_attr, exit_event_name );

        mMemoryMutex = create_mutex_safe_lambda( event_attr, task_mutex );
    }
    else
    {
        mTaskStartEvent = open_event_safe_lambda( event_attr, task_start_name );

        mTaskFinishEvent =
            open_event_safe_lambda( event_attr, task_finish_name );

        mExitEvent = open_event_safe_lambda( event_attr, exit_event_name );

        mMemoryMutex = open_mutex_safe_lambda( event_attr, task_mutex );
    }
}
engine::SharedMemoryTaskQueue::~SharedMemoryTaskQueue()
{
    CloseHandle( mExitEvent );
    CloseHandle( mMemoryMutex );
    CloseHandle( mTaskStartEvent );
    CloseHandle( mTaskFinishEvent );
    UnmapViewOfFile( mMappedMemory );
    CloseHandle( mSharedMemory );
}

[[maybe_unused]] void engine::SharedMemoryTaskQueue::ExecuteTask(
    int64_t id, std::function<void( MemoryWriter &&reader )> &&serializer,
    std::function<void( MemoryReader &&writer )> &&deserializer )
{
    WaitForSingleObject( mMemoryMutex, INFINITE );

    *static_cast<int64_t *>( mMappedMemory ) = id;
    serializer( static_cast<char *>( mMappedMemory ) + sizeof( int64_t ) );
    SetEvent( mTaskStartEvent );

    DWORD wait_res = WaitForSingleObject( mTaskFinishEvent, 100 );
    while ( wait_res == WAIT_TIMEOUT )
    {
        // skip msgs
        MSG m;
        while ( PeekMessage( &m, nullptr, 0, 0, PM_REMOVE ) )
        {
            TranslateMessage( &m );
            DispatchMessage( &m );
        }
        wait_res = WaitForSingleObject( mTaskFinishEvent, 100 );
    }

    assert( *static_cast<int64_t *>( mMappedMemory ) == id );
    deserializer( static_cast<char *>( mMappedMemory ) + sizeof( int64_t ) );

    ReleaseMutex( mMemoryMutex );
}
[[maybe_unused]] void engine::SharedMemoryTaskQueue::TaskLoop()
{
    auto idle_time = 1000L;
    if ( WaitForSingleObject( mTaskStartEvent, idle_time ) == WAIT_OBJECT_0 )
    {
        int64_t task_id = *static_cast<int64_t *>( mMappedMemory );
        mTaskMap[task_id]->mExecute( static_cast<char *>( mMappedMemory ) +
                                     sizeof( int64_t ) );
        SetEvent( mTaskFinishEvent );
    }
}
[[maybe_unused]] void engine::SharedMemoryTaskQueue::RegisterTask(
    int64_t id, std::unique_ptr<SharedMemoryTask> &&task )
{
    mTaskMap[id] = std::move( task );
}

[[maybe_unused]] void engine::SharedMemoryTaskQueue::WaitForExit()
{
    WaitForSingleObject( mExitEvent, INFINITE );
}

[[maybe_unused]] void engine::SharedMemoryTaskQueue::SendExitEvent()
{
    SetEvent( mExitEvent );
}

} // namespace rh::rw