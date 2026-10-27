#include <doctest/doctest.h>

#include "Harness/ComputeTestRunner.h"

#include <Engine/Common/IBuffer.h>

#include <array>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <vector>

using rh::tests::gpu::BufferBinding;
using rh::tests::gpu::ComputeTestRunner;

namespace
{

struct NeighbourCase
{
    float center_normal[3];
    float center_depth;
    float neighbour_normal[3];
    float neighbour_depth;
};

// Two vec4 per case, laid out exactly like the shader input.
std::vector<float> PackCases( const std::vector<NeighbourCase> &cases )
{
    std::vector<float> packed;
    packed.reserve( cases.size() * 8 );
    for ( const auto &c : cases )
    {
        packed.insert( packed.end(), c.center_normal,
                       c.center_normal + 3 );
        packed.push_back( c.center_depth );
        packed.insert( packed.end(), c.neighbour_normal,
                       c.neighbour_normal + 3 );
        packed.push_back( c.neighbour_depth );
    }
    return packed;
}

std::vector<int32_t> RunCompatibility( ComputeTestRunner           &runner,
                                       const std::vector<NeighbourCase> &cases )
{
    const auto packed = PackCases( cases );
    auto *in = runner.CreateStorageBuffer(
        static_cast<uint32_t>( packed.size() * sizeof( float ) ),
        packed.data() );
    auto *out = runner.CreateStorageBuffer(
        static_cast<uint32_t>( cases.size() * sizeof( int32_t ) ) );

    const std::array bindings = { BufferBinding{ 0, 0, in },
                                  BufferBinding{ 0, 1, out } };
    runner.Run( "shaders/tests/restir/reservoir_neighbour_compatible.comp",
                bindings, { static_cast<uint32_t>( cases.size() ), 1, 1 } );

    const auto bytes = runner.Readback(
        out, static_cast<uint32_t>( cases.size() * sizeof( int32_t ) ) );
    std::vector<int32_t> results( cases.size() );
    std::memcpy( results.data(), bytes.data(), bytes.size() );
    return results;
}

NeighbourCase MakeCase( float nx, float ny, float nz, float nd,
                        float cx, float cy, float cz, float cd )
{
    NeighbourCase c{};
    c.center_normal[0] = cx;
    c.center_normal[1] = cy;
    c.center_normal[2] = cz;
    c.center_depth    = cd;
    c.neighbour_normal[0] = nx;
    c.neighbour_normal[1] = ny;
    c.neighbour_normal[2] = nz;
    c.neighbour_depth    = nd;
    return c;
}

} // namespace

// Spatial reuse must reject neighbours from a different surface, otherwise
// reservoirs leak across silhouettes and depth discontinuities.
TEST_CASE( "restir/spatial_neighbour_rejects_other_surfaces" )
{
    ComputeTestRunner runner;

    const std::vector<NeighbourCase> cases = {
        // Same surface.
        MakeCase( 0, 0, 1, 1.00f, 0, 0, 1, 1.00f ),
        // Non-unit normal, same direction.
        MakeCase( 0, 0, 2, 1.00f, 0, 0, 1, 1.00f ),
        // ~14 degrees: inside the normal threshold.
        MakeCase( 0.0f, 0.242f, 0.970f, 1.00f, 0, 0, 1, 1.00f ),
        // ~38 degrees: outside the normal threshold.
        MakeCase( 0.0f, 0.616f, 0.788f, 1.00f, 0, 0, 1, 1.00f ),
        // Flipped normal.
        MakeCase( 0, 0, -1, 1.00f, 0, 0, 1, 1.00f ),
        // 5% depth difference: same surface.
        MakeCase( 0, 0, 1, 1.05f, 0, 0, 1, 1.00f ),
        // 20% depth difference: different surface.
        MakeCase( 0, 0, 1, 1.20f, 0, 0, 1, 1.00f ),
        // Neighbour is sky.
        MakeCase( 0, 0, 1, 6000.0f, 0, 0, 1, 1.00f ),
        // Center pixel is sky.
        MakeCase( 0, 0, 1, 1.00f, 0, 0, 1, 6000.0f ),
    };

    const auto results = RunCompatibility( runner, cases );

    const std::array<bool, 9> expected = { true,  true,  true,  false, false,
                                           true,  false, false, false };
    for ( size_t i = 0; i < expected.size(); ++i )
    {
        CAPTURE( i );
        CHECK( ( results[i] != 0 ) == expected[i] );
    }
}
