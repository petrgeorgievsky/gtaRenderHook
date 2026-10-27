#include <doctest/doctest.h>

#include <rendering_loop/ray_tracing/restir/dispatch_size.h>

#include <array>
#include <cstdint>

TEST_CASE( "restir/dispatch_covers_frame" )
{
    using rh::rw::engine::restir::ComputeDispatchSize;

    constexpr std::array<std::pair<uint32_t, uint32_t>, 4> sizes = {
        { { 1600, 900 }, { 1920, 1080 }, { 800, 600 }, { 1281, 721 } } };

    for ( const auto &[width, height] : sizes )
    {
        CAPTURE( width );
        CAPTURE( height );

        const auto dispatch = ComputeDispatchSize( width, height, 8 );
        CHECK( dispatch.x * 8 >= width );
        CHECK( dispatch.y * 8 >= height );
    }
}
