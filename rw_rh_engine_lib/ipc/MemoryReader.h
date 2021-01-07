//
// Created by peter on 25.06.2020.
//
#pragma once
#include <cstdint>
namespace rh::rw::engine
{
class MemoryReader
{
  private:
    uint64_t offset = 0;
    void *   memory = nullptr;

  public:
    MemoryReader( void *_memory ) : memory( _memory ) {}
    void                     Skip( uint64_t p ) { offset += p; }
    template <typename T> T *Read()
    {
        T *start = static_cast<T *>(
            static_cast<void *>( static_cast<char *>( memory ) + offset ) );
        offset += sizeof( T );
        return start;
    }
    template <typename T> T *Read( uint64_t count )
    {
        T *start = static_cast<T *>(
            static_cast<void *>( static_cast<char *>( memory ) + offset ) );
        offset += sizeof( T ) * count;
        return start;
    }
    uint64_t Pos() { return offset; }
};
} // namespace rh::rw::engine
