#include "ModelLoadingTest.h"
#include <DebugUtils\DebugLogger.h>
#include <Engine\Definitions.h>
#include <TestUtils\WindowsSampleWrapper.h>
#include <windows.h>

#include <ConfigUtils/ConfigurationManager.h>
#include <memory>

int APIENTRY wWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance,
                       LPWSTR lpCmdLine, int )
{
    UNREFERENCED_PARAMETER( hPrevInstance );
    UNREFERENCED_PARAMETER( lpCmdLine );
    // Window params initialization
    rh::tests::WindowsSampleParams initParams;
    initParams.instance    = hInstance;
    initParams.sampleTitle = TEXT( "SimpleSample" );
    initParams.windowClass = TEXT( "RHSimpleSAMPLE" );

    rh::engine::RenderingAPI renderingAPI = rh::engine::RenderingAPI::DX11;
    // rh::engine::ConfigurationManager::Instance().SaveToFile( "config.json" );
    rh::engine::ConfigurationManager::Instance().LoadFromFile( "config.json" );

    rh::engine::String logFilePath = ToRHString( "RHDebug.log" );

    rh::debug::DebugLogger::Init( logFilePath, rh::debug::LogLevel::Info );

    rh::tests::WindowsSampleWrapper sample(
        initParams,
        std::make_unique<ModelLoadingTest>( renderingAPI, hInstance ) );

    // Initialize test sample.
    if ( !sample.Init() )
        return FALSE;

    // Run test sample!
    sample.Run();

    return TRUE;
}
