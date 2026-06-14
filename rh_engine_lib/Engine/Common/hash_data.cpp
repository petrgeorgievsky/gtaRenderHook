#pragma once
#include <rapidhash.h>
namespace rh::engine
{
size_t fast_hash( const void *key, size_t len )
{
    return static_cast<size_t>( rapidhashNano( key, len ) );
}
} // namespace rh::engine