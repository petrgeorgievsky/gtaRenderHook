//
// Created by peter on 25.06.2020.
//
#pragma once
#include <Engine/Common/ArrayProxy.h>
#include <Windows.h>
#include <cstdint>
namespace rh::rw::engine
{
class MemoryWriter
{
  private:
    uint64_t offset = 0;
    void *   memory = nullptr;

  public:
    MemoryWriter( void *_memory ) : memory( _memory ) {}

    void Skip( uint64_t p ) { offset += p; }
    void SeekFromCurrent( int64_t p ) { offset += p; }

    template <typename T> void Write( T *data )
    {
        CopyMemory( static_cast<char *>( memory ) + offset, data, sizeof( T ) );
        offset += sizeof( T );
    }
    template <typename T> void Write( T *data, uint64_t count )
    {
        CopyMemory( static_cast<char *>( memory ) + offset, data,
                    sizeof( T ) * count );
        offset += sizeof( T ) * count;
    }
    template <typename T> void Write( const rh::engine::ArrayProxy<T> &data )
    {
        CopyMemory( static_cast<char *>( memory ) + offset, data.Data(),
                    sizeof( T ) * data.Size() );
        offset += sizeof( T ) * data.Size();
    }
    template <typename T> T &Current()
    {
        return *reinterpret_cast<T *>(
            ( static_cast<char *>( memory ) + offset ) );
    }

    template <typename T> T *CurrentPtr()
    {
        return reinterpret_cast<T *>(
            ( static_cast<char *>( memory ) + offset ) );
    }
    uint64_t Pos() { return offset; }
};
} // namespace rh::rw::engine