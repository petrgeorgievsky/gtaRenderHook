//
// Created by peter on 07.05.2020.
//
#pragma once
#include <DebugUtils/DebugLogger.h>
#include <cstdint>
#include <functional>
#include <vector>

namespace rh::engine
{

template <typename T> class ResourcePool
{
    enum ResourceFlags : uint64_t
    {
        Unused,
        FreeAtNextGC,
        Immortal,
        Used
    };

    /*struct ResourceInfo
    {
        ResourceFlags mFlags = ResourceFlags::Unused;
    };*/

  public:
    ResourcePool( uint64_t size, std::function<void( T &, uint64_t )> destruct )
    {
        mDestructor = destruct;
        mResourcePool.resize( size );
        mResourcePoolData.resize( size );
    }
    ~ResourcePool() { CleanResources(); }

    T &GetResource( uint64_t idx ) { return mResourcePoolData[idx]; }

    const T &GetResource( uint64_t idx ) const
    {
        return mResourcePoolData[idx];
    }

    uint64_t RequestResource( T resource, bool very_important = false )
    {
        if ( mFreeResourceIdx <= mResourcePool.size() )
        {
            // search for first unused resource
            auto res = FindFreeResourceIdx();
            if ( !res.second )
            {
                // try to free a resource
                CollectGarbage( 1 );
                res = FindFreeResourceIdx();
            }
            // out of pool memory
            if ( res.first >= mResourcePool.size() )
            {
                mResourcePool.push_back( {} );
                mResourcePoolData.push_back( {} );
            }
            mResourcePoolData[res.first] = std::move( resource );
            mResourcePool[res.first] =
                very_important ? ResourceFlags::Immortal : ResourceFlags::Used;
            if ( mOnRequestCallback )
                mOnRequestCallback( mResourcePoolData[res.first], res.first );
            mFreeResourceIdx = res.first + 1;
            // good path
            return res.first;
        }
        else // out of pool memory
        {
            mResourcePool.push_back( very_important ? ResourceFlags::Immortal
                                                    : ResourceFlags::Used );
            mResourcePoolData.push_back( std::move( resource ) );
            if ( mOnRequestCallback )
                mOnRequestCallback(
                    mResourcePoolData[mResourcePoolData.size() - 1],
                    mResourcePoolData.size() - 1 );
            return mFreeResourceIdx++;
        }
    }

    std::pair<uint64_t, bool> FindFreeResourceIdx()
    {
        uint64_t idx                 = mFreeResourceIdx;
        bool     found_free_resource = false;
        while ( idx < mResourcePool.size() )
        {
            found_free_resource =
                ( mResourcePool[idx] == ResourceFlags::Unused );
            if ( found_free_resource )
                break;
            ++idx;
        }
        return { idx, found_free_resource };
    }

    void FreeResource( uint64_t idx )
    {
        mResourcePool[idx] = ResourceFlags::FreeAtNextGC;
        // mFreeResourceIdx   = idx;
        mGarbageCount++;
    }

    void CleanResources()
    {
        std::vector<uint32_t> used_resources;
        used_resources.reserve( mResourcePool.size() );
        uint32_t idx = 0;
        for ( auto &resource : mResourcePool )
        {
            if ( resource != ResourceFlags::FreeAtNextGC &&
                 resource != ResourceFlags::Unused )
                debug::DebugLogger::Log(
                    "Resource was not freed before exit!" );
            if ( resource != ResourceFlags::Unused )
                used_resources.push_back( idx );
            resource = ResourceFlags::Unused;
            idx++;
        }
        for ( auto i : used_resources )
        {
            auto &resource = mResourcePoolData[i];
            if ( mDestructor )
                mDestructor( resource, i );
        }
        mGarbageCount    = 0;
        mFreeResourceIdx = 0;
    }

    void CollectGarbage( uint64_t to_free_resource_count )
    {
        if ( mGarbageCount <= 0 )
            return;
        ;
        int64_t free_idx = -1;

        for ( uint64_t idx = 0; idx < mResourcePoolData.size(); idx++ )
        {
            auto &flags = mResourcePool[idx];
            if ( flags != ResourceFlags::FreeAtNextGC )
                continue;

            auto &resource = mResourcePoolData[idx];
            if ( free_idx == -1 )
                free_idx = idx;
            if ( mDestructor )
                mDestructor( resource, idx );
            resource = {};
            flags    = ResourceFlags::Unused;
            mGarbageCount--;
            if ( to_free_resource_count-- <= 0 )
                break;
        }
        if ( free_idx == -1 )
            mFreeResourceIdx = 0;
        else if ( free_idx < mFreeResourceIdx )
            mFreeResourceIdx = free_idx;
    }

    T *      GetStorage() { return mResourcePoolData.data(); }
    uint64_t GetSize() { return mResourcePoolData.size(); }

    void AddOnRequestCallback( std::function<void( T &, uint64_t )> &&cb )
    {
        mOnRequestCallback = [old_cb = mOnRequestCallback,
                              new_cb = std::move( cb )]( T &val, uint64_t id ) {
            if ( old_cb )
                old_cb( val, id );
            new_cb( val, id );
        };
    }

    void AddOnDestructCallback( std::function<void( T &, uint64_t )> &&cb )
    {
        mDestructor = [old_cb = mDestructor,
                       new_cb = std::move( cb )]( T &val, uint64_t id ) {
            new_cb( val, id );
            if ( old_cb )
                old_cb( val, id );
        };
    }

  private:
    uint64_t                                mGarbageCount    = 0;
    uint64_t                                mFreeResourceIdx = 0;
    std::vector<ResourceFlags>              mResourcePool{};
    std::vector<T>                          mResourcePoolData{};
    std::function<void( T &, uint64_t id )> mDestructor{};
    std::function<void( T &, uint64_t id )> mOnRequestCallback{};
};

} // namespace rh::engine