#include <doctest/doctest.h>

#include "Harness/ComputeTestRunner.h"

#include <Engine/Common/IBuffer.h>
#include <rendering_loop/ray_tracing/restir/restir_structs.hpp>

#include <array>
#include <cstdint>
#include <cstring>
#include <vector>

using rh::rw::engine::restir::Reservoir;
using rh::tests::gpu::BufferBinding;
using rh::tests::gpu::ComputeTestRunner;

namespace
{

struct alignas( 16 ) PackedLight
{
    float p0[4];
    float p1[4];
    float p2[4];
};

struct PdfInput
{
    float       sun_dir[4];
    int32_t     counts[4];
    PackedLight lights[8];
    PackedLight tri_lights[8];
    float       tri_transforms[8][16];
    int32_t     test_case[4];
    float       surface_pos[4];
    float       surface_normal[4];
};

struct PdfOutput
{
    float values[4];
};

PdfOutput RunPdf( ComputeTestRunner &runner, const PdfInput &input )
{
    auto *in  = runner.CreateStorageBuffer( sizeof( input ), &input );
    auto *out = runner.CreateStorageBuffer( sizeof( PdfOutput ) );

    const std::array bindings = { BufferBinding{ 0, 0, in },
                                  BufferBinding{ 0, 1, out } };
    runner.Run( "shaders/tests/restir/evaluate_pdf.comp", bindings );

    const auto bytes = runner.Readback( out, sizeof( PdfOutput ) );
    PdfOutput  output{};
    std::memcpy( &output, bytes.data(), sizeof( output ) );
    return output;
}

PdfInput MakeSunInput( float normal_z )
{
    PdfInput input{};
    input.sun_dir[2]        = 1.0f;
    input.counts[0]         = 0;
    input.counts[1]         = 0;
    input.test_case[0]      = -1;
    input.test_case[1]      = 1;
    input.test_case[2]      = 999;
    input.surface_normal[2] = normal_z;
    return input;
}

struct WeightInput
{
    float       sun_dir[4];
    int32_t     counts[4];
    PackedLight lights[8];
    PackedLight tri_lights[8];
    float       tri_transforms[8][16];
    Reservoir   r;
    uint32_t    caller_seeds[4];
    float       surface_pos[4];
    float       surface_normal[4];
};

struct WeightOutput
{
    float values[4];
};

struct CombineSeedInput
{
    float       sun_dir[4];
    int32_t     counts[4];
    PackedLight tri_lights[8];
    float       tri_transforms[8][16];
    Reservoir   a;
    Reservoir   b;
    uint32_t    caller_seed[4];
    float       surface_pos[4];
    float       surface_normal[4];
};

struct CombineSeedOutput
{
    uint32_t values[4];
};

CombineSeedOutput RunCombineSeed( ComputeTestRunner &runner,
                                  const CombineSeedInput &input )
{
    auto *in  = runner.CreateStorageBuffer( sizeof( input ), &input );
    auto *out = runner.CreateStorageBuffer( sizeof( CombineSeedOutput ) );

    const std::array bindings = { BufferBinding{ 0, 0, in },
                                  BufferBinding{ 0, 1, out } };
    runner.Run( "shaders/tests/restir/reservoir_combine_seed.comp", bindings );

    const auto        bytes = runner.Readback( out, sizeof( CombineSeedOutput ) );
    CombineSeedOutput output{};
    std::memcpy( &output, bytes.data(), sizeof( output ) );
    return output;
}

WeightOutput RunWeight( ComputeTestRunner &runner, const WeightInput &input )
{
    auto *in  = runner.CreateStorageBuffer( sizeof( input ), &input );
    auto *out = runner.CreateStorageBuffer( sizeof( WeightOutput ) );

    const std::array bindings = { BufferBinding{ 0, 0, in },
                                  BufferBinding{ 0, 1, out } };
    runner.Run( "shaders/tests/restir/reservoir_weight.comp", bindings );

    const auto  bytes = runner.Readback( out, sizeof( WeightOutput ) );
    WeightOutput output{};
    std::memcpy( &output, bytes.data(), sizeof( output ) );
    return output;
}

} // namespace

TEST_CASE( "restir/evaluate_pdf_backface_is_zero" )
{
    ComputeTestRunner runner;
    const auto       output = RunPdf( runner, MakeSunInput( -1.0f ) );
    CHECK( output.values[0] == doctest::Approx( 0.0f ) );
}

TEST_CASE( "restir/evaluate_pdf_sun_frontface" )
{
    ComputeTestRunner runner;
    const auto       output = RunPdf( runner, MakeSunInput( 1.0f ) );
    CHECK( output.values[0] == doctest::Approx( 1.0f ) );
}

TEST_CASE( "restir/evaluate_pdf_scales_with_light_color" )
{
    ComputeTestRunner runner;

    PdfInput input{};
    input.counts[0]    = 1;
    input.counts[1]    = 0;
    input.test_case[0] = 0;
    input.test_case[1] = 1;
    input.test_case[2] = 1;

    // Point light at (0, 0, 1) with radius 2 (radius must exceed distance,
    // otherwise the geometry term is zero), no spot attenuation.
    input.lights[0].p0[0] = 0.0f;
    input.lights[0].p0[1] = 0.0f;
    input.lights[0].p0[2] = 1.0f;
    input.lights[0].p0[3] = 2.0f;
    input.lights[0].p1[3] = 0.0f;

    input.surface_normal[2] = 1.0f;

    input.lights[0].p2[0] = 1.0f;
    input.lights[0].p2[1] = 0.0f;
    input.lights[0].p2[2] = 0.0f;
    const auto dim = RunPdf( runner, input );

    input.lights[0].p2[0] = 2.0f;
    const auto bright = RunPdf( runner, input );

    REQUIRE( dim.values[0] > 0.0f );
    CHECK( bright.values[0] == doctest::Approx( 2.0f * dim.values[0] ) );
}

TEST_CASE( "restir/sampling_covers_all_ids" )
{
    ComputeTestRunner runner;

    struct
    {
        int32_t  params[4];
        uint32_t seed[4];
    } input{};
    input.params[0] = 6; // LightsCount 3 + TriLightsCount 2 + 1
    input.params[1] = 4096;
    input.seed[0]   = 1234;

    auto *in  = runner.CreateStorageBuffer( sizeof( input ), &input );
    auto *out = runner.CreateStorageBuffer( 4096 * sizeof( int32_t ) );

    const std::array bindings = { BufferBinding{ 0, 0, in },
                                  BufferBinding{ 0, 1, out } };
    runner.Run( "shaders/tests/restir/sampling_ids.comp", bindings );

    const auto           bytes = runner.Readback( out, 4096 * sizeof( int32_t ) );
    std::vector<int32_t> ids( 4096 );
    std::memcpy( ids.data(), bytes.data(), bytes.size() );

    constexpr int32_t MinId = -1;
    constexpr int32_t MaxId = 4; // lightCountFull - 2

    bool seen[6] = {};
    for ( const int32_t id : ids )
    {
        REQUIRE( id >= MinId );
        REQUIRE( id <= MaxId );
        seen[id - MinId] = true;
    }

    for ( bool present : seen )
        CHECK( present );
}

// Contract: a reservoir's weight must be a function of the sample it stores
// (light id + selectedLightSeed), not of the caller's RNG state.
TEST_CASE( "restir/reservoir_weight_uses_stored_seed" )
{
    ComputeTestRunner runner;

    WeightInput input{};
    input.sun_dir[2]        = 1.0f;
    input.counts[0]         = 0;
    input.counts[1]         = 1;
    input.surface_pos[2]    = -1.0f;
    input.surface_normal[2] = 1.0f;

    input.r.totalWeight         = 4.0f;
    input.r.selectedLightWeight = 0.0f;
    input.r.visitedSampleCount  = 4;
    input.r.selectedLightId     = 0; // triangle light id (LightsCount + 0)
    input.r.selectedLightSeed   = 0x00C0FFEEu;

    input.caller_seeds[0] = 1;
    input.caller_seeds[1] = 424242;

    // Triangle with identity transform, instance id 0, intensity 1.
    // Winding is reversed so the geometric normal (0,0,-1) faces the surface
    // below it; otherwise the geometry term clamps to zero.
    input.tri_lights[0].p0[0] = 0.0f;
    input.tri_lights[0].p0[1] = 0.0f;
    input.tri_lights[0].p0[2] = 0.0f;
    input.tri_lights[0].p0[3] = 0.0f; // bit pattern of instance id 0

    input.tri_lights[0].p1[0] = 0.0f;
    input.tri_lights[0].p1[1] = 1.0f;
    input.tri_lights[0].p1[2] = 0.0f;
    input.tri_lights[0].p1[3] = 1.0f; // intensity

    input.tri_lights[0].p2[0] = 1.0f;
    input.tri_lights[0].p2[1] = 0.0f;
    input.tri_lights[0].p2[2] = 0.0f;
    input.tri_lights[0].p2[3] = 0.0f;

    for ( int i = 0; i < 4; ++i )
        input.tri_transforms[0][i * 5] = 1.0f; // identity mat4

    const auto output = RunWeight( runner, input );

    const float p_stored = output.values[0];
    const float w_seed_a = output.values[1];
    const float w_seed_b = output.values[2];
    const float expected =
        ( 1.0f / p_stored ) *
        ( input.r.totalWeight /
          static_cast<float>( input.r.visitedSampleCount ) );

    CAPTURE( p_stored );
    CAPTURE( w_seed_a );
    CAPTURE( w_seed_b );
    CAPTURE( expected );

    REQUIRE( p_stored > 0.0f );
    CHECK( w_seed_a == doctest::Approx( w_seed_b ) );
    CHECK( w_seed_a == doctest::Approx( expected ).epsilon( 1e-4 ) );
}

// combineReservoirs must carry the sample seed of the candidate it selected.
// The caller's RNG seed may only drive the selection, it must not become the
// sample identity. Two different triangle lights + a deterministic winner make
// "which candidate won" observable.
TEST_CASE( "restir/combine_keeps_sample_seed" )
{
    ComputeTestRunner runner;

    CombineSeedInput input{};
    input.sun_dir[2]        = 1.0f;
    input.counts[0]         = 0;
    input.counts[1]         = 2;
    input.surface_pos[2]    = -1.0f;
    input.surface_normal[2] = 1.0f;

    input.a.totalWeight         = 1.0f;
    input.a.visitedSampleCount  = 4;
    input.a.selectedLightId     = 0;
    input.a.selectedLightSeed   = 0x11111111u;

    input.b.totalWeight         = 0.5f;
    input.b.visitedSampleCount  = 2;
    input.b.selectedLightId     = 1;
    input.b.selectedLightSeed   = 0x22222222u;

    input.caller_seed[0] = 0x33333333u;

    // Two triangles with identity transform, instance id 0, intensity 1,
    // wound so their geometric normal (0,0,-1) faces the surface below.
    input.tri_lights[0].p0[3] = 0.0f; // instance id 0
    input.tri_lights[0].p1[1] = 1.0f; // v1 = (0, 1, 0)
    input.tri_lights[0].p1[3] = 1.0f; // intensity
    input.tri_lights[0].p2[0] = 1.0f; // v2 = (1, 0, 0)

    input.tri_lights[1].p0[0] = 2.0f; // v0 = (2, 0, 0)
    input.tri_lights[1].p0[3] = 0.0f; // instance id 0
    input.tri_lights[1].p1[0] = 2.0f; // v1 = (2, 1, 0)
    input.tri_lights[1].p1[1] = 1.0f;
    input.tri_lights[1].p1[3] = 1.0f; // intensity
    input.tri_lights[1].p2[0] = 3.0f; // v2 = (3, 0, 0)

    for ( int t = 0; t < 2; ++t )
        for ( int i = 0; i < 4; ++i )
            input.tri_transforms[t][i * 5] = 1.0f; // identity mat4

    // Case 1: b carries all the weight, so it must win with its own seed.
    input.a.selectedLightWeight = 0.0f;
    input.b.selectedLightWeight = 0.5f;
    const auto b_wins           = RunCombineSeed( runner, input );

    CAPTURE( b_wins.values[0] );
    CAPTURE( b_wins.values[1] );
    CHECK( b_wins.values[0] == input.b.selectedLightId );
    CHECK( b_wins.values[1] == input.b.selectedLightSeed );

    // Case 2: a carries all the weight.
    input.a.selectedLightWeight = 0.5f;
    input.b.selectedLightWeight = 0.0f;
    const auto a_wins           = RunCombineSeed( runner, input );

    CAPTURE( a_wins.values[0] );
    CAPTURE( a_wins.values[1] );
    CHECK( a_wins.values[0] == input.a.selectedLightId );
    CHECK( a_wins.values[1] == input.a.selectedLightSeed );
}

