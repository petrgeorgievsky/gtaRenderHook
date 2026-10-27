#include <doctest/doctest.h>

#include "Harness/ComputeTestRunner.h"

#include <cstdint>
#include <cstring>

TEST_CASE( "gpu/device_available" )
{
    CHECK( rh::tests::gpu::ComputeTestRunner::IsAvailable() );
}

TEST_CASE( "gpu/runner_creates_buffer" )
{
    rh::tests::gpu::ComputeTestRunner runner;

    const uint32_t init   = 42;
    auto          *buffer = runner.CreateStorageBuffer( sizeof( init ), &init );
    REQUIRE( buffer != nullptr );

    const auto bytes = runner.Readback( buffer, sizeof( init ) );
    uint32_t   value = 0;
    std::memcpy( &value, bytes.data(), sizeof( value ) );
    CHECK( value == init );
}
