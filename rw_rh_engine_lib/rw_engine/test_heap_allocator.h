//
// Created by peter on 25.04.2021.
//
#pragma once
#include <DebugUtils/DebugLogger.h>
#include <mutex>
#include <string>
#include <unordered_map>

namespace rh::rw::engine
{

class HeapAllocator
{
    struct UnmanagedHeapBlock
    {
        std::string Name;
        size_t      Size;
    };

  public:
    static HeapAllocator &GlobalHAlloc()
    {
        static HeapAllocator heap_alloc;
        return heap_alloc;
    }

    void *Alloc( const char *name, size_t size )
    {
        std::lock_guard l( mtx );

        auto allocation = malloc( size );

        if ( TrackMemory )
        {
            [[maybe_unused]] auto [it, inserted] =
                Heap.emplace( (uintptr_t)allocation,
                              UnmanagedHeapBlock{ std::string{ name }, size } );
            assert( inserted );
        }

        return allocation;
    }
    void Free( void *ptr )
    {
        std::lock_guard l( mtx );

        free( ptr );

        if ( TrackMemory )
            Heap.erase( (uintptr_t)ptr );
    }

    ~HeapAllocator()
    {
        if ( !Heap.empty() )
        {
            debug::DebugLogger::ErrorFmt(
                "Heap allocated wasn't freed for %u allocations:",
                Heap.size() );
            std::string err_fmt = "Block %s at %llX of size %u;";
            for ( auto [ptr, heap_block] : Heap )
            {
                debug::DebugLogger::ErrorFmt( err_fmt, heap_block.Name.c_str(),
                                              (uintptr_t)ptr, heap_block.Size );
            }
        }
    }

  private:
    bool                                              TrackMemory = false;
    std::mutex                                        mtx;
    std::unordered_map<uintptr_t, UnmanagedHeapBlock> Heap;
};

// default implementation
template <typename T> struct TypeName
{
    static const char *Get() { return typeid( T ).name(); }
};

template <typename Type> Type *hAlloc( const char *name )
{
    auto result =
        (Type *)HeapAllocator::GlobalHAlloc().Alloc( name, sizeof( Type ) );
    debug::DebugLogger::LogFmt(
        "Allocated data for %s of type %s at %X of size: %u",
        debug::LogLevel::Info, name, TypeName<Type>::Get(), (uint64_t)result,
        sizeof( Type ) );
    return result;
}

template <typename Type> Type *hAlloc( const char *name, size_t size )
{
    auto result = (Type *)HeapAllocator::GlobalHAlloc().Alloc( name, size );
    debug::DebugLogger::LogFmt( "Allocated data for %s of type %s at %X of "
                                "size: %u, original structure size: %u",
                                debug::LogLevel::Info, name,
                                TypeName<Type>::Get(), (uint64_t)result, size,
                                sizeof( Type ) );
    return result;
}

template <typename Type> Type *hAllocArray( const char *name, size_t count )
{
    auto result = (Type *)HeapAllocator::GlobalHAlloc().Alloc(
        name, sizeof( Type ) * count );
    debug::DebugLogger::LogFmt( "Allocated array for %s of type %s at %X of "
                                "size: %u, count: %u",
                                debug::LogLevel::Info, name,
                                TypeName<Type>::Get(), (uint64_t)result,
                                sizeof( Type ) * count, count );
    return result;
}

template <typename Type> void hFree( Type *t )
{
    debug::DebugLogger::LogFmt( "Freeing data at %X of type %s",
                                debug::LogLevel::Info, (uint64_t)t,
                                TypeName<Type>::Get() );
    HeapAllocator::GlobalHAlloc().Free( t );
}
} // namespace rh::rw::engine