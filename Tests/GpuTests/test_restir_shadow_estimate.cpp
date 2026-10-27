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

struct alignas( 16 ) PackedLight
{
    float p0[4];
    float p1[4];
    float p2[4];
};

struct EstimateInput
{
    float       sun_dir[4];
    int32_t     counts[4];
    PackedLight lights[8];
    float       light_color[4];
    Reservoir   r;
    float       surface_pos[4];
    float       surface_normal[4];
};

struct EstimateOutput
{
    float estimate[4];
    float tint[4];
};

EstimateOutput RunEstimate( ComputeTestRunner &runner,
                            const EstimateInput &input )
{
    auto *in  = runner.CreateStorageBuffer( sizeof( input ), &input );
    auto *out = runner.CreateStorageBuffer( sizeof( EstimateOutput ) );

    const std::array bindings = { BufferBinding{ 0, 0, in },
                                  BufferBinding{ 0, 1, out } };
    runner.Run( "shaders/tests/restir/reservoir_shadow_estimate.comp",
                bindings );

    const auto     bytes = runner.Readback( out, sizeof( EstimateOutput ) );
    EstimateOutput output{};
    std::memcpy( &output, bytes.data(), sizeof( output ) );
    return output;
}

EstimateInput MakePointLightInput( float r, float g, float b )
{
    EstimateInput input{};
    input.sun_dir[2]        = 1.0f;
    input.counts[0]         = 1; // LightsCount
    input.counts[1]         = 0; // TriLightsCount
    input.surface_normal[2] = 1.0f;

    // Point light at (0, 0, 1), radius 2, no spot attenuation.
    input.lights[0].p0[2] = 1.0f;
    input.lights[0].p0[3] = 2.0f;
    input.lights[0].p1[3] = 0.0f;
    input.lights[0].p2[0] = r;
    input.lights[0].p2[1] = g;
    input.lights[0].p2[2] = b;
    input.lights[0].p2[3] = 1.0f;

    input.light_color[0] = r;
    input.light_color[1] = g;
    input.light_color[2] = b;
    input.light_color[3] = 1.0f;

    input.r.totalWeight         = 1.0f;
    input.r.selectedLightWeight = 0.5f;
    input.r.visitedSampleCount  = 4;
    input.r.selectedLightId     = 0;
    input.r.selectedLightSeed   = 1234;

    return input;
}

} // namespace

// p_hat carries |L| already, so the estimate must be linear in the light
// radiance. Applying the full color on top of p_hat squares the magnitude.
TEST_CASE( "restir/shadow_estimate_scales_linearly_with_light" )
{
    ComputeTestRunner runner;

    const auto dim =
        RunEstimate( runner, MakePointLightInput( 1.0f, 0.5f, 0.0f ) );
    const auto bright =
        RunEstimate( runner, MakePointLightInput( 2.0f, 1.0f, 0.0f ) );

    CAPTURE( dim.estimate[0] );
    CAPTURE( bright.estimate[0] );
    CAPTURE( dim.tint[0] );
    CAPTURE( bright.tint[0] );

    REQUIRE( dim.estimate[0] > 0.0f );

    // Linear in radiance: 2x the light color -> 2x the estimate (not 4x).
    CHECK( bright.estimate[0] == doctest::Approx( 2.0f * dim.estimate[0] ) );
    CHECK( bright.estimate[1] == doctest::Approx( 2.0f * dim.estimate[1] ) );
    CHECK( bright.estimate[2] == doctest::Approx( 2.0f * dim.estimate[2] ) );

    // Only the chromaticity is applied: unit length, same direction.
    const float tint_length = std::sqrt( dim.tint[0] * dim.tint[0] +
                                         dim.tint[1] * dim.tint[1] +
                                         dim.tint[2] * dim.tint[2] );
    CHECK( tint_length == doctest::Approx( 1.0f ) );
    CHECK( dim.tint[0] == doctest::Approx( bright.tint[0] ) );
    CHECK( dim.tint[1] == doctest::Approx( bright.tint[1] ) );
    CHECK( dim.tint[0] / dim.tint[1] ==
           doctest::Approx( 2.0f ) ); // (1, 0.5, 0) direction
}
