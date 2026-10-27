#include <doctest/doctest.h>

#include "Harness/ComputeTestRunner.h"

#include <Engine/Common/IBuffer.h>
#include <rendering_loop/ray_tracing/restir/restir_structs.hpp>

#include <array>
#include <cmath>
#include <cstdint>
#include <cstring>

using rh::rw::engine::restir::Reservoir;
using rh::tests::gpu::BufferBinding;
using rh::tests::gpu::ComputeTestRunner;

namespace
{

struct UpdateInput
{
    uint32_t seed;
    int32_t  light_ids[8];
    uint32_t sample_seeds[8];
    float    weights[8];
    int32_t  count;
};

struct CombineInput
{
    float    sun_dir[4];
    int32_t  counts[4];
    Reservoir a;
    Reservoir b;
    uint32_t seed;
};

struct CombineOutput
{
    Reservoir result;
};

Reservoir RunUpdate( ComputeTestRunner &runner, const UpdateInput &input )
{
    auto *in  = runner.CreateStorageBuffer( sizeof( input ), &input );
    auto *out = runner.CreateStorageBuffer( sizeof( Reservoir ) );

    const std::array bindings = { BufferBinding{ 0, 0, in },
                                  BufferBinding{ 0, 1, out } };
    runner.Run( "shaders/tests/restir/reservoir_update.comp", bindings );

    const auto bytes = runner.Readback( out, sizeof( Reservoir ) );
    Reservoir  result{};
    std::memcpy( &result, bytes.data(), sizeof( result ) );
    return result;
}

Reservoir RunCombine( ComputeTestRunner &runner, const CombineInput &input )
{
    auto *in  = runner.CreateStorageBuffer( sizeof( input ), &input );
    auto *out = runner.CreateStorageBuffer( sizeof( CombineOutput ) );

    const std::array bindings = { BufferBinding{ 0, 0, in },
                                  BufferBinding{ 0, 1, out } };
    runner.Run( "shaders/tests/restir/reservoir_combine.comp", bindings );

    const auto    bytes = runner.Readback( out, sizeof( CombineOutput ) );
    CombineOutput output{};
    std::memcpy( &output, bytes.data(), sizeof( output ) );
    return output.result;
}

} // namespace

TEST_CASE( "restir/update_reservoir_invariants" )
{
    ComputeTestRunner runner;

    UpdateInput input{};
    input.seed         = 1234;
    input.count        = 4;
    input.light_ids[0] = -1;
    input.light_ids[1] = 0;
    input.light_ids[2] = 1;
    input.light_ids[3] = 2;
    input.sample_seeds[0] = 0x1111u;
    input.sample_seeds[1] = 0x2222u;
    input.sample_seeds[2] = 0x3333u;
    input.sample_seeds[3] = 0x4444u;
    input.weights[0]   = 1.0f;
    input.weights[1]   = 2.0f;
    input.weights[2]   = 3.0f;
    input.weights[3]   = 4.0f;

    const auto result = RunUpdate( runner, input );

    CHECK( result.visitedSampleCount == 4 );
    CHECK( result.totalWeight == doctest::Approx( 10.0f ) );
    CHECK( result.selectedLightId >= -1 );
    CHECK( result.selectedLightId <= 2 );

    // The stored seed must belong to the selected sample, not to the caller's
    // selection RNG state.
    bool seed_matches_selected_light = false;
    for ( int i = 0; i < input.count; ++i )
    {
        if ( input.light_ids[i] == result.selectedLightId &&
             input.sample_seeds[i] == result.selectedLightSeed )
            seed_matches_selected_light = true;
    }
    CAPTURE( result.selectedLightId );
    CAPTURE( result.selectedLightSeed );
    CHECK( seed_matches_selected_light );
}

TEST_CASE( "restir/combine_reservoirs_weight" )
{
    ComputeTestRunner runner;

    CombineInput input{};
    input.sun_dir[2] = 1.0f;
    input.counts[0]  = 0; // LightsCount
    input.counts[1]  = 0; // TriLightsCount
    input.seed       = 777;

    input.a.totalWeight         = 1.0f;
    input.a.selectedLightWeight = 0.25f;
    input.a.visitedSampleCount  = 4;
    input.a.selectedLightId     = -1;

    input.b.totalWeight         = 0.5f;
    input.b.selectedLightWeight = 0.25f;
    input.b.visitedSampleCount  = 2;
    input.b.selectedLightId     = -1;

    const auto result = RunCombine( runner, input );

    // Sun straight up + normal straight up -> p_hat == 1, so
    // W == totalWeight / M and totalWeight == sum of both contributions.
    CHECK( result.visitedSampleCount == 6 );
    CHECK( result.totalWeight == doctest::Approx( 1.5f ) );
    CHECK( result.selectedLightWeight == doctest::Approx( 0.25f ) );
    CHECK( result.selectedLightId == -1 );
}
