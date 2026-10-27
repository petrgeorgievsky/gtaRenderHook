#include <doctest/doctest.h>

#include "Harness/ComputeTestRunner.h"

#include <Engine/Common/IBuffer.h>

#include <array>
#include <cstdint>
#include <cstring>

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

struct SampleStepInput
{
    float       sun_dir[4];
    int32_t     counts[4];
    PackedLight tri_lights[8];
    float       tri_transforms[8][16];
    uint32_t    seed[4];
    float       surface_pos[4];
    float       surface_normal[4];
};

struct SampleStepOutput
{
    uint32_t values[4];
};

SampleStepOutput RunSampleStep( ComputeTestRunner &runner,
                                const SampleStepInput &input )
{
    auto *in  = runner.CreateStorageBuffer( sizeof( input ), &input );
    auto *out = runner.CreateStorageBuffer( sizeof( SampleStepOutput ) );

    const std::array bindings = { BufferBinding{ 0, 0, in },
                                  BufferBinding{ 0, 1, out } };
    runner.Run( "shaders/tests/restir/sample_reservoir_candidate.comp",
                bindings );

    const auto       bytes = runner.Readback( out, sizeof( SampleStepOutput ) );
    SampleStepOutput output{};
    std::memcpy( &output, bytes.data(), sizeof( output ) );
    return output;
}

} // namespace

// The sampling step must take the sample point and the reservoir-selection
// uniform from independent RNG states. Otherwise the light selection is
// correlated with the barycentric coordinate of the sampled point.
TEST_CASE( "restir/sample_candidate_separates_rng_streams" )
{
    ComputeTestRunner runner;

    SampleStepInput input{};
    input.sun_dir[2]        = 1.0f;
    input.counts[0]         = 0; // LightsCount
    input.counts[1]         = 1; // TriLightsCount
    input.counts[2]         = 2; // lightCountFull = 0 + 1 + 1
    input.counts[3]         = 0; // sample index
    input.seed[0]           = 1234;
    input.surface_pos[2]    = -1.0f;
    input.surface_normal[2] = 1.0f;

    // Triangle with identity transform, instance id 0, intensity 1.
    input.tri_lights[0].p0[3] = 0.0f; // instance id 0
    input.tri_lights[0].p1[1] = 1.0f; // v1 = (0, 1, 0)
    input.tri_lights[0].p1[3] = 1.0f; // intensity
    input.tri_lights[0].p2[0] = 1.0f; // v2 = (1, 0, 0)

    for ( int i = 0; i < 4; ++i )
        input.tri_transforms[0][i * 5] = 1.0f; // identity mat4

    const auto output = RunSampleStep( runner, input );

    CAPTURE( output.values[0] );
    CAPTURE( output.values[1] );
    CAPTURE( output.values[2] );
    CAPTURE( output.values[3] );

    // The stored sample seed is derived from the stream, not copied from it.
    CHECK( output.values[0] == output.values[1] );

    // The selection uniform must not be the same draw the sample used.
    CHECK( output.values[2] != output.values[3] );
}
