#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest/doctest.h>

#include "Harness/ComputeTestRunner.h"

#include <cstdio>

int main( int argc, char **argv )
{
    if ( !rh::tests::gpu::ComputeTestRunner::IsAvailable() )
    {
        std::printf( "GPU tests skipped: Vulkan device unavailable or "
                     "RH_SKIP_GPU_TESTS is set\n" );
        return 77;
    }

    doctest::Context context( argc, argv );
    return context.run();
}
