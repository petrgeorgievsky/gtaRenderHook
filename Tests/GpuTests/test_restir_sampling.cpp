#include <doctest/doctest.h>

#include "Harness/ComputeTestRunner.h"

#include <Engine/Common/IBuffer.h>

#include <array>
#include <cmath>
#include <cstdint>
#include <cstring>

using rh::tests::gpu::BufferBinding;
using rh::tests::gpu::ComputeTestRunner;

namespace
{

constexpr int32_t SampleCount = 4096;

struct SamplingInput
{
    uint32_t seed[4];
    int32_t  params[4];
};

} // namespace

// The sampled point must be distributed over the whole triangle, not on one
// of its edges. Mean barycentric weights of a uniform area sample are 1/3.
TEST_CASE( "restir/triangle_barycentrics_cover_triangle" )
{
    ComputeTestRunner runner;

    SamplingInput input{};
    input.seed[0]   = 1234;
    input.params[0] = SampleCount;

    auto *in  = runner.CreateStorageBuffer( sizeof( input ), &input );
    auto *out = runner.CreateStorageBuffer( SampleCount * sizeof( float ) * 4 );

    const std::array bindings = { BufferBinding{ 0, 0, in },
                                  BufferBinding{ 0, 1, out } };
    runner.Run( "shaders/tests/restir/sampling_barycentrics.comp", bindings );

    const auto bytes = runner.Readback( out, SampleCount * sizeof( float ) * 4 );
    REQUIRE( bytes.size() ==
             static_cast<size_t>( SampleCount ) * sizeof( float ) * 4 );

    const auto *bary = reinterpret_cast<const float *>( bytes.data() );

    double sum[3]     = { 0.0, 0.0, 0.0 };
    float  min_weight[3] = { 1.0f, 1.0f, 1.0f };
    float  max_weight[3] = { 0.0f, 0.0f, 0.0f };
    int    invalid_weights = 0;
    int    invalid_sums    = 0;

    for ( int i = 0; i < SampleCount; ++i )
    {
        const float *b = bary + i * 4;

        if ( std::abs( ( b[0] + b[1] + b[2] ) - 1.0f ) > 1e-4f )
            ++invalid_sums;

        for ( int k = 0; k < 3; ++k )
        {
            if ( b[k] < -1e-6f || b[k] > 1.0f + 1e-6f )
                ++invalid_weights;

            sum[k] += b[k];
            min_weight[k] = std::min( min_weight[k], b[k] );
            max_weight[k] = std::max( max_weight[k], b[k] );
        }
    }

    CHECK( invalid_sums == 0 );
    CHECK( invalid_weights == 0 );

    for ( int k = 0; k < 3; ++k )
    {
        const double mean = sum[k] / SampleCount;
        CAPTURE( k );
        CAPTURE( mean );
        CAPTURE( min_weight[k] );
        CAPTURE( max_weight[k] );
        CHECK( mean == doctest::Approx( 1.0 / 3.0 ).epsilon( 0.06 ) );
    }
}
