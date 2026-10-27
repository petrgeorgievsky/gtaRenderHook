#include <doctest/doctest.h>

#include <cstddef>
#include <rendering_loop/ray_tracing/restir/restir_structs.hpp>

using rh::rw::engine::restir::Reservoir;

TEST_CASE( "restir/reservoir_cpp_layout" )
{
    static_assert( sizeof( Reservoir ) == 32 );
    static_assert( offsetof( Reservoir, totalWeight ) == 0 );
    static_assert( offsetof( Reservoir, selectedLightWeight ) == 4 );
    static_assert( offsetof( Reservoir, visitedSampleCount ) == 8 );
    static_assert( offsetof( Reservoir, selectedLightId ) == 12 );
    static_assert( offsetof( Reservoir, selectedLightSeed ) == 16 );
    CHECK( true );
}
