#include <doctest/doctest.h>

#include "Harness/ComputeTestRunner.h"

#include <Engine/Common/IBuffer.h>
#include <rendering_loop/ray_tracing/restir/restir_structs.hpp>

#include <array>
#include <cstring>

using rh::rw::engine::restir::Reservoir;

static_assert( sizeof( Reservoir ) == 32,
               "Reservoir layout changed: update the GLSL struct and this test" );

TEST_CASE( "restir/reservoir_layout_roundtrip" )
{
    rh::tests::gpu::ComputeTestRunner runner;

    const float input[8] = { 1.5f, 2.5f, 3.0f, -1.0f,
                             4.5f, 5.5f, 6.0f, 7.0f };

    auto *in  = runner.CreateStorageBuffer( sizeof( input ), input );
    auto *out = runner.CreateStorageBuffer( 2 * sizeof( Reservoir ) );

    const std::array bindings = { rh::tests::gpu::BufferBinding{ 0, 0, in },
                                  rh::tests::gpu::BufferBinding{ 0, 1, out } };
    runner.Run( "shaders/tests/restir/reservoir_layout.comp", bindings );

    const auto bytes = runner.Readback( out, 2 * sizeof( Reservoir ) );
    Reservoir  records[2]{};
    std::memcpy( records, bytes.data(), sizeof( records ) );

    CHECK( records[0].totalWeight == doctest::Approx( 1.5f ) );
    CHECK( records[0].selectedLightWeight == doctest::Approx( 2.5f ) );
    CHECK( records[0].visitedSampleCount == 3 );
    CHECK( records[0].selectedLightId == -1 );
    CHECK( records[0].selectedLightSeed == 3000u );

    CHECK( records[1].totalWeight == doctest::Approx( 4.5f ) );
    CHECK( records[1].selectedLightWeight == doctest::Approx( 5.5f ) );
    CHECK( records[1].visitedSampleCount == 6 );
    CHECK( records[1].selectedLightId == 7 );
    CHECK( records[1].selectedLightSeed == 6000u );
}
