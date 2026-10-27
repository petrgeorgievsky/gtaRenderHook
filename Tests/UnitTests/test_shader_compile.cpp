#include <doctest/doctest.h>

#include <DebugUtils/DebugLogger.h>
#include <Engine/VulkanImpl/VulkanShader.h>

#include "TestPaths.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace
{

const fs::path LogPath = rh::tests::ModuleDir() / "rh_unit_tests.log";

struct LogFixture
{
    LogFixture()
    {
        rh::debug::DebugLogger::Init( LogPath.string(),
                                      rh::debug::LogLevel::Info );
    }
};

std::string StageFor( const fs::path &path )
{
    const auto ext = path.extension().string();
    if ( ext == ".comp" ) return "comp";
    if ( ext == ".vert" ) return "vert";
    if ( ext == ".frag" ) return "frag";
    if ( ext == ".rgen" ) return "rgen";
    if ( ext == ".rmiss" ) return "rmiss";
    if ( ext == ".rchit" ) return "rchit";
    if ( ext == ".rahit" ) return "rahit";
    return {};
}

std::vector<fs::path> CollectShaders( const fs::path &root )
{
    std::vector<fs::path> result;
    if ( !fs::exists( root ) )
        return result;

    for ( const auto &entry : fs::recursive_directory_iterator( root ) )
    {
        if ( !entry.is_regular_file() )
            continue;
        if ( StageFor( entry.path() ).empty() )
            continue;
        result.push_back( entry.path() );
    }

    std::sort( result.begin(), result.end() );
    return result;
}

std::string ReadLog()
{
    rh::debug::DebugLogger::SyncDebugFile();
    std::ifstream log( LogPath );
    return std::string( std::istreambuf_iterator<char>( log ),
                        std::istreambuf_iterator<char>() );
}

} // namespace

TEST_CASE_FIXTURE( LogFixture, "shader/restir_shaders_compile" )
{
    const auto shaders = CollectShaders( rh::tests::ModuleDir() / "shaders" /
                                         "vulkan" / "engine" / "restir" );
    REQUIRE( shaders.size() >= 4 );

    for ( const auto &shader : shaders )
    {
        CAPTURE( shader.generic_string() );
        CHECK( rh::engine::TranslateHLSL_to_SPIRV(
            rh::tests::ModuleRelative( shader ), "unit_test_tmp.spirv",
            StageFor( shader ), "main" ) );
    }
}

TEST_CASE_FIXTURE( LogFixture, "shader/broken_shader_is_rejected" )
{
    const auto broken = rh::tests::ModuleDir() / "broken_test.comp";
    {
        std::ofstream out( broken );
        out << "#version 460\nvoid main() { this_is_not_glsl; }\n";
    }

    CHECK_FALSE( rh::engine::TranslateHLSL_to_SPIRV(
        "broken_test.comp", "broken_test.spirv.comp", "comp", "main" ) );

    // The failure must stay visible: the log carries the compiler's exit code.
    CHECK( ReadLog().find( "glslangValidator failed for" ) != std::string::npos );
}

// Compiler output is drained from a pipe in 4096-byte chunks. Each chunk has to
// be cut at the number of bytes actually read, otherwise the tail of the log is
// uninitialized memory instead of the compiler's message.
TEST_CASE_FIXTURE( LogFixture, "shader/large_error_output_is_clean" )
{
    std::string source = "#version 460\nvoid main() {\n";
    for ( int i = 0; i < 200; ++i )
        source += "    undefined_identifier_" + std::to_string( i ) + ";\n";
    source += "}\n";

    const auto path = rh::tests::ModuleDir() / "large_error_test.comp";
    {
        std::ofstream out( path );
        out << source;
    }

    CHECK_FALSE( rh::engine::TranslateHLSL_to_SPIRV(
        "large_error_test.comp", "large_error_test.spirv.comp", "comp",
        "main" ) );

    const std::string log = ReadLog();

    size_t junk = 0;
    for ( const unsigned char c : log )
        if ( ( c < 0x20 && c != '\n' && c != '\r' && c != '\t' ) || c == 0x7F )
            ++junk;

    CAPTURE( log.size() );
    CAPTURE( junk );
    CHECK( junk == 0 );
}

TEST_CASE_FIXTURE( LogFixture, "shader/success_does_not_log_error" )
{
    REQUIRE( rh::engine::TranslateHLSL_to_SPIRV(
        "shaders/vulkan/engine/restir/reservoir_population_pass.comp",
        "unit_test_tmp.spirv", "comp", "main" ) );

    CHECK( ReadLog().find( "glslangValidator returned" ) == std::string::npos );
}

TEST_CASE_FIXTURE( LogFixture, "shader/all_shaders_compile" )
{
    const auto shaders =
        CollectShaders( rh::tests::ModuleDir() / "shaders" / "vulkan" );
    REQUIRE( shaders.size() >= 20 );

    std::vector<std::string> failed;
    for ( const auto &shader : shaders )
    {
        if ( !rh::engine::TranslateHLSL_to_SPIRV(
                 rh::tests::ModuleRelative( shader ), "unit_test_tmp.spirv",
                 StageFor( shader ), "main" ) )
            failed.push_back( shader.generic_string() );
    }

    CAPTURE( failed );
    CHECK( failed.empty() );
}

// GPU test kernels must compile here, before rh_gpu_tests runs them: a broken
// kernel would otherwise terminate that process instead of failing a test.
TEST_CASE_FIXTURE( LogFixture, "shader/test_kernels_compile" )
{
    const auto kernels =
        CollectShaders( rh::tests::ModuleDir() / "shaders" / "tests" );
    REQUIRE( kernels.size() >= 5 );

    std::vector<std::string> failed;
    for ( const auto &kernel : kernels )
    {
        if ( !rh::engine::TranslateHLSL_to_SPIRV(
                 rh::tests::ModuleRelative( kernel ), "unit_test_tmp.spirv",
                 StageFor( kernel ), "main" ) )
            failed.push_back( kernel.generic_string() );
    }

    CAPTURE( failed );
    CHECK( failed.empty() );
}
